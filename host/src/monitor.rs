use local_ip_address::local_ip;
use mac_address::get_mac_address;
use serde::Serialize;
use sysinfo::{Disks, Networks, System, Users};

pub struct SystemMonitor {
    sys: System,
}

#[derive(Debug, Serialize)]
#[allow(dead_code)]
pub struct StaticInfo {
    pub hostname: String,
    pub ip: String,
    pub mac: String,
    pub os: String,
    pub user: String,
}

#[derive(Debug, Serialize)]
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
}

#[derive(Debug, Serialize, Deserialize)]
pub struct FileInfo {
    pub n: String, // name
    pub s: u64,    // size
    pub d: bool,   // is_dir
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ChunkData {
    pub path: String,
    pub offset: usize,
    pub data: String, // Base64 encoded
}

#[derive(Debug, Serialize)]
#[serde(tag = "type", content = "data")]
pub enum HostMessage {
    Identity(StaticInfo),
    Stats(SystemStats),
    ListFiles { path: String },
    WriteChunk(ChunkData),
}

#[derive(Debug, Deserialize)]
#[serde(tag = "type", content = "data")]
pub enum DeviceMessage {
    Version { version: String },
    FileList(Vec<FileInfo>),
    OperationResult { success: bool, message: String },
}

impl SystemMonitor {
    pub fn new() -> Self {
        Self {
            sys: System::new_all(),
        }
    }

    pub fn get_static_info(&self) -> StaticInfo {
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

    pub fn update_and_get_stats(&mut self) -> SystemStats {
        self.sys.refresh_all();

        let cpu_percent = self.sys.global_cpu_info().cpu_usage();
        let ram_used = self.sys.used_memory();
        let ram_total = self.sys.total_memory();

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

        SystemStats {
            cpu_percent,
            ram_used,
            ram_total,
            disk_used,
            disk_total,
            net_up,
            net_down,
            uptime,
        }
    }
}
