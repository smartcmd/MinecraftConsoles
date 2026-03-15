FROM debian:bookworm-slim

ARG DEBIAN_FRONTEND=noninteractive
# basically, it only works with a Release build.(libs are not included in Debug build)
ARG MC_RUNTIME_DIR=x64/Minecraft.Server/Release

RUN dpkg --add-architecture i386 \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
    ca-certificates \
    wine \
    wine64 \
    wine32:i386 \
    xvfb \
    tini \
    && rm -rf /var/lib/apt/lists/*

ENV WINEARCH=win64
ENV WINEPREFIX=/var/opt/wineprefix64

WORKDIR /srv/mc

COPY ${MC_RUNTIME_DIR}/Minecraft.Server.exe /srv/mc/Minecraft.Server.exe
COPY ${MC_RUNTIME_DIR}/iggy_w64.dll /srv/mc/iggy_w64.dll
COPY ${MC_RUNTIME_DIR}/Common /srv/mc/Common
COPY ${MC_RUNTIME_DIR}/Windows64 /srv/mc/Windows64
COPY docker/dedicated-server/entrypoint.sh /usr/local/bin/entrypoint.sh

RUN chmod 0755 /usr/local/bin/entrypoint.sh \
    && mkdir -p /var/opt/wineprefix64 /srv/mc \
    && test -f /srv/mc/Minecraft.Server.exe

ENTRYPOINT ["/usr/bin/tini", "--", "/usr/local/bin/entrypoint.sh"]
