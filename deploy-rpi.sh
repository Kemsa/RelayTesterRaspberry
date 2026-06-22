#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-rpi"
LOCAL_BINARY="${BUILD_DIR}/relayTester"

RPI_USER="${RPI_USER:-pi}"
RPI_HOST="${RPI_HOST:-192.168.1.11}"
RPI_PORT="${RPI_PORT:-22}"
RPI_REMOTE_DIR="${RPI_REMOTE_DIR:-/home/pi/relayTester}"
RPI_SSH_KEY="${RPI_SSH_KEY:-}"

# Load optional VS Code deploy config if present
DEPLOY_ENV_FILE="${SCRIPT_DIR}/.vscode/rpi-deploy.env"
if [ -f "${DEPLOY_ENV_FILE}" ]; then
  # shellcheck disable=SC1090
  source "${DEPLOY_ENV_FILE}"
fi

# Expand ~ in SSH key path if provided
if [ -n "${RPI_SSH_KEY}" ]; then
  RPI_SSH_KEY="${RPI_SSH_KEY/#\~/$HOME}"
fi

# Setup SSH options
SSH_OPTS=(
  -p "${RPI_PORT}"
  -o StrictHostKeyChecking=no
  -o UserKnownHostsFile=/dev/null
)

if [ -n "${RPI_SSH_KEY}" ]; then
  SSH_OPTS+=( -i "${RPI_SSH_KEY}" )
fi

# No cleanup needed

if [ ! -f "${LOCAL_BINARY}" ]; then
  echo "Error: compiled binary not found: ${LOCAL_BINARY}"
  echo "Run ./build-rpi-docker.sh first, then retry."
  exit 1
fi

if [ ! -x "${LOCAL_BINARY}" ]; then
  echo "Warning: local binary is not executable on this host, but it will be made executable on the Raspberry Pi."
fi

echo "Deploying ${LOCAL_BINARY} to ${RPI_USER}@${RPI_HOST}:${RPI_REMOTE_DIR}"

# Create remote directory
echo "[1/3] Creating remote directory..."
if ! ssh "${SSH_OPTS[@]}" "${RPI_USER}@${RPI_HOST}" mkdir -p "${RPI_REMOTE_DIR}"; then
  echo "Error: Failed to create remote directory"
  exit 1
fi

# Copy binary
echo "[2/3] Copying binary..."
if ! scp -P "${RPI_PORT}" -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null \
  "${LOCAL_BINARY}" "${RPI_USER}@${RPI_HOST}:${RPI_REMOTE_DIR}/"; then
  echo "Error: Failed to copy binary"
  exit 1
fi

# Make executable
echo "[3/3] Making executable..."
if ! ssh "${SSH_OPTS[@]}" "${RPI_USER}@${RPI_HOST}" chmod +x "${RPI_REMOTE_DIR}/relayTester"; then
  echo "Error: Failed to make executable"
  exit 1
fi

echo "Deployment complete."
echo "Remote executable path: ${RPI_REMOTE_DIR}/relayTester"
