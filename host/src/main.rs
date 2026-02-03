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
}

fn main() -> Result<()> {
    let args = Args::parse();

    let port_name = match args.port {
        Some(p) => p,
        None => autodetect_port()?,
    };

    println!("Opening serial port: {} at {} baud", port_name, args.baud_rate);

    let mut port = serialport::new(&port_name, args.baud_rate)
        .timeout(Duration::from_millis(1000))
        .open()
        .with_context(|| format!("Failed to open serial port '{}'", port_name))?;

    let mut sys = System::new_all();

    loop {
        // Refresh system info
        sys.refresh_all();

        // 1. Hostname
        let hostname = System::host_name().unwrap_or_else(|| "Unknown".to_string());

        // 2. IP Address
        let ip = match local_ip() {
            Ok(ip) => ip.to_string(),
            Err(_) => "No IP".to_string(),
        };

        // 3. MAC Address
        let mac = match get_mac_address() {
            Ok(Some(mac)) => mac.to_string(),
            Ok(None) => "No MAC".to_string(),
            Err(_) => "Error".to_string(),
        };

        // Format payload: "HOSTNAME|IP|MAC\n"
        let payload = format!("{}|{}|{}\n", hostname, ip, mac);

        print!("Sending: {}", payload);

        if let Err(e) = port.write_all(payload.as_bytes()) {
            eprintln!("Failed to write to serial port: {}", e);
            // Attempt to reconnect or just wait? For now, we wait and retry next loop.
        }

        thread::sleep(Duration::from_secs(5));
    }
}

fn autodetect_port() -> Result<String> {
    let ports = serialport::available_ports().context("No serial ports found")?;

    // Filter for Espressif VID (0x303A)
    // ESP32-C6 default USB CDC VID:PID is often 303A:1001
    let esp_port = ports.iter().find(|p| {
        if let SerialPortType::UsbPort(UsbPortInfo { vid, .. }) = p.port_type {
            vid == 0x303A
        } else {
            false
        }
    });

    if let Some(p) = esp_port {
        println!("Auto-detected ESP32 device: {}", p.port_name);
        Ok(p.port_name.clone())
    } else {
        // Fallback: just take the first one or error
        if let Some(first) = ports.first() {
            println!("No Espressif device found. Defaulting to first port: {}", first.port_name);
            Ok(first.port_name.clone())
        } else {
            anyhow::bail!("No serial ports detected.");
        }
    }
}
