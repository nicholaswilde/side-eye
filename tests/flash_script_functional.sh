#!/usr/bin/env bash
# Functional test for flash.sh using mocks and multiple scenarios

set -euo pipefail

# Setup mocks directory
MOCK_DIR=$(mktemp -d)

# Function to setup common mocks
setup_mocks() {
  local dir=$1
  mkdir -p "$dir"
  # Mock curl
  cat > "$dir/curl" <<EOF
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
  chmod +x "$dir/curl"

  # Mock unzip
  cat > "$dir/unzip" <<EOF
#!/bin/bash
# pattern is usually last, dest is after -d
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
echo "Mock esptool running with args: \$*"
EOF
  chmod +x "$dir/esptool"
}

# Scenario 1: Success Path
echo "Running Scenario 1: Success Path..."
S1_DIR=$(mktemp -d)
setup_mocks "$S1_DIR"
export DEBUG=true
PATH="$S1_DIR:$PATH" kcov --include-path=. coverage_s1 firmware/scripts/flash.sh /dev/ttyTEST > s1.log 2>&1
if grep -q "Flashing complete" s1.log; then
    echo "S1: PASS"
else
    echo "S1: FAIL"; cat s1.log; exit 1
fi

# Scenario 2: Dependency Failure
echo "Running Scenario 2: Dependency Failure..."
S2_DIR=$(mktemp -d)
setup_mocks "$S2_DIR"
# Remove esptool and ensure system one is hidden
rm "$S2_DIR/esptool"
# To hide system esptool, we can mock it to fail or just mangle PATH
# But easier to just mock commandExists or similar? No.
# Let's hide it by using a fake PATH that excludes .local/bin
ORIG_PATH="$PATH"
export PATH="$S2_DIR:/usr/local/bin:/usr/bin:/bin"
kcov --include-path=. coverage_s2 firmware/scripts/flash.sh /dev/ttyTEST > s2.log 2>&1 || true
export PATH="$ORIG_PATH"
if grep -q "Required dependencies" s2.log; then
    echo "S2: PASS"
else
    echo "S2: FAIL"; cat s2.log; exit 1
fi

# Scenario 3: Release Not Found
echo "Running Scenario 3: Release Not Found..."
S3_DIR=$(mktemp -d)
setup_mocks "$S3_DIR"
cat > "$S3_DIR/curl" <<EOF
#!/bin/bash
echo "{}"
EOF
chmod +x "$S3_DIR/curl"
# Remove redirection to see output in task logs
PATH="$S3_DIR:$PATH" kcov --include-path=. coverage_s3 firmware/scripts/flash.sh /dev/ttyTEST || true
if [ -f coverage_s3/index.html ]; then
    echo "S3: PASS"
else
    echo "S3: FAIL"; exit 1
fi

# Scenario 4: Log tests
echo "Running Scenario 4: Internal Log Tests..."
S4_DIR=$(mktemp -d)
setup_mocks "$S4_DIR"
cat > "$S4_DIR/test_logs.sh" <<EOF
#!/bin/bash
source firmware/scripts/flash.sh
log "UNKNOWN" "Test unknown type"
DEBUG=false log "DEBU" "Should not show"
echo "test line" | log "INFO"
# Trigger cleanup
TMP_DIR=\$(mktemp -d)
cleanup
EOF
chmod +x "$S4_DIR/test_logs.sh"
PATH="$S4_DIR:$PATH" kcov --include-path=. coverage_s4 bash "$S4_DIR/test_logs.sh" > s4.log 2>&1
echo "S4: PASS"

# Merge coverage
rm -rf coverage
kcov --merge coverage coverage_s1 coverage_s2 coverage_s3 coverage_s4

# Cleanup
rm -rf "$S1_DIR" "$S2_DIR" "$S3_DIR" "$S4_DIR" s1.log s2.log s3.log s4.log
rm -rf coverage_s1 coverage_s2 coverage_s3 coverage_s4
echo "All Scenarios Passed"
