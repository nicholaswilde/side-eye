use anyhow::{Context, Result};
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Config {
    #[serde(default = "default_ports")]
    pub ports: Vec<String>,
    #[serde(default = "default_monitor_all")]
    pub monitor_all: bool,
    #[serde(default = "default_target_vids")]
    pub target_vids: Vec<u16>,
    #[serde(default = "default_baud_rate")]
    pub baud_rate: u32,
}

fn default_ports() -> Vec<String> {
    Vec::new()
}

fn default_monitor_all() -> bool {
    false
}

fn default_target_vids() -> Vec<u16> {
    vec![0x303A]
}

fn default_baud_rate() -> u32 {
    115200
}

impl Default for Config {
    fn default() -> Self {
        Self {
            ports: default_ports(),
            monitor_all: default_monitor_all(),
            target_vids: default_target_vids(),
            baud_rate: default_baud_rate(),
        }
    }
}

impl Config {
    pub fn load(path: Option<PathBuf>) -> Result<Self> {
        let config_path = if let Some(p) = path {
            p
        } else {
            // Default path: ~/.config/side-eye/config.toml
            let mut p = home::home_dir().context("Could not find home directory")?;
            p.push(".config");
            p.push("side-eye");
            p.push("config.toml");
            p
        };

        if !config_path.exists() {
            return Ok(Config::default());
        }

        let content = fs::read_to_string(&config_path)
            .with_context(|| format!("Failed to read config file at {:?}", config_path))?;

        let config: Config = toml::from_str(&content)
            .with_context(|| format!("Failed to parse TOML config at {:?}", config_path))?;

        Ok(config)
    }
}
