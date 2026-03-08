# Minecraft.Server 開発ガイド (日本語)

このドキュメントは、`Minecraft.Server` の内部構成を知らない人でも、機能追加や修正を安全に進められるようにまとめた実践ガイドです。

## 1. Minecraft.Server の役割

`Minecraft.Server` は本リポジトリの Dedicated Server 実行エントリです。

主な責務:
- `server.properties` の読み込みと正規化
- Windows/Network/Runtime の初期化
- ワールドのロードまたは新規作成
- メインループ実行 (ネットワーク進行、XUIアクション、オートセーブ、CLI処理)
- 安全な保存とシャットダウン

## 2. 重要ファイル

### 起動と実行ループ
- `Windows64/ServerMain.cpp`
  - エントリポイント `main`
  - 引数パース
  - 初期化から終了までの実行フロー
  - メインループとオートセーブ

### ワールド選択とセーブ読み込み
- `WorldManager.h`
- `WorldManager.cpp`
  - `level-id` 優先でセーブ探索
  - 見つからない場合は world 名でフォールバック
  - 非同期完了待機ヘルパー

### サーバー設定
- `ServerProperties.h`
- `ServerProperties.cpp`
  - 既定値定義
  - `server.properties` のパース/正規化/保存
  - `ServerPropertiesConfig` の提供

### ログ出力
- `ServerLogger.h`
- `ServerLogger.cpp`
  - ログレベル解釈
  - タイムスタンプ付き色付きログ
  - カテゴリ別ログ (`startup`, `world-io`, `console`)

### コンソールコマンド
- `Console/ServerCli.cpp` (ファサード)
- `Console/ServerCliInput.cpp` (linenoise 入力スレッド)
- `Console/ServerCliParser.cpp` (トークン分解、クォート、補完コンテキスト)
- `Console/ServerCliEngine.cpp` (実行ディスパッチ、補完、共通ヘルパー)
- `Console/ServerCliRegistry.cpp` (登録と名前解決)
- `Console/commands/*` (各コマンド実装)

## 3. 起動フロー全体

`Windows64/ServerMain.cpp` の流れ:
1. `LoadServerPropertiesConfig()` で設定読込
2. CLI 引数で上書き (`-port`, `-bind`, `-name`, `-seed`, `-loglevel`)
3. 各サブシステム初期化 (window/device/profile/network/thread storage)
4. `ServerPropertiesConfig` をゲームホスト設定へ反映
5. `BootstrapWorldForServer(...)` でワールド決定
6. `RunNetworkGameThreadProc` でサーバーゲーム開始
7. メインループ:
   - `TickCoreSystems()`
   - `HandleXuiActions()`
   - `serverCli.Poll()`
   - オートセーブスケジュール
8. 終了時:
   - Action Idle 待機
   - 最終保存要求
   - サーバー停止と各サブシステム終了

## 4. よくある開発作業

### 4.1 CLI コマンドを追加する

`/kick` や `/time` のようなコマンド追加時の基本手順:

1. `Console/commands/` にファイル追加
   - `CliCommandYourCommand.h`
   - `CliCommandYourCommand.cpp`
2. `IServerCliCommand` を実装
   - `Name()`, `Usage()`, `Description()`, `Execute(...)`
   - 必要なら `Aliases()` と `Complete(...)`
3. `ServerCliEngine::RegisterDefaultCommands()` に登録
4. ビルド定義に追加
   - `CMakeLists.txt` (`MINECRAFT_SERVER_SOURCES`)
   - `Minecraft.Server/Minecraft.Server.vcxproj` (`<ClCompile>`, `<ClInclude>`)
5. 手動確認
   - `help` に表示される
   - 実行が期待通り
   - 補完が `cmd` と `/cmd` の両方で動く

参考実装:
- `CliCommandHelp.cpp` (単純コマンド)
- `CliCommandTp.cpp` (複数引数 + 補完 + 実行時チェック)
- `CliCommandGamemode.cpp` (引数解釈 + モード検証)

### 4.2 `server.properties` キーを追加/変更する

1. `ServerProperties.h` の `ServerPropertiesConfig` にフィールド追加
2. `ServerProperties.cpp` の `kServerPropertyDefaults` に既定値追加
3. `LoadServerPropertiesConfig()` で読み込みと正規化を実装
   - 既存の read helper を利用 (bool/int/string/int64/log level)
4. 保存時に維持したい値なら `SaveServerPropertiesConfig()` も更新
5. 実行時反映箇所を更新
   - `ApplyServerPropertiesToDedicatedConfig(...)`
   - `ServerMain.cpp` の `app.SetGameHostOption(...)` など
6. 手動確認
   - キー欠損時の自動補完
   - 不正値の正規化
   - 実行時挙動への反映

### 4.3 ワールドロード/新規作成ロジックを変更する

主な実装は `WorldManager.cpp` にあります。

現在の探索ポリシー:
1. `level-id` (`UTF8SaveFilename`) 完全一致を優先
2. 失敗時に world 名一致へフォールバック

変更時の注意:
- `ApplyWorldStorageTarget(...)` で title と save ID を常にセットで扱う
- 待機ループで `tickProc` を回し続ける
  - これを止めると非同期進行が止まり、タイムアウトしやすくなる
- タイムアウトや失敗ログは具体的に残す
- 確認項目
  - 既存ワールドを正しく再利用できるか
  - 意図しない新規セーブ先が増えていないか
  - 終了時保存が成功するか

### 4.4 ログを追加する

`ServerLogger` の API を利用:
- `LogDebug`, `LogInfo`, `LogWarn`, `LogError`
- フォーマット付きは `LogInfof` など

カテゴリ指針:
- `startup`: 起動/終了手順
- `world-io`: ワールドと保存処理
- `console`: CLI 入出力とコマンド処理

## 5. ビルドと実行

リポジトリルートで実行:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target MinecraftServer
cd .\build\Debug
.\Minecraft.Server.exe -port 25565 -bind 0.0.0.0 -name DedicatedServer
```

補足:
- 実行ディレクトリ基準で相対パス解決するため、出力ディレクトリから起動する
- `server.properties` はカレントディレクトリから読み込まれる
- Visual Studio の運用はルートの `COMPILE.md` を参照

## 6. 変更前チェックリスト

- `server.properties` が空または欠損でも起動できる
- 既存ワールドが期待した `level-id` でロードされる
- 新規ワールド作成時の初回保存が実行される
- CLI 入力と補完が正常に動く
- 非同期待機ループから `TickCoreSystems()` などを消していない
- 新規追加したソースが CMake と `.vcxproj` の両方に入っている

## 7. トラブルシュート

- コマンドが認識されない:
  - `RegisterDefaultCommands()` とビルド定義を確認
- オートセーブ/終了時保存がタイムアウトする:
  - 待機中に `TickCoreSystems()` と `HandleXuiActions()` を回しているか確認
- 再起動時に同じワールドを使わない:
  - `level-id` の正規化と `WorldManager.cpp` の一致判定を確認
- 設定変更が効かない:
  - `ServerPropertiesConfig` へのロードと `ServerMain.cpp` の反映経路を確認
