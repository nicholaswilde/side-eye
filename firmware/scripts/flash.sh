#!/usr/bin/env bash
# ==============================================================================
#
# flash.sh
# -------
# Downloads the latest release from GitHub to the /tmp directory,
# extracts the bin files to the /tmp directory, checks if esptool is
# installed, and flashes the device.
#
# Usage: ./flash_latest.sh <SERIAL_PORT
#
# @author Nicholas Wilde, 0xb299a622                                                        
# @date 07 Aug 2025  
# @version 0.1.0
#
# ==============================================================================

set -euo pipefail

# --- variables ---
GITHUB_REPO="nicholaswilde/side-eye"
SERIAL_PORT="${1:-/dev/ttyACM0}"
DEBUG="${DEBUG:-false}"
TMP_DIR=""

# --- Constants ---
readonly BLUE=$(tput setaf 4)
readonly RED=$(tput setaf 1)
readonly YELLOW=$(tput setaf 3)
readonly PURPLE=$(tput setaf 5)
readonly RESET=$(tput sgr0)

readonly SCRIPT_NAME=$(basename "$0")

# Source .env file if it exists
if [ -f "$(dirname "$0")/../../.env" ]; then
  # shellcheck source=/dev/null
  source "$(dirname "$0")/../../.env"
fi

# --- functions ---

# Cleanup function
function cleanup() {
  if [[ -n "${TMP_DIR:-}" && -d "${TMP_DIR}" ]]; then
    log "INFO" "Cleaning up temporary files..."
    rm -rf "${TMP_DIR}"
  fi
}

# Logging function
function log() {
  local type="$1"
  local message="${2:-}"
  local color="$RESET"

  if [ "${type}" = "DEBU" ] && [ "${DEBUG}" != "true" ]; then
    return 0
  fi

  case "$type" in
    INFO)
      color="$BLUE"
      ;;
    WARN)
      color="$YELLOW"
      ;;
    ERRO)
      color="$RED"
      ;;
    DEBU)
      color="$PURPLE"
      ;;
    *)
      type="LOGS"
      ;;
  esac

  local timestamp
  timestamp=$(date +'%Y-%m-%d %H:%M:%S')

  if [[ -n "${message}" ]]; then
    echo -e "${color}${type}${RESET}[${timestamp}] ${message}"
  else
    while IFS= read -r line; do
      echo -e "${color}${type}${RESET}[${timestamp}] ${line}"
    done
  fi
}

# Checks if a command exists.
function commandExists() {
  command -v "$1" >/dev/null 2>&1
}

function check_dependencies() {
  # --- check for dependencies ---
  if ! commandExists curl || ! commandExists grep || ! commandExists unzip || ! commandExists esptool ; then
    log "ERRO" "Required dependencies (curl, grep, unzip, esptool) are not installed." >&2
    exit 1
  fi  
}

function download_release(){
  local curl_args=()
  if [ -n "${GITHUB_TOKEN:-}" ]; then
    curl_args+=('-H' "Authorization: Bearer ${GITHUB_TOKEN}")
  fi
  
  RELEASE=$(curl -fsSL "${curl_args[@]}" "https://api.github.com/repos/${GITHUB_REPO}/releases/latest" | grep -o '"tag_name": *"[^"]*"' | cut -d '"' -f 4)
  log "INFO" "Latest release: ${RELEASE}"

  # --- get the latest release download URL ---
  log "INFO" "Fetching the latest release from ${GITHUB_REPO}..."
  LATEST_RELEASE_URL=$(curl -sL "${curl_args[@]}" "https://api.github.com/repos/${GITHUB_REPO}/releases/latest" |  grep "browser_download_url" | grep -o 'https://[^"]*' | grep -E '/side-eye-[0-9.]+-firmware\.zip$')

  if [ -z "${LATEST_RELEASE_URL}" ]; then
    log "ERRO" "Could not find the latest release zip file." >&2
    exit 1
  fi

  # --- download and extract the release ---
  TMP_DIR=$(mktemp -d)
  log "INFO" "Downloading latest release from ${LATEST_RELEASE_URL}..."
  curl -sL "${LATEST_RELEASE_URL}" -o "${TMP_DIR}/latest_release.zip"
}

function extract_files() {
  log "INFO" "Extracting bin files to ${TMP_DIR}..."
  unzip -o "${TMP_DIR}/latest_release.zip" -d "${TMP_DIR}" "*.bin" &> /dev/null
}

function flash_device() {
  log "INFO" "Ready to flash the device on port ${SERIAL_PORT}."

  esptool \
    --chip esp32c6 \
    --port "${SERIAL_PORT}" \
    --baud 460800 \
    --before default-reset \
    --after hard-reset \
    write-flash \
      --no-progress \
      -z \
      --flash-mode dio \
      --flash-freq 80m \
      --flash-size detect \
      0x0000 "${TMP_DIR}/bootloader.bin" \
      0x8000 "${TMP_DIR}/partitions.bin" \
      0x10000 "${TMP_DIR}/firmware.bin" 2>&1 | log "INFO"
}

# Downloads and flashes the latest release.
function main() {
  trap cleanup EXIT
  check_dependencies  
  download_release
  extract_files
  if [[ $DEBUG ]]; then
    find "${TMP_DIR}" -name "*.bin" -print
  fi
  flash_device
  log "INFO" "--- Flashing complete ---"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
  main "$@"
fi
