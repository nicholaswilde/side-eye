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
        Arc::new(RealPortScanner),
        Arc::new(RealPortOpener),
    )
}

fn run_loop(
    config: config::Config,
    mut monitor: monitor::SystemMonitor,
    dry_run: bool,
    run_once: bool,
    injected_connections: Option<Arc<Mutex<HashMap<String, DeviceConnection>>>>,
    scanner: Arc<dyn PortScanner>,
    opener: Arc<dyn PortOpener>,
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
    let discovery_scanner = Arc::clone(&scanner);
    let discovery_opener = Arc::clone(&opener);

    if !dry_run && !run_once {
        thread::spawn(move || loop {
            if let Err(e) = discover_and_connect(
                &discovery_config,
                &discovery_connections,
                verbose,
                discovery_scanner.as_ref(),
                discovery_opener.as_ref(),
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

trait PortOpener: Send + Sync {
    fn open(&self, name: &str, baud_rate: u32) -> Result<Box<dyn serialport::SerialPort>>;
}

struct RealPortOpener;
impl PortOpener for RealPortOpener {
    fn open(&self, name: &str, baud_rate: u32) -> Result<Box<dyn serialport::SerialPort>> {
        serialport::new(name, baud_rate)
            .timeout(Duration::from_millis(1000))
            .open()
            .map_err(|e| anyhow::anyhow!("Failed to open port {}: {}", name, e))
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
    opener: &dyn PortOpener,
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

            match opener.open(&port.port_name, config.baud_rate) {
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

    struct MockPortOpener {
        should_fail: bool,
    }
    impl PortOpener for MockPortOpener {
        fn open(&self, _name: &str, _baud_rate: u32) -> Result<Box<dyn serialport::SerialPort>> {
            if self.should_fail {
                Err(anyhow::anyhow!("Mock failure"))
            } else {
                Ok(Box::new(MockSerialPort))
            }
        }
    }

    struct MockSerialPort;
    impl std::io::Read for MockSerialPort {
        fn read(&mut self, _buf: &mut [u8]) -> std::io::Result<usize> {
            Ok(0)
        }
    }
    impl std::io::Write for MockSerialPort {
        fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
            Ok(buf.len())
        }
        fn flush(&mut self) -> std::io::Result<()> {
            Ok(())
        }
    }
    impl serialport::SerialPort for MockSerialPort {
        fn name(&self) -> Option<String> {
            None
        }
        fn baud_rate(&self) -> serialport::Result<u32> {
            Ok(9600)
        }
        fn data_bits(&self) -> serialport::Result<serialport::DataBits> {
            Ok(serialport::DataBits::Eight)
        }
        fn flow_control(&self) -> serialport::Result<serialport::FlowControl> {
            Ok(serialport::FlowControl::None)
        }
        fn parity(&self) -> serialport::Result<serialport::Parity> {
            Ok(serialport::Parity::None)
        }
        fn stop_bits(&self) -> serialport::Result<serialport::StopBits> {
            Ok(serialport::StopBits::One)
        }
        fn timeout(&self) -> Duration {
            Duration::from_millis(0)
        }
        fn set_baud_rate(&mut self, _baud_rate: u32) -> serialport::Result<()> {
            Ok(())
        }
        fn set_data_bits(&mut self, _data_bits: serialport::DataBits) -> serialport::Result<()> {
            Ok(())
        }
        fn set_flow_control(
            &mut self,
            _flow_control: serialport::FlowControl,
        ) -> serialport::Result<()> {
            Ok(())
        }
        fn set_parity(&mut self, _parity: serialport::Parity) -> serialport::Result<()> {
            Ok(())
        }
        fn set_stop_bits(&mut self, _stop_bits: serialport::StopBits) -> serialport::Result<()> {
            Ok(())
        }
        fn set_timeout(&mut self, _timeout: Duration) -> serialport::Result<()> {
            Ok(())
        }
        fn write_request_to_send(&mut self, _level: bool) -> serialport::Result<()> {
            Ok(())
        }
        fn write_data_terminal_ready(&mut self, _level: bool) -> serialport::Result<()> {
            Ok(())
        }
        fn read_clear_to_send(&mut self) -> serialport::Result<bool> {
            Ok(false)
        }
        fn read_data_set_ready(&mut self) -> serialport::Result<bool> {
            Ok(false)
        }
        fn read_ring_indicator(&mut self) -> serialport::Result<bool> {
            Ok(false)
        }
        fn read_carrier_detect(&mut self) -> serialport::Result<bool> {
            Ok(false)
        }
        fn bytes_to_read(&self) -> serialport::Result<u32> {
            Ok(0)
        }
        fn bytes_to_write(&self) -> serialport::Result<u32> {
            Ok(0)
        }
        fn clear(&self, _buffer_to_clear: serialport::ClearBuffer) -> serialport::Result<()> {
            Ok(())
        }
        fn try_clone(&self) -> serialport::Result<Box<dyn serialport::SerialPort>> {
            Ok(Box::new(MockSerialPort))
        }
        fn set_break(&self) -> serialport::Result<()> {
            Ok(())
        }
        fn clear_break(&self) -> serialport::Result<()> {
            Ok(())
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
        let result = run_loop(
            config,
            monitor,
            true,
            true,
            None,
            Arc::new(RealPortScanner),
            Arc::new(RealPortOpener),
        );
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
            Arc::new(RealPortScanner),
            Arc::new(RealPortOpener),
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
            Arc::new(RealPortScanner),
            Arc::new(RealPortOpener),
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
        let opener = MockPortOpener { should_fail: false };

        let result = discover_and_connect(&config, &connections, true, &scanner, &opener);
        assert!(result.is_ok());
        // It should have added it now because opener succeeded
        assert!(!connections.lock().unwrap().is_empty());
    }

    #[test]
    fn test_discover_and_connect_failure() {
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
        let opener = MockPortOpener { should_fail: true };

        let result = discover_and_connect(&config, &connections, true, &scanner, &opener);
        assert!(result.is_ok());
        assert!(connections.lock().unwrap().is_empty());
    }

    #[test]
    fn test_broadcast_stats_verbose() {
        let (tx, rx) = std::sync::mpsc::channel();
        let connections = Arc::new(Mutex::new(HashMap::new()));
        connections
            .lock()
            .unwrap()
            .insert("test".to_string(), DeviceConnection { sender: tx });

        broadcast_stats(&connections, "test payload", true);
        let msg = rx.recv().unwrap();
        assert_eq!(msg, "test payload\n");
    }

    #[test]
    fn test_broadcast_stats_connection_lost_verbose() {
        let (tx, rx) = std::sync::mpsc::channel();
        drop(rx);
        let connections = Arc::new(Mutex::new(HashMap::new()));
        connections
            .lock()
            .unwrap()
            .insert("test".to_string(), DeviceConnection { sender: tx });

        broadcast_stats(&connections, "test payload", true);
        assert!(connections.lock().unwrap().is_empty());
    }

    #[test]
    fn test_real_port_scanner_smoke() {
        let scanner = RealPortScanner;
        let _ = scanner.available_ports();
    }

    #[test]
    fn test_real_port_opener_smoke() {
        let opener = RealPortOpener;
        // This will likely fail but we cover the code path
        let _ = opener.open("NONEXISTENT", 9600);
    }

    #[test]
    fn test_run_smoke() {
        // We can't easily test the full run() because of Args::parse() and possible blocking
        // but we can test merge_args_into_config and run_loop separately which we already do.
    }

    #[test]
    fn test_discover_and_connect_threads() {
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
        let opener = MockPortOpener { should_fail: false };

        let result = discover_and_connect(&config, &connections, true, &scanner, &opener);
        assert!(result.is_ok());

        // Wait a bit for threads to start and potentially do something
        thread::sleep(Duration::from_millis(100));

        let cons = connections.lock().unwrap();
        assert!(cons.contains_key("MOCK1"));
        let sender = &cons.get("MOCK1").unwrap().sender;

        // Test sending a message through the connection
        assert!(sender.send("test message".to_string()).is_ok());

        thread::sleep(Duration::from_millis(50));
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
