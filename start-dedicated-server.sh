#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
COMPOSE_FILE="${SCRIPT_DIR}/docker-compose.dedicated-server.ghcr.yml"
SERVICE_NAME="minecraft-lce-dedicated-server"
PERSIST_DIR="${SCRIPT_DIR}/server-data"

if [[ ! -f "${COMPOSE_FILE}" ]]; then
  echo "[error] docker-compose file not found: ${COMPOSE_FILE}" >&2
  exit 1
fi

if command -v docker >/dev/null 2>&1; then
  COMPOSE_CMD=(docker compose)
elif command -v docker-compose >/dev/null 2>&1; then
  COMPOSE_CMD=(docker-compose)
else
  echo "[error] docker compose is not available." >&2
  exit 1
fi

if [[ "${#}" -gt 1 ]]; then
  echo "Usage: $0 [--no-pull]" >&2
  exit 1
fi

DO_PULL=1
if [[ "${#}" -eq 1 ]]; then
  if [[ "${1}" == "--no-pull" ]]; then
    DO_PULL=0
  else
    echo "Usage: $0 [--no-pull]" >&2
    exit 1
  fi
fi

if [[ "${DO_PULL}" -eq 1 ]]; then
  echo "[info] Pulling latest image from GHCR..."
  "${COMPOSE_CMD[@]}" -f "${COMPOSE_FILE}" pull "${SERVICE_NAME}"
fi

echo "[info] Starting dedicated server..."
"${COMPOSE_CMD[@]}" -f "${COMPOSE_FILE}" up -d "${SERVICE_NAME}"
echo "[info] Dedicated server started."
