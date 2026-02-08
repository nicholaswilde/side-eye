use base64::{engine::general_purpose, Engine as _};
use side_eye_host::config::SDSyncConfig;
use side_eye_host::monitor::HostMessage;
use side_eye_host::sync::SDSyncEngine;
use std::fs;
use std::sync::mpsc;
use tempfile::tempdir;

#[test]
fn test_sync_protocol_single_file() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("small.txt");
    let content = "Hello Protocol";
    fs::write(&file_path, content).unwrap();

    let (tx, rx) = mpsc::channel();
    let config = SDSyncConfig {
        local_path: Some(dir.path().to_str().unwrap().to_string()),
        sync_mode: "one_way".to_string(),
        conflict_resolution: "host_wins".to_string(),
    };

    let engine = SDSyncEngine::new(config, tx);
    engine.run_sync().expect("Sync should succeed");

    let mut messages = Vec::new();
    while let Ok(msg) = rx.try_recv() {
        messages.push(msg);
    }

    assert_eq!(
        messages.len(),
        1,
        "Should have sent exactly 1 chunk for a small file"
    );

    let json_msg: HostMessage =
        serde_json::from_str(&messages[0]).expect("Message should be valid HostMessage JSON");
    if let HostMessage::WriteChunk(chunk) = json_msg {
        assert_eq!(chunk.path, "/small.txt");
        assert_eq!(chunk.offset, 0);
        let decoded = general_purpose::STANDARD
            .decode(&chunk.data)
            .expect("Data should be valid Base64");
        assert_eq!(String::from_utf8(decoded).unwrap(), content);
    } else {
        panic!("Message should be a WriteChunk");
    }
}

#[test]
fn test_sync_protocol_multi_chunk() {
    let dir = tempdir().unwrap();
    let file_path = dir.path().join("large.bin");

    // Create a 2.5KB file (should result in 3 chunks of 1KB, 1KB, 0.5KB)
    let content = vec![0u8; 2500];
    fs::write(&file_path, &content).unwrap();

    let (tx, rx) = mpsc::channel();
    let config = SDSyncConfig {
        local_path: Some(dir.path().to_str().unwrap().to_string()),
        sync_mode: "one_way".to_string(),
        conflict_resolution: "host_wins".to_string(),
    };

    let engine = SDSyncEngine::new(config, tx);
    engine.run_sync().expect("Sync should succeed");

    let mut messages = Vec::new();
    while let Ok(msg) = rx.try_recv() {
        messages.push(msg);
    }

    assert_eq!(
        messages.len(),
        3,
        "Should have sent exactly 3 chunks for a 2.5KB file"
    );

    let mut reassembled = Vec::new();
    for (i, msg_str) in messages.iter().enumerate() {
        let json_msg: HostMessage =
            serde_json::from_str(msg_str).expect("Message should be valid HostMessage JSON");
        if let HostMessage::WriteChunk(chunk) = json_msg {
            assert_eq!(chunk.path, "/large.bin");
            assert_eq!(chunk.offset, i * 1024);
            let decoded = general_purpose::STANDARD
                .decode(&chunk.data)
                .expect("Data should be valid Base64");
            reassembled.extend(decoded);
        } else {
            panic!("Message {} should be a WriteChunk", i);
        }
    }

    assert_eq!(
        reassembled, content,
        "Reassembled content should match original"
    );
}
