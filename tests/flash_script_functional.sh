#!/usr/bin/env bash
# Functional test for flash.sh using mocks

# Setup mocks
MOCK_DIR=$(mktemp -d)
export PATH="$MOCK_DIR:$PATH"

# Mock curl
cat > "$MOCK_DIR/curl" <<EOF
#!/bin/bash
if [[ "\$*" == *"releases/latest"* ]]; then
  if [[ "\$*" == *"-fsSL"* ]]; then
    # JSON for RELEASE
    echo '{"tag_name": "v0.1.3", "assets": [{"browser_download_url": "https://example.com/side-eye-0.1.3-firmware.zip"}]}'
  else
    # Grepable text for LATEST_RELEASE_URL
    echo 'browser_download_url: https://example.com/side-eye-0.1.3-firmware.zip'
  fi
else
  # Handle download
  touch "\${@: -1}"
fi
EOF
chmod +x "$MOCK_DIR/curl"

# Mock unzip
cat > "$MOCK_DIR/unzip" <<EOF
#!/bin/bash
# Arguments: -o <zip> -d <dest> <pattern>
# $1: -o
# $2: <zip>
# $3: -d
# $4: <dest>
# $5: <pattern>
DEST="\$4"
mkdir -p "\$DEST"
touch "\$DEST/bootloader.bin"
touch "\$DEST/partitions.bin"
touch "\$DEST/firmware.bin"
EOF
chmod +x "$MOCK_DIR/unzip"

# Mock esptool
cat > "$MOCK_DIR/esptool" <<EOF
#!/bin/bash
echo "Mock esptool running with args: \$*"
EOF
chmod +x "$MOCK_DIR/esptool"

# Run the script in debug mode to see trace
export DEBUG=true
# Use kcov to run the script and generate coverage
kcov --include-pattern=firmware/scripts/flash.sh coverage bash firmware/scripts/flash.sh /dev/ttyTEST > test_output.log 2>&1

# Check results
if grep -q "Latest release: v0.1.3" test_output.log && 
   grep -q "Mock esptool running with args:" test_output.log && 
   grep -q "Flashing complete" test_output.log; then
    echo "PASS: Functional test successful with mocks"
else
    echo "FAIL: Functional test failed"
    cat test_output.log
    exit 1
fi

# Cleanup
rm -rf "$MOCK_DIR" test_output.log
exit 0
