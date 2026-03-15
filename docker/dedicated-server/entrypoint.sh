#!/usr/bin/env bash
set -euo pipefail

SERVER_DIR="/srv/mc"
SERVER_EXE="Minecraft.Server.exe"
# ip & port are fixed since they run inside the container
SERVER_PORT="25565"
SERVER_BIND_IP="0.0.0.0"

PERSIST_DIR="/srv/persist"
WINE_CMD=""

ensure_persist_file() {
  local persist_path="$1"
  local runtime_path="$2"
  local default_text="$3"

  if [ ! -f "${persist_path}" ]; then
    if [ -f "${runtime_path}" ] && [ ! -L "${runtime_path}" ]; then
      cp -f "${runtime_path}" "${persist_path}"
    else
      printf '%b' "${default_text}" > "${persist_path}"
    fi
  fi

  if [ -e "${runtime_path}" ] && [ ! -L "${runtime_path}" ]; then
    rm -f "${runtime_path}"
  fi

  ln -sfn "${persist_path}" "${runtime_path}"
}

if [ ! -d "$SERVER_DIR" ]; then
  echo "[error] Server directory not found: $SERVER_DIR" >&2
  exit 1
fi

cd "$SERVER_DIR"

if [ ! -f "$SERVER_EXE" ]; then
  echo "[error] ${SERVER_EXE} not found in ${SERVER_DIR}" >&2
  echo "[hint] Rebuild image with a valid MC_RUNTIME_DIR build arg that contains dedicated server runtime files." >&2
  exit 1
fi

mkdir -p "${PERSIST_DIR}"

# created because it is not implemented on the server side
mkdir -p "${PERSIST_DIR}/GameHDD"

ensure_persist_file "${PERSIST_DIR}/server.properties" "server.properties" ""
ensure_persist_file "${PERSIST_DIR}/banned-players.json" "banned-players.json" "[]\n"
ensure_persist_file "${PERSIST_DIR}/banned-ips.json" "banned-ips.json" "[]\n"

# differs from the structure, but it’s reorganized into a more manageable structure to the host side
if [ -e "Windows64/GameHDD" ] && [ ! -L "Windows64/GameHDD" ]; then
  rm -rf "Windows64/GameHDD"
fi
ln -sfn "${PERSIST_DIR}/GameHDD" "Windows64/GameHDD"

# for compatibility with other images
if command -v wine64 >/dev/null 2>&1; then
  WINE_CMD="wine64"
elif [ -x "/usr/lib/wine/wine64" ]; then
  WINE_CMD="/usr/lib/wine/wine64"
elif command -v wine >/dev/null 2>&1; then
  WINE_CMD="wine"
else
  echo "[error] No Wine executable found (wine64/wine)." >&2
  exit 1
fi

if [ ! -d "${WINEPREFIX}" ] || [ -z "$(ls -A "${WINEPREFIX}" 2>/dev/null)" ]; then
  mkdir -p "${WINEPREFIX}"
fi

# in the current implementation, a virtual screen is required because the client-side logic is being called for compatibility
if [ -z "${DISPLAY:-}" ]; then
  export DISPLAY="${XVFB_DISPLAY:-:99}"
  XVFB_SCREEN="${XVFB_SCREEN:-64x64x16}"
  Xvfb "${DISPLAY}" -nolisten tcp -screen 0 "${XVFB_SCREEN}" >/tmp/xvfb.log 2>&1 &
  sleep 0.2
fi

args=(
  -port "${SERVER_PORT}"
  -bind "${SERVER_BIND_IP}"
)

echo "[info] Starting ${SERVER_EXE} on ${SERVER_BIND_IP}:${SERVER_PORT}"
exec "${WINE_CMD}" "${SERVER_EXE}" "${args[@]}"
