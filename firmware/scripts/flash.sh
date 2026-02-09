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
GITHUB_REPO="nicholaswilde/side-eye"

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

# Download the firmware release
function download_release() {
    local version=$1
    local download_url
    local latest_tag

    # Create temp directory
    TMP_DIR=$(mktemp -d)
    log "INFO" "Created temporary directory: $TMP_DIR"

    if [ -z "$version" ]; then
        log "INFO" "Fetching latest release tag..."
        latest_tag=$(curl -s "https://api.github.com/repos/$GITHUB_REPO/releases/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')
        
        if [ -z "$latest_tag" ]; then
            log "ERRO" "Failed to fetch latest release tag."
            exit 1
        fi
        
        log "INFO" "Latest release: $latest_tag"
        version=$latest_tag
    fi

    # Construct download URL (assuming standard naming convention for firmware zip)
    # Note: SideEye releases contain side-eye-<version>-firmware.zip
    download_url="https://github.com/$GITHUB_REPO/releases/download/$version/side-eye-${version}-firmware.zip"
    
    log "INFO" "Downloading firmware from: $download_url"
    
    if [ "$DRY_RUN" = false ]; then
        curl -L -o "$TMP_DIR/firmware.zip" "$download_url"
        if [ $? -ne 0 ]; then
            log "ERRO" "Download failed."
            exit 1
        fi
        log "INFO" "Download complete."
    else
        log "INFO" "[DRY RUN] Would download: $download_url to $TMP_DIR/firmware.zip"
    fi
}
