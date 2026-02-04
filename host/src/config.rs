use anyhow::Result;
use figment::{
    providers::{Env, Format, Json, Serialized, Toml, Yaml},
    Figment,
};
use serde::{Deserialize, Serialize};
use std::path::PathBuf;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Config {
    pub ports: Vec<String>,
    pub monitor_all: bool,
    pub filters: Vec<u16>,
    pub baud_rate: u32,
    pub interval: u64,
    pub verbose: bool,
    pub sd_sync: SDSyncConfig,
}

#[derive(Debug, Serialize, Deserialize, Clone, Default)]
pub struct SDSyncConfig {
    pub local_path: Option<String>,
    pub sync_mode: String,
    pub conflict_resolution: String,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            ports: Vec::new(),
            monitor_all: true,
            filters: vec![0x303A],
            baud_rate: 115200,
            interval: 1000,
            verbose: false,
            sd_sync: SDSyncConfig {
                local_path: None,
                sync_mode: "one_way".to_string(),
                conflict_resolution: "host_wins".to_string(),
            },
        }
    }
}

impl Config {
    pub fn load(path: Option<PathBuf>) -> Result<Self> {
        dotenvy::dotenv().ok();

        let mut figment =
            Figment::from(Serialized::defaults(Config::default())).merge(Env::prefixed("SIDEEYE_"));

        // Add config files from CWD
        figment = figment
            .merge(Toml::file("side-eye.toml"))
            .merge(Yaml::file("side-eye.yaml"))
            .merge(Yaml::file("side-eye.yml"))
            .merge(Json::file("side-eye.json"));

        // Add config files from ~/.config/side-eye/
        if let Some(mut config_dir) = home::home_dir() {
            config_dir.push(".config");
            config_dir.push("side-eye");

            figment = figment
                .merge(Toml::file(config_dir.join("config.toml")))
                .merge(Yaml::file(config_dir.join("config.yaml")))
                .merge(Yaml::file(config_dir.join("config.yml")))
                .merge(Json::file(config_dir.join("config.json")));
        }

        // Merge explicit path if provided
        if let Some(p) = path {
            let extension = p.extension().and_then(|e| e.to_str()).unwrap_or("");
            figment = match extension {
                "toml" => figment.merge(Toml::file(p)),
                "yaml" | "yml" => figment.merge(Yaml::file(p)),
                "json" => figment.merge(Json::file(p)),
                _ => figment, // Or error out?
            };
        }

        let config: Config = figment.extract()?;
        Ok(config)
    }
}
