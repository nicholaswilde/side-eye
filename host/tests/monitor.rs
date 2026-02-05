use side_eye_host::monitor::SystemMonitor;

#[test]
fn test_get_static_info() {
    let monitor = SystemMonitor::new();
    let info = monitor.get_static_info();

    assert!(!info.hostname.is_empty());
    assert!(!info.os.is_empty());
    assert!(!info.user.is_empty());
    // IP and MAC might be "No IP" or "Error" depending on the environment,
    // but they should be present.
    assert!(!info.ip.is_empty());
    assert!(!info.mac.is_empty());
}

#[test]
fn test_update_and_get_stats() {
    let mut monitor = SystemMonitor::new();
    let thresholds = side_eye_host::config::ThresholdsConfig::default();
    let stats = monitor.update_and_get_stats(&thresholds);

    // Stats should be non-negative
    assert!(stats.cpu_percent >= 0.0);
    assert!(stats.ram_total > 0);
    assert!(stats.ram_used <= stats.ram_total);
    // Alert level should be 0, 1, or 2
    assert!(stats.alert_level <= 2);
}
