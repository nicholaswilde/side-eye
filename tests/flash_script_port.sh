#!/usr/bin/env bash
# Test for firmware/scripts/flash.sh port auto-detection

set -euo pipefail

SCRIPT="firmware/scripts/flash.sh"
MOCK_DIR=$(mktemp -d)

setup_mocks() {
  local dir=$1
  mkdir -p "$dir"
  # Mock curl to return valid json
  cat > "$dir/curl" <<EOF
#!/bin/bash
echo '{"tag_name": "v0.1.3", "assets": [{"browser_download_url": "https://example.com/side-eye-0.1.3-firmware.zip"}]}'
EOF
  chmod +x "$dir/curl"

  # Mock unzip
  cat > "$dir/unzip" <<EOF
#!/bin/bash
DEST="\$4"
mkdir -p "\$DEST"
touch "\$DEST/bootloader.bin"
touch "\$DEST/partitions.bin"
touch "\$DEST/firmware.bin"
EOF
  chmod +x "$dir/unzip"

  # Mock esptool
  cat > "$dir/esptool" <<EOF
#!/bin/bash
echo "Mock esptool running on port: \$*"
EOF
  chmod +x "$dir/esptool"

  # Mock ls to return a specific port
  cat > "$dir/ls" <<EOF
#!/bin/bash
if [[ "\$*" == *"/dev/ttyACM*"* ]] || [[ "\$*" == *"/dev/ttyUSB*"* ]]; then
  echo "/dev/ttyUSB99"
  exit 0
fi
command ls "\$@"
EOF
  chmod +x "$dir/ls"
}

setup_mocks "$MOCK_DIR"
export PATH="$MOCK_DIR:$PATH"

echo "Test 1: Auto-detection of port /dev/ttyUSB99..."
if bash "$SCRIPT" --dry-run > test1.log 2>&1; then
    if grep -q "Ready to flash the device on port /dev/ttyUSB99" test1.log; then
        echo "Test 1 PASS"
    else
        echo "Test 1 FAIL: Port /dev/ttyUSB99 not detected"; grep "Ready to flash" test1.log || echo "No 'Ready to flash' log found"; cat test1.log; exit 1
    fi
else
    echo "Test 1 FAIL: Script failed"; cat test1.log; exit 1
fi

rm -rf "$MOCK_DIR" test1.log
echo "All Port Tests Passed"
