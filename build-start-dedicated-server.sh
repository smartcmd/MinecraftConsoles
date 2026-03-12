#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
COMPOSE_FILE="${SCRIPT_DIR}/docker-compose.dedicated-server.yml"
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

# can choose between release and debug, but honestly there isn’t much use
if [[ "${#}" -gt 1 ]]; then
  echo "Usage: $0 [release|debug|<runtime-dir-in-repo>]" >&2
  exit 1
fi

if [[ "${#}" -eq 1 ]]; then
  case "${1}" in
    release)
      MC_RUNTIME_DIR_INPUT="x64/Minecraft.Server/Release"
      ;;
    debug)
      MC_RUNTIME_DIR_INPUT="x64/Minecraft.Server/Debug"
      ;;
    *)
      MC_RUNTIME_DIR_INPUT="${1}"
      ;;
  esac
else
  if [[ -f "${SCRIPT_DIR}/x64/Minecraft.Server/Release/Minecraft.Server.exe" ]]; then
    MC_RUNTIME_DIR_INPUT="x64/Minecraft.Server/Release"
  else
    MC_RUNTIME_DIR_INPUT="x64/Minecraft.Server/Debug"
  fi
fi

if [[ "${MC_RUNTIME_DIR_INPUT}" = /* ]]; then
  if [[ "${MC_RUNTIME_DIR_INPUT}" != "${SCRIPT_DIR}"/* ]]; then
    echo "[error] runtime-dir must be inside repository: ${MC_RUNTIME_DIR_INPUT}" >&2
    exit 1
  fi
  MC_RUNTIME_DIR_REL="${MC_RUNTIME_DIR_INPUT#${SCRIPT_DIR}/}"
else
  MC_RUNTIME_DIR_REL="${MC_RUNTIME_DIR_INPUT#./}"
fi

if [[ -z "${MC_RUNTIME_DIR_REL}" || "${MC_RUNTIME_DIR_REL}" = "." ]]; then
  echo "[error] runtime-dir is invalid: ${MC_RUNTIME_DIR_INPUT}" >&2
  exit 1
fi

if ! RUNTIME_ABS="$(cd "${SCRIPT_DIR}" && cd "${MC_RUNTIME_DIR_REL}" 2>/dev/null && pwd)"; then
  echo "[error] runtime-dir not found: ${MC_RUNTIME_DIR_INPUT}" >&2
  exit 1
fi

if [[ "${RUNTIME_ABS}" != "${SCRIPT_DIR}"/* ]]; then
  echo "[error] runtime-dir must resolve inside repository: ${MC_RUNTIME_DIR_INPUT}" >&2
  exit 1
fi

MC_RUNTIME_DIR_REL="${RUNTIME_ABS#${SCRIPT_DIR}/}"

if [[ ! -f "${RUNTIME_ABS}/Minecraft.Server.exe" ]]; then
  echo "[error] Minecraft.Server.exe not found in: ${RUNTIME_ABS}" >&2
  echo "[hint] Build dedicated server first, then retry." >&2
  exit 1
fi

echo "[info] Using runtime (build arg): ${MC_RUNTIME_DIR_REL}"
echo "[info] Persistent data dir: ${PERSIST_DIR}"
MC_RUNTIME_DIR="${MC_RUNTIME_DIR_REL}" "${COMPOSE_CMD[@]}" -f "${COMPOSE_FILE}" up -d --build "${SERVICE_NAME}"
echo "[info] Dedicated server started."
