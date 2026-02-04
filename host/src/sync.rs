use crate::config::SDSyncConfig;
use crate::monitor::HostMessage;
use anyhow::{Context, Result};
use base64::{engine::general_purpose, Engine as _};
use std::fs;
use std::path::Path;
use std::sync::mpsc::Sender;

pub struct SDSyncEngine {
    config: SDSyncConfig,
    tx: Sender<String>,
}

impl SDSyncEngine {
    pub fn new(config: SDSyncConfig, tx: Sender<String>) -> Self {
        Self { config, tx }
    }

    pub fn run_sync(&self) -> Result<()> {
        let local_path = match &self.config.local_path {
            Some(p) => Path::new(p),
            None => return Ok(()), // Sync disabled
        };

        if !local_path.exists() {
            return Err(anyhow::anyhow!(
                "Local sync path does not exist: {:?}",
                local_path
            ));
        }

        println!("Starting SD sync from {:?}", local_path);
        self.sync_directory(local_path, "/")?;
        println!("SD sync complete.");
        Ok(())
    }

    fn sync_directory(&self, local_root: &Path, remote_dir: &str) -> Result<()> {
        for entry in fs::read_dir(local_root)? {
            let entry = entry?;
            let path = entry.path();
            let file_name = entry
                .file_name()
                .into_string()
                .map_err(|_| anyhow::anyhow!("Invalid filename"))?;
            let remote_path = if remote_dir == "/" {
                format!("/{}", file_name)
            } else {
                format!("{}/{}", remote_dir, file_name)
            };

            if path.is_dir() {
                self.sync_directory(&path, &remote_path)?;
            } else {
                self.sync_file(&path, &remote_path)?;
            }
        }
        Ok(())
    }

    fn sync_file(&self, local_path: &Path, remote_path: &str) -> Result<()> {
        // For now, simple one-way overwrite sync
        // In a real implementation, we'd check file sizes/hashes first via ListFiles

        let data = fs::read(local_path).context("Failed to read local file")?;
        let total_size = data.len();
        let chunk_size = 1024; // 1KB chunks for serial stability

        println!("Syncing {} ({} bytes)...", remote_path, total_size);

        for (i, chunk) in data.chunks(chunk_size).enumerate() {
            let offset = i * chunk_size;
            let b64_data = general_purpose::STANDARD.encode(chunk);

            let msg = HostMessage::WriteChunk(crate::monitor::ChunkData {
                path: remote_path.to_string(),
                offset,
                data: b64_data,
            });

            let json = serde_json::to_string(&msg)?;
            self.tx
                .send(
                    json + "
",
                )
                .context("Failed to send chunk to serial thread")?;

            // Small delay to prevent serial buffer overflow
            std::thread::sleep(std::time::Duration::from_millis(50));
        }

        Ok(())
    }
}
