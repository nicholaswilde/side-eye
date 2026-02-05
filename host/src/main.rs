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
    let run_once = std::env::var("SIDEEYE_RUN_ONCE").is_ok();
    run_loop(
        config,
        monitor,
        args.dry_run,
        run_once,
        None,
        &RealPortScanner,
    )
}

fn run_loop(
    config: config::Config,
    mut monitor: monitor::SystemMonitor,
    dry_run: bool,
    run_once: bool,
    injected_connections: Option<Arc<Mutex<HashMap<String, DeviceConnection>>>>,
    _scanner: &dyn PortScanner,
) -> Result<()> {
    let static_info = monitor.get_static_info();

    if config.verbose {
        println!("Static Info: {:?}", static_info);
    }

    let connections = injected_connections.unwrap_or_else(|| Arc::new(Mutex::new(HashMap::new())));

    // Discovery / Management Loop
    let discovery_connections = Arc::clone(&connections);
    let discovery_config = config.clone();
    let verbose = config.verbose;

    if !dry_run && !run_once {
        thread::spawn(move || loop {
            if let Err(e) = discover_and_connect(
                &discovery_config,
                &discovery_connections,
                verbose,
                &RealPortScanner,
            ) {
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
            broadcast_stats(&connections, &payload, config.verbose);
        }

        if run_once {
            break;
        }
        thread::sleep(Duration::from_millis(config.interval)); // Configurable interval
    }

    Ok(())
}

fn broadcast_stats(
    connections: &Arc<Mutex<HashMap<String, DeviceConnection>>>,
    payload: &str,
    verbose: bool,
) {
    let mut cons = connections.lock().unwrap();
    let mut to_remove = Vec::new();

    for (name, conn) in cons.iter() {
        if verbose {
            println!("Sending to {}: {}", name, payload);
        }
        if conn.sender.send(payload.to_string() + "\n").is_err() {
            if verbose {
                println!("Connection to {} lost.", name);
            }
            to_remove.push(name.clone());
        }
    }

    for name in to_remove {
        cons.remove(&name);
    }
}

trait PortScanner: Send + Sync {
    fn available_ports(&self) -> Result<Vec<serialport::SerialPortInfo>>;
}

struct RealPortScanner;
impl PortScanner for RealPortScanner {
    fn available_ports(&self) -> Result<Vec<serialport::SerialPortInfo>> {
        serialport::available_ports().context("Failed to list serial ports")
    }
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

fn should_connect(port: &serialport::SerialPortInfo, config: &config::Config) -> bool {
    if config.monitor_all {
        if let SerialPortType::UsbPort(UsbPortInfo { vid, .. }) = port.port_type {
            config.filters.contains(&vid)
        } else {
            false
        }
    } else {
        config.ports.contains(&port.port_name)
    }
}

fn discover_and_connect(
    config: &config::Config,
    connections: &Arc<Mutex<HashMap<String, DeviceConnection>>>,
    verbose: bool,
    scanner: &dyn PortScanner,
) -> Result<()> {
    let available_ports = scanner.available_ports()?;
    let mut cons = connections.lock().unwrap();

    for port in available_ports {
        if cons.contains_key(&port.port_name) {
            continue;
        }

        if should_connect(&port, config) {
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

#[cfg(test)]
mod tests {
    use super::*;
    use side_eye_host::monitor::{StaticInfo, SystemDataProvider, SystemStats};

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

    struct MockPortScanner {
        ports: Vec<serialport::SerialPortInfo>,
    }
    impl PortScanner for MockPortScanner {
        fn available_ports(&self) -> Result<Vec<serialport::SerialPortInfo>> {
            Ok(self.ports.clone())
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
        let result = run_loop(config, monitor, true, true, None, &RealPortScanner);
        assert!(result.is_ok());
    }

    #[test]
    fn test_run_loop_with_connection() {
        let config = config::Config {
            verbose: true,
            ..Default::default()
        };
        let monitor = monitor::SystemMonitor::with_provider(Box::new(MockProvider));

        let (tx, rx) = std::sync::mpsc::channel();
        let connections = Arc::new(Mutex::new(HashMap::new()));
        connections
            .lock()
            .unwrap()
            .insert("test-port".to_string(), DeviceConnection { sender: tx });

        let result = run_loop(
            config,
            monitor,
            false,
            true,
            Some(connections),
            &RealPortScanner,
        );
        assert!(result.is_ok());

        let msg = rx.try_recv().expect("Should have received a message");
        assert!(msg.contains("Stats"));
    }

    #[test]
    fn test_run_loop_connection_lost() {
        let config = config::Config {
            verbose: true,
            ..Default::default()
        };
        let monitor = monitor::SystemMonitor::with_provider(Box::new(MockProvider));

        let (tx, rx) = std::sync::mpsc::channel();
        drop(rx); // Drop receiver immediately

        let connections = Arc::new(Mutex::new(HashMap::new()));
        connections
            .lock()
            .unwrap()
            .insert("lost-port".to_string(), DeviceConnection { sender: tx });

        let result = run_loop(
            config,
            monitor,
            false,
            true,
            Some(connections.clone()),
            &RealPortScanner,
        );
        assert!(result.is_ok());

        assert!(connections.lock().unwrap().is_empty());
    }

    #[test]
    fn test_discover_and_connect_mock() {
        let config = config::Config {
            monitor_all: true,
            filters: vec![0x1234],
            ..Default::default()
        };

        let connections = Arc::new(Mutex::new(HashMap::new()));
        let scanner = MockPortScanner {
            ports: vec![serialport::SerialPortInfo {
                port_name: "MOCK1".into(),
                port_type: SerialPortType::UsbPort(UsbPortInfo {
                    vid: 0x1234,
                    pid: 0x5678,
                    serial_number: None,
                    manufacturer: None,
                    product: None,
                }),
            }],
        };

        // This will attempt to open the port "MOCK1", which will fail because it doesn't exist.
        // But we cover the decision logic.
        let _ = discover_and_connect(&config, &connections, true, &scanner);
        // It shouldn't have added it because open() failed.
        assert!(connections.lock().unwrap().is_empty());
    }

    #[test]
    fn test_should_connect() {
        use serialport::SerialPortInfo;

        let config_monitor_all = config::Config {
            monitor_all: true,
            filters: vec![0x1234],
            ..Default::default()
        };

        let port_ok = SerialPortInfo {
            port_name: "COM1".into(),
            port_type: SerialPortType::UsbPort(UsbPortInfo {
                vid: 0x1234,
                pid: 0x5678,
                serial_number: None,
                manufacturer: None,
                product: None,
            }),
        };

        let port_wrong_vid = SerialPortInfo {
            port_name: "COM2".into(),
            port_type: SerialPortType::UsbPort(UsbPortInfo {
                vid: 0x9999,
                pid: 0x5678,
                serial_number: None,
                manufacturer: None,
                product: None,
            }),
        };

        let port_not_usb = SerialPortInfo {
            port_name: "COM3".into(),
            port_type: SerialPortType::Unknown,
        };

        assert!(should_connect(&port_ok, &config_monitor_all));
        assert!(!should_connect(&port_wrong_vid, &config_monitor_all));
        assert!(!should_connect(&port_not_usb, &config_monitor_all));

        let config_specific_ports = config::Config {
            monitor_all: false,
            ports: vec!["COM3".into()],
            ..Default::default()
        };
        assert!(should_connect(&port_not_usb, &config_specific_ports));
        assert!(!should_connect(&port_ok, &config_specific_ports));
    }
}
