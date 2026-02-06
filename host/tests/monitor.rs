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

#[test]
fn test_host_message_serialization() {
    use side_eye_host::monitor::{ChunkData, HostMessage, StaticInfo, SystemStats};

    let static_info = StaticInfo {
        hostname: "test".into(),
        ip: "1.2.3.4".into(),
        mac: "AA".into(),
        os: "OS".into(),
        user: "user".into(),
    };
    let msg = HostMessage::Identity(static_info);
    let json = serde_json::to_string(&msg).unwrap();
    assert!(json.contains("Identity"));

    let stats = SystemStats {
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
    };
    let msg_stats = HostMessage::Stats(stats);
    let json_stats = serde_json::to_string(&msg_stats).unwrap();
    assert!(json_stats.contains("Stats"));

    let msg_list = HostMessage::ListFiles { path: "/".into() };
    let json_list = serde_json::to_string(&msg_list).unwrap();
    assert!(json_list.contains("ListFiles"));

    let chunk = ChunkData {
        path: "test".into(),
        offset: 0,
        data: "abc".into(),
    };
    let msg_chunk = HostMessage::WriteChunk(chunk);
    let json_chunk = serde_json::to_string(&msg_chunk).unwrap();
    assert!(json_chunk.contains("WriteChunk"));
}
