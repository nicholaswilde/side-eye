#!/bin/bash
# Test for firmware/scripts/flash.sh basic structure

SCRIPT="firmware/scripts/flash.sh"

if [ ! -f "$SCRIPT" ]; then
    echo "FAIL: $SCRIPT does not exist"
    exit 1
fi

# Check for shebang
head -n 1 "$SCRIPT" | grep -q "^#!/bin/bash" || { echo "FAIL: Missing shebang"; exit 1; }

# Check for color constants
grep -q "BLUE=" "$SCRIPT" || { echo "FAIL: Missing BLUE constant"; exit 1; }
grep -q "RED=" "$SCRIPT" || { echo "FAIL: Missing RED constant"; exit 1; }
grep -q "YELLOW=" "$SCRIPT" || { echo "FAIL: Missing YELLOW constant"; exit 1; }
grep -q "RESET=" "$SCRIPT" || { echo "FAIL: Missing RESET constant"; exit 1; }

# Check for log function
grep -q "function log()" "$SCRIPT" || grep -q "log()" "$SCRIPT" || { echo "FAIL: Missing log function"; exit 1; }

echo "PASS: $SCRIPT has basic structure and constants"
exit 0
