use local_ip_address::local_ip;
use mac_address::get_mac_address;
use nvml_wrapper::Nvml;
use serde::{Deserialize, Serialize};
use sysinfo::{Components, Disks, Networks, System, Users};

#[derive(Debug, Serialize, Clone)]
#[allow(dead_code)]
pub struct StaticInfo {
    pub hostname: String,
    pub ip: String,
    pub mac: String,
    pub os: String,
    pub user: String,
}

#[derive(Debug, Serialize, Clone)]
#[allow(dead_code)]
pub struct SystemStats {
    pub cpu_percent: f32,
    pub ram_used: u64,
    pub ram_total: u64,
    pub disk_used: u64,
    pub disk_total: u64,
    pub net_up: u64,
    pub net_down: u64,
    pub uptime: u64,
    pub thermal_c: f32,
    pub gpu_percent: f32,
    pub alert_level: u8,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct FileInfo {
    pub n: String, // name
    pub s: u64,    // size
    pub d: bool,   // is_dir
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ChunkData {
    pub path: String,
    pub offset: usize,
    pub data: String, // Base64 encoded
}

#[derive(Debug, Serialize, Clone)]
#[serde(tag = "type", content = "data")]
pub enum HostMessage {
    Identity(StaticInfo),
    Stats(SystemStats),
    ListFiles { path: String },
    WriteChunk(ChunkData),
}

#[derive(Debug, Deserialize, Clone)]
#[serde(tag = "type", content = "data")]
pub enum DeviceMessage {
    Version { version: String },
    FileList(Vec<FileInfo>),
    OperationResult { success: bool, message: String },
}

pub trait SystemDataProvider {
    fn get_static_info(&self) -> StaticInfo;
    fn update_and_get_stats(&mut self, thresholds: &crate::config::ThresholdsConfig)
        -> SystemStats;
}

pub struct RealDataProvider {
    sys: System,
    nvml: Option<Nvml>,
}

impl RealDataProvider {
    pub fn new() -> Self {
        Self {
            sys: System::new_all(),
            nvml: Nvml::init().ok(),
        }
    }
}

impl Default for RealDataProvider {
    fn default() -> Self {
        Self::new()
    }
}

impl SystemDataProvider for RealDataProvider {
    fn get_static_info(&self) -> StaticInfo {
        let hostname = System::host_name().unwrap_or_else(|| "Unknown".to_string());

        let ip = match local_ip() {
            Ok(ip) => ip.to_string(),
            Err(_) => "No IP".to_string(),
        };

        let mac = match get_mac_address() {
            Ok(Some(mac)) => mac.to_string(),
            Ok(None) => "No MAC".to_string(),
            Err(_) => "Error".to_string(),
        };

        let os = format!(
            "{} {}",
            System::name().unwrap_or_else(|| "Unknown".to_string()),
            System::os_version().unwrap_or_default()
        );

        let users = Users::new_with_refreshed_list();
        let user = users
            .first()
            .map(|u| u.name().to_string())
            .unwrap_or_else(|| "Unknown".to_string());

        StaticInfo {
            hostname,
            ip,
            mac,
            os,
            user,
        }
    }

    fn update_and_get_stats(
        &mut self,
        thresholds: &crate::config::ThresholdsConfig,
    ) -> SystemStats {
        self.sys.refresh_all();

        let cpu_percent = self.sys.global_cpu_info().cpu_usage();
        let ram_used = self.sys.used_memory();
        let ram_total = self.sys.total_memory();
        let ram_percent = (ram_used as f32 / ram_total as f32) * 100.0;

        let alert_level =
            SystemMonitor::calculate_alert_level(cpu_percent, ram_percent, thresholds);

        let mut disk_used = 0;
        let mut disk_total = 0;
        let disks = Disks::new_with_refreshed_list();
        for disk in &disks {
            disk_total += disk.total_space();
            disk_used += disk.total_space() - disk.available_space();
        }

        let mut net_down = 0;
        let mut net_up = 0;
        let networks = Networks::new_with_refreshed_list();
        for (_, data) in &networks {
            net_down += data.received();
            net_up += data.transmitted();
        }

        let uptime = System::uptime();

        // Thermal info
        let mut thermal_c = 0.0;
        let components = Components::new_with_refreshed_list();
        for component in &components {
            if component.label().to_lowercase().contains("cpu")
                || component.label().to_lowercase().contains("core")
            {
                thermal_c = component.temperature();
                break;
            }
        }

        // GPU info (NVIDIA)
        let mut gpu_percent = 0.0;
        if let Some(ref nvml) = self.nvml {
            if let Ok(device) = nvml.device_by_index(0) {
                if let Ok(util) = device.utilization_rates() {
                    gpu_percent = util.gpu as f32;
                }
            }
        }

        SystemStats {
            cpu_percent,
            ram_used,
            ram_total,
            disk_used,
            disk_total,
            net_up,
            net_down,
            uptime,
            thermal_c,
            gpu_percent,
            alert_level,
        }
    }
}

pub struct SystemMonitor {
    provider: Box<dyn SystemDataProvider>,
}

impl SystemMonitor {
    pub fn new() -> Self {
        Self {
            provider: Box::new(RealDataProvider::new()),
        }
    }

    pub fn with_provider(provider: Box<dyn SystemDataProvider>) -> Self {
        Self { provider }
    }

    pub fn get_static_info(&self) -> StaticInfo {
        self.provider.get_static_info()
    }

    pub fn update_and_get_stats(
        &mut self,
        thresholds: &crate::config::ThresholdsConfig,
    ) -> SystemStats {
        self.provider.update_and_get_stats(thresholds)
    }

    pub fn calculate_alert_level(
        cpu_percent: f32,
        ram_percent: f32,
        thresholds: &crate::config::ThresholdsConfig,
    ) -> u8 {
        let cpu_level = if cpu_percent >= thresholds.cpu_critical as f32 {
            2
        } else if cpu_percent >= thresholds.cpu_warning as f32 {
            1
        } else {
            0
        };

        let ram_level = if ram_percent >= thresholds.ram_critical as f32 {
            2
        } else if ram_percent >= thresholds.ram_warning as f32 {
            1
        } else {
            0
        };

        std::cmp::max(cpu_level, ram_level)
    }
}

impl Default for SystemMonitor {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::config::ThresholdsConfig;

    struct MockDataProvider {
        static_info: StaticInfo,
        stats: SystemStats,
    }

    impl SystemDataProvider for MockDataProvider {
        fn get_static_info(&self) -> StaticInfo {
            self.static_info.clone()
        }
        fn update_and_get_stats(
            &mut self,
            _thresholds: &crate::config::ThresholdsConfig,
        ) -> SystemStats {
            self.stats.clone()
        }
    }

    #[test]
    fn test_mock_monitor() {
        let static_info = StaticInfo {
            hostname: "test-host".into(),
            ip: "1.2.3.4".into(),
            mac: "AA:BB:CC:DD:EE:FF".into(),
            os: "Test OS".into(),
            user: "test-user".into(),
        };
        let stats = SystemStats {
            cpu_percent: 10.0,
            ram_used: 1000,
            ram_total: 8000,
            disk_used: 500,
            disk_total: 1000,
            net_up: 100,
            net_down: 200,
            uptime: 3600,
            thermal_c: 45.0,
            gpu_percent: 5.0,
            alert_level: 0,
        };

        let provider = MockDataProvider {
            static_info: static_info.clone(),
            stats: stats.clone(),
        };
        let mut monitor = SystemMonitor::with_provider(Box::new(provider));

        let info = monitor.get_static_info();
        assert_eq!(info.hostname, "test-host");

        let thresholds = ThresholdsConfig::default();
        let s = monitor.update_and_get_stats(&thresholds);
        assert_eq!(s.cpu_percent, 10.0);
    }

    #[test]
    fn test_calculate_alert_level() {
        let thresholds = ThresholdsConfig {
            cpu_warning: 70,
            cpu_critical: 90,
            ram_warning: 80,
            ram_critical: 95,
        };

        // Normal
        assert_eq!(
            SystemMonitor::calculate_alert_level(50.0, 50.0, &thresholds),
            0
        );

        // CPU Warning
        assert_eq!(
            SystemMonitor::calculate_alert_level(75.0, 50.0, &thresholds),
            1
        );

        // CPU Critical
        assert_eq!(
            SystemMonitor::calculate_alert_level(95.0, 50.0, &thresholds),
            2
        );

        // RAM Warning
        assert_eq!(
            SystemMonitor::calculate_alert_level(50.0, 85.0, &thresholds),
            1
        );

        // RAM Critical
        assert_eq!(
            SystemMonitor::calculate_alert_level(50.0, 97.0, &thresholds),
            2
        );
    }
}
