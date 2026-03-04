# zmk-config-roBa

roBa キーボード（分割キーボード）の ZMK ファームウェア設定リポジトリ。

## アーキテクチャ

- ZMK `v0.3-branch`（Zephyr 4.1 以前）を使用
- ボード: Seeeduino XIAO BLE (`seeeduino_xiao_ble`)
- トラックボール: PMW3610 ドライバ（kumamuk-git/zmk-pmw3610-driver）

## 重要ファイル

| ファイル | 役割 |
|---------|------|
| `config/west.yml` | ZMK マニフェスト（使用ブランチ・依存定義） |
| `config/roBa.keymap` | キーマップ定義 |
| `config/roBa.json` | ZMK Studio 用メタデータ |
| `build.yaml` | GitHub Actions ビルドマトリクス |
| `.github/workflows/build.yml` | CI ワークフロー |
| `boards/` | カスタムボード定義 |

## CI / ビルド

- CI は `zmkfirmware/zmk` の **reusable workflow** を呼び出している
- `build.yml` のワークフロー参照と `west.yml` の revision は **同じブランチ（`v0.3-branch`）** を指すこと
  - 不一致になると、ボード名の形式差異などでビルドが失敗する
  - 2026-03-05 に `@main` を参照していたため CI が壊れた（Zephyr 4.1 でボード名が `seeeduino_xiao_ble/nrf52840` 形式に変更されたが、v0.3-branch では `seeeduino_xiao_ble` のまま）
- `build.yaml` のボード名は **`seeeduino_xiao_ble`**（qualifier なし）。v0.3-branch ではこの形式が正しい
