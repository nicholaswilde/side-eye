use serde::{Deserialize, Serialize};

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
