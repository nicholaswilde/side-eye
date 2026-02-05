use side_eye_host::config::SDSyncConfig;
use side_eye_host::sync::SDSyncEngine;
use std::fs;
use std::sync::mpsc;
use tempfile::tempdir;

#[test]
fn test_sd_sync_one_file() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("test.txt");
    fs::write(&file_path, "Hello SideEye").unwrap();

    let (tx, rx) = mpsc::channel();
    let config = SDSyncConfig {
        local_path: Some(dir.path().to_str().unwrap().to_string()),
        sync_mode: "one_way".to_string(),
        conflict_resolution: "host_wins".to_string(),
    };

    let engine = SDSyncEngine::new(config, tx);
    engine.run_sync().unwrap();

    // Should have received at least one message
    let msg = rx.try_recv().expect("Should have received a message");
    assert!(msg.contains("test.txt"));
    assert!(msg.contains("WriteChunk"));
    assert!(msg.contains("SGVsbG8gU2lkZUV5ZQ==")); // Base64 for "Hello SideEye"
}

#[test]
fn test_sd_sync_disabled() {
    let (tx, rx) = mpsc::channel();
    let config = SDSyncConfig {
        local_path: None,
        sync_mode: "one_way".to_string(),
        conflict_resolution: "host_wins".to_string(),
    };

    let engine = SDSyncEngine::new(config, tx);
    engine.run_sync().unwrap();

    // Should have received NO messages
    assert!(rx.try_recv().is_err());
}

#[test]
fn test_sd_sync_nonexistent_path() {
    let (tx, _rx) = mpsc::channel();
    let config = SDSyncConfig {
        local_path: Some("/nonexistent/path/side-eye-test".to_string()),
        sync_mode: "one_way".to_string(),
        conflict_resolution: "host_wins".to_string(),
    };

    let engine = SDSyncEngine::new(config, tx);
    let result = engine.run_sync();
    assert!(result.is_err());
}

#[test]
fn test_sd_sync_subdirectories() {
    let dir = tempdir().unwrap();
    let sub_dir = dir.path().join("subdir");
    fs::create_dir(&sub_dir).unwrap();
    let file_path = sub_dir.join("test.txt");
    fs::write(&file_path, "Deep file").unwrap();

    let (tx, rx) = mpsc::channel();
    let config = SDSyncConfig {
        local_path: Some(dir.path().to_str().unwrap().to_string()),
        sync_mode: "one_way".to_string(),
        conflict_resolution: "host_wins".to_string(),
    };

    let engine = SDSyncEngine::new(config, tx);
    engine.run_sync().unwrap();

    let msg = rx.try_recv().expect("Should have received a message");
    assert!(msg.contains("/subdir/test.txt"));
    assert!(msg.contains("RGVlcCBmaWxl")); // Base64 for "Deep file"
}
