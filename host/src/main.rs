mod config;

use anyhow::{Context, Result};
use clap::Parser;
use local_ip_address::local_ip;
use mac_address::get_mac_address;
use serialport::{SerialPortType, UsbPortInfo};
use std::{thread, time::Duration};
use sysinfo::System;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Serial port path (e.g., /dev/ttyACM0). If not provided, auto-detection is attempted.
    #[arg(short, long)]
    port: Option<String>,

    /// Baud rate
    #[arg(short, long, default_value_t = 115200)]
    baud_rate: u32,

    /// Enable verbose output
    #[arg(short, long)]
    verbose: bool,

    /// Print stats to stdout without sending to serial port
    #[arg(short, long)]
    dry_run: bool,

    /// Path to configuration file
    #[arg(short, long)]
    config: Option<String>,

    /// Automatically monitor all compatible devices
    #[arg(short, long)]
    monitor_all: bool,
}

fn main() -> Result<()> {
    let args = Args::parse();

    // Load configuration
    let config_path = args.config.as_ref().map(std::path::PathBuf::from);
    let mut config = config::Config::load(config_path)?;

    // Merge CLI args into config (CLI takes precedence)
    if let Some(ref port) = args.port {
        config.ports = vec![port.clone()];
    }
    if args.monitor_all {
        config.monitor_all = true;
    }
    if args.baud_rate != 115200 {
        config.baud_rate = args.baud_rate;
    }

    if args.verbose {
        println!("Configuration: {:?}", config);
    }

    let mut sys = System::new_all();
    let run_once = std::env::var("SIDEEYE_RUN_ONCE").is_ok();

    loop {
        if args.dry_run {
            // Dry-run mode: just print and sleep
            match gather_payload(&mut sys) {
                Ok(payload) => {
                    if args.verbose {
                        println!("Dry-run: gathered stats successfully.");
                    }
                    println!("Dry-Run Payload: {}", payload.trim());
                }
                Err(e) => eprintln!("Error gathering stats: {}", e),
            }
            if run_once {
                break;
            }
            thread::sleep(Duration::from_secs(5));
            continue;
        }

        // Connection Mode
        let port_name = match &args.port {
            Some(p) => p.clone(),
            None => match autodetect_port(args.verbose) {
                Ok(name) => name,
                Err(e) => {
                    if args.verbose {
                        eprintln!("Auto-detect failed: {}. Retrying in 5s...", e);
                    }
                    if run_once {
                        return Err(e);
                    }
                    thread::sleep(Duration::from_secs(5));
                    continue;
                }
            },
        };

        if args.verbose {
            println!("Connecting to {} at {} baud...", port_name, args.baud_rate);
        }

        match serialport::new(&port_name, args.baud_rate)
            .timeout(Duration::from_millis(1000))
            .open()
        {
            Ok(mut port) => {
                println!("Connected to {}.", port_name);

                // Inner Loop: Stream Data
                loop {
                    let payload = match gather_payload(&mut sys) {
                        Ok(p) => p,
                        Err(e) => {
                            eprintln!("Error gathering stats: {}", e);
                            break; // Should rarely happen, but let's reset
                        }
                    };

                    if args.verbose {
                        println!("Sending: {}", payload.trim());
                    }

                    // Add a leading newline to clear any partial buffers on the device
                    let mut final_payload = String::with_capacity(payload.len() + 1);
                    final_payload.push('\n');
                    final_payload.push_str(&payload);

                    if let Err(e) = port.write_all(final_payload.as_bytes()) {
                        eprintln!("Write error: {}. Dropping connection.", e);
                        break;
                    }
                    if let Err(e) = port.flush() {
                        eprintln!("Flush error: {}. Dropping connection.", e);
                        break;
                    }

                    if run_once {
                        return Ok(());
                    }
                    thread::sleep(Duration::from_secs(5));
                }
            }
            Err(e) => {
                eprintln!("Failed to open serial port: {}. Retrying in 2s...", e);
                if run_once {
                    return Err(e.into());
                }
                thread::sleep(Duration::from_secs(2));
            }
        }
    }
    Ok(())
}

fn gather_payload(sys: &mut System) -> Result<String> {
    sys.refresh_all();
    let hostname = System::host_name().unwrap_or_else(|| "Unknown".to_string());

    let ip = match local_ip() {
        Ok(ip) => ip.to_string(),
        Err(_) => "No IP".to_string(),
    };

    let mac = match get_mac_address() {
        Ok(Some(mac)) => mac.to_string(),
        Ok(None) => "No MAC".to_string(),
        Err(_) => "Error".to_string(),
    };

    Ok(format!("{}|{}|{}\n", hostname, ip, mac))
}

fn autodetect_port(verbose: bool) -> Result<String> {
    let ports = serialport::available_ports().context("No serial ports found")?;

    if verbose {
        println!("Available ports: {:?}", ports);
    }

    let esp_port = ports.iter().find(|p| {
        if let SerialPortType::UsbPort(UsbPortInfo { vid, pid, .. }) = p.port_type {
            if verbose {
                println!(
                    "Checking port {} (VID: {:04x}, PID: {:04x})",
                    p.port_name, vid, pid
                );
            }
            vid == 0x303A
        } else {
            false
        }
    });

    if let Some(p) = esp_port {
        Ok(p.port_name.clone())
    } else {
        let mut msg = String::from("No Espressif device found (VID 0x303A).");
        if !ports.is_empty() {
            msg.push_str("\nFound other ports:");
            for p in ports {
                if let SerialPortType::UsbPort(UsbPortInfo { vid, pid, .. }) = p.port_type {
                    msg.push_str(&format!(
                        "\n- {} (VID:{:04x} PID:{:04x})",
                        p.port_name, vid, pid
                    ));
                } else {
                    msg.push_str(&format!("\n- {} (Non-USB)", p.port_name));
                }
            }
        }
        anyhow::bail!(msg);
    }
}
