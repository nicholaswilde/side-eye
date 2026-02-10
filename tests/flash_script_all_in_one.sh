#!/usr/bin/env bash
# Integrated functional test runner for flash.sh to maximize coverage

set -euo pipefail

SCRIPT="firmware/scripts/flash.sh"
MOCK_DIR=$(mktemp -d)
export PATH="$MOCK_DIR:$PATH"

setup_mocks() {
  cat > "$MOCK_DIR/curl" <<EOF
#!/bin/bash
if [[ "\$*" == *"releases/latest"* ]]; then
  if [[ "\$*" == *"-fsSL"* ]]; then
    echo '{"tag_name": "v0.1.3", "assets": [{"browser_download_url": "https://example.com/side-eye-0.1.3-firmware.zip"}]}'
  else
    echo 'browser_download_url: https://example.com/side-eye-0.1.3-firmware.zip'
  fi
else
  touch "\${@: -1}"
fi
EOF
  chmod +x "$MOCK_DIR/curl"

  cat > "$MOCK_DIR/unzip" <<EOF
#!/bin/bash
DEST="\$4"; mkdir -p "\$DEST"
touch "\$DEST/bootloader.bin" "\$DEST/partitions.bin" "\$DEST/firmware.bin"
EOF
  chmod +x "$MOCK_DIR/unzip"

  cat > "$MOCK_DIR/esptool" <<EOF
#!/bin/bash
exit 0
EOF
  chmod +x "$MOCK_DIR/esptool"
}

# 1. Success Path
setup_mocks
export DEBUG=true
bash "$SCRIPT" /dev/ttyTEST

# 2. Release Not Found
cat > "$MOCK_DIR/curl" <<EOF
#!/bin/bash
echo "{}"
EOF
bash "$SCRIPT" /dev/ttyTEST || true

# 3. Log functions & Edge cases
source "$SCRIPT"
log "INFO" "Normal log"
log "WARN" "Warn log"
log "ERRO" "Error log"
log "DEBU" "Debug log"
log "UNKNOWN" "Unknown log"
DEBUG=false log "DEBU" "Should not see"
echo "piped log" | log "INFO"
echo "piped debug" | log "DEBU"
echo "piped debug hidden" | DEBUG=false log "DEBU"

# 4. Dependency failure (Needs to be a separate run to avoid PATH issues)
# We'll do it by mocking commandExists or just renaming esptool
mv "$MOCK_DIR/esptool" "$MOCK_DIR/esptool_hidden"
bash "$SCRIPT" /dev/ttyTEST || true

# Cleanup
rm -rf "$MOCK_DIR"
