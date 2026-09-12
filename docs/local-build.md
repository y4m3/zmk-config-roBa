# Local Docker/Podman build

This repository is pinned to ZMK `v0.3-branch` and Zephyr 3.5. The supported local build method is the official ZMK ARM development container, pinned to the matching `3.5-branch` image.

The container includes the Zephyr SDK, `west`, CMake, Ninja, the ARM compiler, and the Devicetree tools. The host only needs Podman or Docker and Git. The first build downloads the container (approximately 1 GB) and the ZMK west dependencies; subsequent builds reuse both caches.

## Build everything

From the repository root:

```sh
./scripts/build-local.sh
```

This builds:

- `roBa_R` with the `studio-rpc-usb-uart` snippet
- `roBa_L`
- `settings_reset`

The UF2 files are written to `artifacts/`. The west workspace cache is stored in `.zmk-docker/`.

## Build one target

```sh
./scripts/build-local.sh roBa_R
./scripts/build-local.sh roBa_L
./scripts/build-local.sh settings_reset
```

The `settings_reset` image clears Bluetooth bonds and persistent settings. Flash it only when a settings reset is intentionally required, then flash the normal `roBa_R` and `roBa_L` images afterwards.

## Configuration

The default image is `docker.io/zmkfirmware/zmk-dev-arm:3.5-branch`. It can be overridden for testing:

```sh
ZMK_IMAGE=docker.io/zmkfirmware/zmk-dev-arm:3.5-branch-20250927025316-3.5.0-0.16.3-f93290b3ab1a-18054070147 \
  ./scripts/build-local.sh roBa_R
```

Set `ZMK_DOCKER_WORKSPACE` to move the west cache outside the repository:

```sh
ZMK_DOCKER_WORKSPACE=/var/tmp/roba-west ./scripts/build-local.sh
```

The script prefers Podman and falls back to Docker. Both use the same container command and repository mounts.

The west dependencies are updated automatically on the first build. Set `ZMK_UPDATE=1` when the manifest or a dependency revision changes:

```sh
ZMK_UPDATE=1 ./scripts/build-local.sh roBa_R
```
