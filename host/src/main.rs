mod config;
mod monitor;

use anyhow::{Context, Result};
use clap::Parser;
use serialport::{SerialPortType, UsbPortInfo};
use std::{
    collections::HashMap,
    sync::{Arc, Mutex},
    thread,
    time::Duration,
};

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

struct DeviceConnection {
    sender: std::sync::mpsc::Sender<String>,
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

    let mut monitor = monitor::SystemMonitor::new();
    let static_info = monitor.get_static_info();

    if args.verbose {
        println!("Static Info: {:?}", static_info);
    }

    let run_once = std::env::var("SIDEEYE_RUN_ONCE").is_ok();
    let connections: Arc<Mutex<HashMap<String, DeviceConnection>>> =
        Arc::new(Mutex::new(HashMap::new()));

    // Discovery / Management Loop
    let discovery_connections = Arc::clone(&connections);
    let discovery_config = config.clone();
    let verbose = args.verbose;

    if !args.dry_run {
        thread::spawn(move || loop {
            if let Err(e) = discover_and_connect(&discovery_config, &discovery_connections, verbose)
            {
                if verbose {
                    eprintln!("Discovery error: {}", e);
                }
            }
            thread::sleep(Duration::from_secs(5));
        });
    }

    loop {
        let stats = monitor.update_and_get_stats();
        let msg = monitor::HostMessage::Stats(stats);
        let payload = serde_json::to_string(&msg).unwrap_or_else(|_| "".to_string());

        if args.dry_run {
            if args.verbose {
                println!("Stats: {:?}", msg);
            }
            println!("Dry-Run Payload: {}", payload);
        } else {
            let mut cons = connections.lock().unwrap();
            let mut to_remove = Vec::new();

            for (name, conn) in cons.iter() {
                if args.verbose {
                    println!("Sending to {}: {}", name, payload);
                }
                if conn.sender.send(payload.clone() + "\n").is_err() {
                    if args.verbose {
                        println!("Connection to {} lost.", name);
                    }
                    to_remove.push(name.clone());
                }
            }

            for name in to_remove {
                cons.remove(&name);
            }
        }

        if run_once {
            break;
        }
        thread::sleep(Duration::from_secs(1)); // Faster updates per spec
    }

    Ok(())
}

fn discover_and_connect(
    config: &config::Config,
    connections: &Arc<Mutex<HashMap<String, DeviceConnection>>>,
    verbose: bool,
) -> Result<()> {
    let available_ports = serialport::available_ports().context("Failed to list serial ports")?;
    let mut cons = connections.lock().unwrap();

    for port in available_ports {
        if cons.contains_key(&port.port_name) {
            continue;
        }

        let should_connect = if config.monitor_all {
            if let SerialPortType::UsbPort(UsbPortInfo { vid, .. }) = port.port_type {
                config.target_vids.contains(&vid)
            } else {
                false
            }
        } else {
            config.ports.contains(&port.port_name)
        };

        if should_connect {
            if verbose {
                println!("Found new device at {}. Connecting...", port.port_name);
            }

            match serialport::new(&port.port_name, config.baud_rate)
                .timeout(Duration::from_millis(1000))
                .open()
            {
                Ok(serial) => {
                    let (tx, rx) = std::sync::mpsc::channel::<String>();
                    let port_name = port.port_name.clone();
                    let port_name_inner = port_name.clone();

                    println!("Connected to {}.", port_name);

                    // Send Identity immediately
                    let monitor = monitor::SystemMonitor::new();
                    let identity = monitor.get_static_info();
                    let msg = monitor::HostMessage::Identity(identity);
                    if let Ok(json) = serde_json::to_string(&msg) {
                        if tx.send(json + "\n").is_err() {
                            if verbose {
                                eprintln!("Failed to send initial identity to {}", port_name);
                            }
                        }
                    }

                    thread::spawn(move || {
                        let mut serial = serial;
                        while let Ok(payload) = rx.recv() {
                            let mut final_payload = String::with_capacity(payload.len() + 1);
                            final_payload.push('\n');
                            final_payload.push_str(&payload);

                            if let Err(e) = serial.write_all(final_payload.as_bytes()) {
                                if verbose {
                                    eprintln!("Write error on {}: {}", port_name_inner, e);
                                }
                                break;
                            }
                            if serial.flush().is_err() {
                                break;
                            }
                        }
                    });

                    cons.insert(port_name.clone(), DeviceConnection { sender: tx });
                }
                Err(e) => {
                    if verbose {
                        eprintln!("Failed to open {}: {}", port.port_name, e);
                    }
                }
            }
        }
    }

    Ok(())
}
