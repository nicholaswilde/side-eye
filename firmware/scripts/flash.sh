#!/bin/bash
# SideEye Firmware Flash Script

# Colors
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
RESET='\033[0m'

# Utility function for logging
function log() {
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

# Check if a command exists
function commandExists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for required dependencies
function check_dependencies() {
    local dependencies=("curl" "grep" "unzip" "esptool")
    local missing=()

    for dep in "${dependencies[@]}"; do
        if ! commandExists "$dep"; then
            missing+=("$dep")
        fi
    done

    if [ ${#missing[@]} -ne 0 ]; then
        log "ERRO" "Missing required dependencies: ${missing[*]}"
        log "INFO" "Please install them and try again."
        exit 1
    fi
}

# Configuration
DRY_RUN=false
VERSION=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        v*)
            VERSION=$1
            shift
            ;;
        *)
            log "WARN" "Unknown argument: $1"
            shift
            ;;
    esac
done
