# zmk-config-roBa

ZMK firmware configuration repository for the roBa split keyboard.

## Architecture

- Uses ZMK `v0.3-branch` (before the Zephyr 4.1 transition).
- Board: Seeeduino XIAO BLE (`seeeduino_xiao_ble`).
- Trackball: PMW3610 driver from `kumamuk-git/zmk-pmw3610-driver`.

## Important files

| File | Purpose |
| --- | --- |
| `config/west.yml` | ZMK manifest, branch, and dependency definitions |
| `config/roBa.keymap` | Keymap definition |
| `config/roBa.json` | ZMK Studio metadata |
| `build.yaml` | GitHub Actions build matrix |
| `.github/workflows/build.yml` | CI workflow |
| `boards/` | Custom board definitions |
| `roba_module/` | Per-Bluetooth-profile layout and connection-policy behavior |

## CI and builds

- CI calls the reusable workflow provided by `zmkfirmware/zmk`.
- The workflow reference in `build.yml` and the revision in `west.yml` must point to the same branch (`v0.3-branch`).
  - A mismatch can make the build fail because of differences such as board-name formats.
  - On 2026-03-05, CI broke because it referenced `@main`: Zephyr 4.1 changed the board name to `seeeduino_xiao_ble/nrf52840`, while `v0.3-branch` still uses `seeeduino_xiao_ble`.
- The board name in `build.yaml` is **`seeeduino_xiao_ble`** without a qualifier. This is the correct format for `v0.3-branch`.
