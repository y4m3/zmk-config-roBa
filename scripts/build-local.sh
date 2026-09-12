#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
image="${ZMK_IMAGE:-docker.io/zmkfirmware/zmk-dev-arm:3.5-branch}"
workspace_dir="${ZMK_DOCKER_WORKSPACE:-${repo_root}/.zmk-docker}"
artifact_dir="${repo_root}/artifacts"
target="${1:-all}"

if command -v podman >/dev/null 2>&1; then
    container_engine="podman"
elif command -v docker >/dev/null 2>&1; then
    container_engine="docker"
else
    echo "Podman or Docker is required." >&2
    exit 1
fi

case "${target}" in
    all|roBa_R|roBa_L|settings_reset) ;;
    *)
        echo "Usage: $0 [all|roBa_R|roBa_L|settings_reset]" >&2
        exit 2
        ;;
esac

mkdir -p "${workspace_dir}" "${artifact_dir}"
"${container_engine}" pull "${image}"

"${container_engine}" run --rm -i \
    -v "${repo_root}:/src:Z" \
    -v "${workspace_dir}:/work:Z" \
    -w /work \
    "${image}" \
    bash -s -- "${target}" <<'CONTAINER_SCRIPT'
set -euo pipefail

target="$1"

rm -rf /work/config
mkdir -p /work/config
cp -a /src/config/. /work/config/

if [ ! -d /work/.west ]; then
    west init -l /work/config
fi

if [ "${ZMK_UPDATE:-0}" = "1" ] || [ ! -d /work/zmk/app ]; then
    west update --fetch-opt=--filter=tree:0
fi
west zephyr-export

build_one() {
    local shield="$1"
    local build_dir="/work/build/${shield}"
    local snippet_args=()

    if [ "${shield}" = "roBa_R" ]; then
        snippet_args=(-S studio-rpc-usb-uart)
    fi

    west build -s zmk/app \
        -d "${build_dir}" \
        -b seeeduino_xiao_ble \
        "${snippet_args[@]}" \
        -- \
        -DSHIELD="${shield}" \
        -DZMK_CONFIG=/work/config \
        -DZMK_EXTRA_MODULES=/src

    cp "${build_dir}/zephyr/zmk.uf2" "/src/artifacts/${shield}.uf2"
}

case "${target}" in
    all)
        build_one roBa_R
        build_one roBa_L
        build_one settings_reset
        ;;
    *)
        build_one "${target}"
        ;;
esac
CONTAINER_SCRIPT

echo "Build artifacts are in ${artifact_dir}"
