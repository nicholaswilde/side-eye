#!/usr/bin/env bash
# Test for firmware/scripts/flash.sh basic structure

SCRIPT="firmware/scripts/flash.sh"

if [ ! -f "$SCRIPT" ]; then
    echo "FAIL: $SCRIPT does not exist"
    exit 1
fi

# Check for shebang
head -n 1 "$SCRIPT" | grep -q "^#!/usr/bin/env bash" || { echo "FAIL: Missing or incorrect shebang"; exit 1; }

# Check for color constants
grep -q "BLUE=" "$SCRIPT" || { echo "FAIL: Missing BLUE constant"; exit 1; }
grep -q "RED=" "$SCRIPT" || { echo "FAIL: Missing RED constant"; exit 1; }
grep -q "YELLOW=" "$SCRIPT" || { echo "FAIL: Missing YELLOW constant"; exit 1; }
grep -q "PURPLE=" "$SCRIPT" || { echo "FAIL: Missing PURPLE constant"; exit 1; }
grep -q "RESET=" "$SCRIPT" || { echo "FAIL: Missing RESET constant"; exit 1; }

# Check for functions
grep -q "function log()" "$SCRIPT" || { echo "FAIL: Missing function keyword for log"; exit 1; }
grep -q "function check_dependencies()" "$SCRIPT" || { echo "FAIL: Missing function keyword for check_dependencies"; exit 1; }
grep -q "function download_release()" "$SCRIPT" || { echo "FAIL: Missing function keyword for download_release"; exit 1; }
grep -q "function extract_files()" "$SCRIPT" || { echo "FAIL: Missing function keyword for extract_files"; exit 1; }
grep -q "function flash_device()" "$SCRIPT" || { echo "FAIL: Missing function keyword for flash_device"; exit 1; }
grep -q "function cleanup()" "$SCRIPT" || { echo "FAIL: Missing function keyword for cleanup"; exit 1; }

# Check for dependency checks
grep -q "curl" "$SCRIPT" || { echo "FAIL: Missing curl dependency check"; exit 1; }
grep -q "grep" "$SCRIPT" || { echo "FAIL: Missing grep dependency check"; exit 1; }
grep -q "unzip" "$SCRIPT" || { echo "FAIL: Missing unzip dependency check"; exit 1; }
grep -q "esptool" "$SCRIPT" || { echo "FAIL: Missing esptool dependency check"; exit 1; }

# Check for configuration
grep -q "GITHUB_REPO=" "$SCRIPT" || { echo "FAIL: Missing GITHUB_REPO constant"; exit 1; }
grep -q "SERIAL_PORT=" "$SCRIPT" || { echo "FAIL: Missing SERIAL_PORT initialization"; exit 1; }

echo "PASS: $SCRIPT has basic structure and constants"
exit 0
