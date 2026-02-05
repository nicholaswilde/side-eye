use side_eye_host::config;
use side_eye_host::monitor;
use side_eye_host::sync;

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
    #[arg(short, long)]
    baud_rate: Option<u32>,

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
    monitor_all: Option<bool>,

    /// Data refresh interval in milliseconds
    #[arg(short, long)]
    interval: Option<u64>,
}

struct DeviceConnection {
    sender: std::sync::mpsc::Sender<String>,
}
fn main() -> Result<()> {
    let args = Args::parse();
    run(args)
}

fn run(args: Args) -> Result<()> {
    // Load configuration
    let config_path = args.config.as_ref().map(std::path::PathBuf::from);
    let mut config = config::Config::load(config_path)?;

    // Merge CLI args into config (CLI takes precedence)
    config = merge_args_into_config(config, &args);

    if config.verbose {
        println!("Configuration: {:?}", config);
    }

    let monitor = monitor::SystemMonitor::new();
    run_loop(config, monitor, args.dry_run)
}

fn run_loop(config: config::Config, mut monitor: monitor::SystemMonitor, dry_run: bool) -> Result<()> {
    let static_info = monitor.get_static_info();

    if config.verbose {
        println!("Static Info: {:?}", static_info);
    }

    let run_once = std::env::var("SIDEEYE_RUN_ONCE").is_ok();
    let connections: Arc<Mutex<HashMap<String, DeviceConnection>>> =
        Arc::new(Mutex::new(HashMap::new()));

    // Discovery / Management Loop
    let discovery_connections = Arc::clone(&connections);
    let discovery_config = config.clone();
    let verbose = config.verbose;

    if !dry_run {
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
        let stats = monitor.update_and_get_stats(&config.thresholds);
        let msg = monitor::HostMessage::Stats(stats);
        let payload = serde_json::to_string(&msg).unwrap_or_else(|_| "".to_string());

        if dry_run {
            if config.verbose {
                println!("Stats: {:?}", msg);
            }
            println!("Dry-Run Payload: {}", payload);
        } else {
            let mut cons = connections.lock().unwrap();
            let mut to_remove = Vec::new();

            for (name, conn) in cons.iter() {
                if config.verbose {
                    println!("Sending to {}: {}", name, payload);
                }
                if conn.sender.send(payload.clone() + "\n").is_err() {
                    if config.verbose {
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
        thread::sleep(Duration::from_millis(config.interval)); // Configurable interval
    }

    Ok(())
}


fn merge_args_into_config(mut config: config::Config, args: &Args) -> config::Config {
    if let Some(ref port) = args.port {
        config.ports = vec![port.clone()];
    }
    if let Some(monitor_all) = args.monitor_all {
        config.monitor_all = monitor_all;
    }
    if let Some(baud_rate) = args.baud_rate {
        config.baud_rate = baud_rate;
    }
    if let Some(interval) = args.interval {
        config.interval = interval;
    }
    if args.verbose {
        config.verbose = true;
    }
    config
}

#[cfg(test)]
mod tests {
    use super::*;
    use side_eye_host::monitor::{StaticInfo, SystemStats, SystemDataProvider};

    struct MockProvider;
    impl SystemDataProvider for MockProvider {
        fn get_static_info(&self) -> StaticInfo {
            StaticInfo {
                hostname: "test".into(),
                ip: "1.1.1.1".into(),
                mac: "AA".into(),
                os: "OS".into(),
                user: "user".into(),
            }
        }
        fn update_and_get_stats(&mut self, _t: &config::ThresholdsConfig) -> SystemStats {
            SystemStats {
                cpu_percent: 0.0,
                ram_used: 0,
                ram_total: 100,
                disk_used: 0,
                disk_total: 100,
                net_up: 0,
                net_down: 0,
                uptime: 0,
                thermal_c: 0.0,
                gpu_percent: 0.0,
                alert_level: 0,
            }
        }
    }

    #[test]
    fn test_merge_args() {
        let config = config::Config::default();
        let args = Args {
            port: Some("/dev/test".into()),
            baud_rate: Some(9600),
            verbose: true,
            dry_run: true,
            config: None,
            monitor_all: Some(false),
            interval: Some(500),
        };
        let merged = merge_args_into_config(config, &args);
        assert_eq!(merged.ports[0], "/dev/test");
        assert_eq!(merged.baud_rate, 9600);
        assert!(merged.verbose);
        assert_eq!(merged.interval, 500);
    }

    #[test]
    fn test_run_loop_dry_run() {
        let config = config::Config::default();
        let monitor = monitor::SystemMonitor::with_provider(Box::new(MockProvider));
        std::env::set_var("SIDEEYE_RUN_ONCE", "1");
        let result = run_loop(config, monitor, true);
        assert!(result.is_ok());
        std::env::remove_var("SIDEEYE_RUN_ONCE");
    }
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
                config.filters.contains(&vid)
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
                        if tx.send(json + "\n").is_err() && verbose {
                            eprintln!("Failed to send initial identity to {}", port_name);
                        }
                    }

                    // Trigger SD Sync
                    let sync_config = config.sd_sync.clone();
                    let sync_tx = tx.clone();
                    thread::spawn(move || {
                        let engine = sync::SDSyncEngine::new(sync_config, sync_tx);
                        if let Err(e) = engine.run_sync() {
                            eprintln!("SD Sync error: {}", e);
                        }
                    });

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
