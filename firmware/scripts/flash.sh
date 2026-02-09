#!/bin/bash
# SideEye Firmware Flash Script

# Colors
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
RESET='\033[0m'

# Utility function for logging
log() {
    local level=$1
    local msg=$2
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    case $level in
        "INFO") echo -e "${BLUE}[$timestamp] INFO:${RESET} $msg" ;;
        "WARN") echo -e "${YELLOW}[$timestamp] WARN:${RESET} $msg" ;;
        "ERRO") echo -e "${RED}[$timestamp] ERRO:${RESET} $msg" ;;
        *) echo -e "[$timestamp] $msg" ;;
    esac
}
