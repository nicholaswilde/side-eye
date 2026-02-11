#!/usr/bin/env bash
# Test for firmware/scripts/flash.sh argument parsing and version support

set -euo pipefail

SCRIPT="firmware/scripts/flash.sh"
MOCK_DIR=$(mktemp -d)

setup_mocks() {
  local dir=$1
  mkdir -p "$dir"
  # Mock curl to return specific version data
  cat > "$dir/curl" <<EOF
#!/bin/bash
if [[ "\$*" == *"releases/tags/v0.1.2"* ]]; then
  echo '{"tag_name": "v0.1.2", "assets": [{"browser_download_url": "https://example.com/side-eye-0.1.2-firmware.zip"}]}'
elif [[ "\$*" == *"releases/latest"* ]]; then
  echo '{"tag_name": "v0.1.3", "assets": [{"browser_download_url": "https://example.com/side-eye-0.1.3-firmware.zip"}]}'
else
  # Check if the last argument is a file path (for -o)
  last_arg="\${@: -1}"
  if [[ "\$last_arg" == *.zip ]]; then
    touch "\$last_arg"
  fi
fi
EOF
  chmod +x "$dir/curl"

  # Mock unzip
  cat > "$dir/unzip" <<EOF
#!/bin/bash
# unzip -o <zip> -d <dest> <pattern>
# \$1: -o, \$2: <zip>, \$3: -d, \$4: <dest>
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
echo "Mock esptool running"
EOF
  chmod +x "$dir/esptool"
}

setup_mocks "$MOCK_DIR"
export PATH="$MOCK_DIR:$PATH"

echo "Test 1: Specific version argument..."
if bash "$SCRIPT" v0.1.2 /dev/ttyTEST > test1.log 2>&1; then
    if grep -q "v0.1.2" test1.log; then
        echo "Test 1 PASS"
    else
        echo "Test 1 FAIL: Version v0.1.2 not found in logs"; cat test1.log; exit 1
    fi
else
    echo "Test 1 FAIL: Script failed"; cat test1.log; exit 1
fi

echo "Test 2: --dry-run flag..."
if bash "$SCRIPT" --dry-run /dev/ttyTEST > test2.log 2>&1; then
    if grep -q "Dry-run mode enabled" test2.log && ! grep -q "Mock esptool running" test2.log; then
        echo "Test 2 PASS"
    else
        echo "Test 2 FAIL: Dry-run not handled correctly"; cat test2.log; exit 1
    fi
else
    echo "Test 2 FAIL: Script failed"; cat test2.log; exit 1
fi

rm -rf "$MOCK_DIR" test1.log test2.log
echo "All Arg Tests Passed"
