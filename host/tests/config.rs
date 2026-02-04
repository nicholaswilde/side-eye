use side_eye_host::config::Config;
use std::fs;
use tempfile::tempdir;

#[test]
fn test_load_toml() {
    let dir = tempdir().unwrap();
    let config_path = dir.path().join("config.toml");
    fs::write(
        &config_path,
        "interval = 5000
baud_rate = 9600",
    )
    .unwrap();

    let config = Config::load(Some(config_path)).unwrap();
    assert_eq!(config.interval, 5000);
    assert_eq!(config.baud_rate, 9600);
}

#[test]
fn test_load_yaml() {
    let dir = tempdir().unwrap();
    let config_path = dir.path().join("config.yaml");
    fs::write(
        &config_path,
        "interval: 2000
baud_rate: 19200",
    )
    .unwrap();

    let config = Config::load(Some(config_path)).unwrap();
    assert_eq!(config.interval, 2000);
    assert_eq!(config.baud_rate, 19200);
}

#[test]
fn test_load_json() {
    let dir = tempdir().unwrap();
    let config_path = dir.path().join("config.json");
    fs::write(&config_path, r#"{"interval": 3000, "baud_rate": 38400}"#).unwrap();

    let config = Config::load(Some(config_path)).unwrap();
    assert_eq!(config.interval, 3000);
    assert_eq!(config.baud_rate, 38400);
}

#[test]
fn test_load_env() {
    std::env::set_var("SIDEEYE_INTERVAL", "4000");
    std::env::set_var("SIDEEYE_BAUD_RATE", "57600");

    let config = Config::load(None).unwrap();
    assert_eq!(config.interval, 4000);
    assert_eq!(config.baud_rate, 57600);

    // Cleanup
    std::env::remove_var("SIDEEYE_INTERVAL");
    std::env::remove_var("SIDEEYE_BAUD_RATE");
}
