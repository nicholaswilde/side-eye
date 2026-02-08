use predicates::prelude::*;

#[test]
fn file_doesnt_exist() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = assert_cmd::cargo::cargo_bin_cmd!("side-eye-host");

    cmd.arg("--non-existent-flag");
    cmd.assert()
        .failure()
        .stderr(predicate::str::contains("unexpected argument"));

    Ok(())
}

#[test]
fn version_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = assert_cmd::cargo::cargo_bin_cmd!("side-eye-host");

    cmd.arg("--version");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains(format!(
            "side-eye-host {}",
            env!("CARGO_PKG_VERSION")
        )));

    Ok(())
}

#[test]
fn help_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = assert_cmd::cargo::cargo_bin_cmd!("side-eye-host");

    cmd.arg("--help");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Usage:"));

    Ok(())
}

#[test]
fn verbose_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = assert_cmd::cargo::cargo_bin_cmd!("side-eye-host");

    // We combine with dry-run so it doesn't block forever
    cmd.env("SIDEEYE_RUN_ONCE", "1")
        .arg("--verbose")
        .arg("--dry-run");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Configuration:"));

    Ok(())
}

#[test]
fn dry_run_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = assert_cmd::cargo::cargo_bin_cmd!("side-eye-host");

    cmd.env("SIDEEYE_RUN_ONCE", "1").arg("--dry-run");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Dry-Run Payload:")); // Dry run should print to stdout

    Ok(())
}

#[test]
fn test_cli_overrides_config() -> Result<(), Box<dyn std::error::Error>> {
    let dir = tempfile::tempdir()?;
    let config_path = dir.path().join("side-eye.toml");
    std::fs::write(&config_path, "interval = 9999\nbaud_rate = 9600")?;

    let mut cmd = assert_cmd::cargo::cargo_bin_cmd!("side-eye-host");
    cmd.env("SIDEEYE_RUN_ONCE", "1")
        .arg("--config")
        .arg(config_path.to_str().unwrap())
        .arg("--interval")
        .arg("1234")
        .arg("--verbose")
        .arg("--dry-run");

    cmd.assert()
        .success()
        .stdout(predicate::str::contains("interval: 1234"))
        .stdout(predicate::str::contains("baud_rate: 9600"));

    Ok(())
}
