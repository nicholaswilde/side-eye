use assert_cmd::Command;
use predicates::prelude::*;

#[test]
fn file_doesnt_exist() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("side-eye-host")?;

    cmd.arg("--non-existent-flag");
    cmd.assert()
        .failure()
        .stderr(predicate::str::contains("unexpected argument"));

    Ok(())
}

#[test]
fn version_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("side-eye-host")?;

    cmd.arg("--version");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("side-eye-host 0.1.0"));

    Ok(())
}

#[test]
fn help_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("side-eye-host")?;

    cmd.arg("--help");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Usage:"));

    Ok(())
}

#[test]
fn verbose_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("side-eye-host")?;

    // We combine with dry-run so it doesn't block forever
    cmd.env("SIDEEYE_RUN_ONCE", "1")
        .arg("--verbose")
        .arg("--dry-run");
    cmd.assert().success().stdout(predicate::str::contains(
        "Dry-run: gathered stats successfully.",
    ));

    Ok(())
}

#[test]
fn dry_run_flag_works() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("side-eye-host")?;

    cmd.env("SIDEEYE_RUN_ONCE", "1").arg("--dry-run");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Dry-Run Payload:")); // Dry run should print to stdout

    Ok(())
}
