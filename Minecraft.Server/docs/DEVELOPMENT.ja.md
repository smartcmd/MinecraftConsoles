# Minecraft.Server 開発ガイド (日本語)

この文書は、`Minecraft.Server` に新しく入る開発者が、安全に機能追加や改修を行うための実践的な地図として使うことを想定しています

## 1. このサーバーが担うこと

`Minecraft.Server` は、このコードベースにおける専用サーバー実行ファイルのエントリーポイントです

主な責務:
- 相対パスのファイル I/O を行う前に、カレントディレクトリを実行ファイルのあるフォルダへ切り替える
- `server.properties` を読み込み、正規化し、不足や不正値を補完する
- 専用サーバー向けランタイム、接続ログ、アクセス制御を初期化する
- 対象ワールドをロードまたは新規作成し、実際のセーブ先と `level-id` を整合させる
- 専用サーバーのメインループを回す (network tick, XUI actions, autosave, CLI input)
- `banned-players.json` や `banned-ips.json` など運用向けファイルを維持する
- 新規ワールドの初回保存を実行し、その後安全にシャットダウンする

## 2. 重要ファイル

### 起動とランタイム
- `Windows64/ServerMain.cpp`
  - `PrintUsage()` と `ParseCommandLine()`
  - `SetExeWorkingDirectory()`
  - 起動/終了フロー
  - 新規ワールド初回保存の経路
  - メインループ、オートセーブ、CLI ポーリング

### ワールド選択とセーブ読込
- `WorldManager.h`
- `WorldManager.cpp`
  - `level-id` 優先、その後 world 名フォールバックでセーブ探索
  - storage title と save ID を常にセットで適用
  - 非同期 storage/server action 完了待ちの helper を提供

### サーバー設定
- `ServerProperties.h`
- `ServerProperties.cpp`
  - 既定値と正規化レンジ
  - `server.properties` の読込/補修/書込
  - `ServerPropertiesConfig` の提供
  - `SaveServerPropertiesConfig()` は `level-name` / `level-id` / `white-list` を書き換える

### アクセス制御と BAN / Whitelist 永続化
- `Access/Access.h`
- `Access/Access.cpp`
  - プロセス全体で使うアクセス制御 facade
  - コンソールコマンドとログイン判定から参照される公開スナップショット管理
- `Access/BanManager.h`
- `Access/BanManager.cpp`
  - `banned-players.json` と `banned-ips.json` の読込/書込
  - 識別子の正規化と、期限切れエントリを除いた snapshot 出力
- `Access/WhitelistManager.h`
- `Access/WhitelistManager.cpp`
  - `whitelist.json` の読込/書込
  - ログイン判定と CLI で使う XUID whitelist の正規化管理

### ログと接続監査
- `ServerLogger.h`
- `ServerLogger.cpp`
  - ログレベル解釈
  - 色付き/タイムスタンプ付きコンソールログ
  - `startup`, `world-io`, `console`, `access`, `network`, `shutdown` などのカテゴリ
- `ServerLogManager.h`
- `ServerLogManager.cpp`
  - TCP 接続 accept/reject ログ
  - ログイン/切断の監査ログ
  - `ban-ip <player>` が使う remote IP キャッシュ

### コンソールコマンドシステム
- `Console/ServerCli.cpp` (facade)
- `Console/ServerCliInput.cpp` (linenoise 入力スレッド + completion bridge)
- `Console/ServerCliParser.cpp` (トークン分解、クォート、補完コンテキスト)
- `Console/ServerCliEngine.cpp` (実行ディスパッチ、補完、共通ヘルパー)
- `Console/ServerCliRegistry.cpp` (登録と名前解決)
- `Console/commands/*` (各コマンド実装)

## 3. 起動フロー全体

`Windows64/ServerMain.cpp` の主な流れ:
1. `SetExeWorkingDirectory()` でカレントディレクトリを実行ファイルのフォルダへ切り替える
2. `LoadServerPropertiesConfig()` で `server.properties` を読み込み、正規化する
3. `DedicatedServerConfig` へ反映したあと、CLI 引数で上書きする (`-port`, `-ip`/`-bind`, `-name`, `-maxplayers`, `-seed`, `-loglevel`, `-help`/`--help`/`-h`)
4. プロセス状態、`ServerLogManager`、`Access::Initialize(".")` を初期化する
5. window/device/profile/network/thread-local 系を初期化する
6. `ServerPropertiesConfig` をゲームホスト設定へ反映する
7. `BootstrapWorldForServer(...)` でワールドを決定する
8. 読み込まれたセーブ ID が正規化後に変わった場合は、`SaveServerPropertiesConfig()` で書き戻す
9. `RunNetworkGameThreadProc` でホストゲームスレッドを起動する
10. 新規ワールドが作成された場合は、専用サーバー側で明示的に初回保存を要求する
11. メインループに入る:
   - `TickCoreSystems()`
   - `HandleXuiActions()`
   - `serverCli.Poll()`
   - オートセーブスケジュール
12. 終了時:
   - CLI 入力を停止
   - save-on-exit を要求してサーバー停止
   - ネットワーク停止完了を待機
   - ログ/アクセス制御/ネットワーク/デバイスを終了

## 4. 現在の運用インターフェース

### 4.1 起動引数
- `-port <1-65535>`
- `-ip <addr>` または `-bind <addr>`
- `-name <name>` (実行時上限 16 文字)
- `-maxplayers <1-8>`
- `-seed <int64>`
- `-loglevel <debug|info|warn|error>`
- `-help`, `--help`, `-h`

補足:
- CLI による上書きは、その起動中のプロセスにだけ効きます
- 現在サーバーが書き戻す値は `level-name` と `level-id` だけで、ワールド解決時に識別情報が変わった場合に限られます

### 4.2 組み込みコンソールコマンド
- `help` / `?`
- `stop`
- `list`
- `ban <player> [reason ...]`
  - 現状では対象プレイヤーがオンラインである必要があります
- `ban-ip <address|player> [reason ...]`
  - リテラル IPv4/IPv6 か、オンラインプレイヤーの現在 IP を対象にできます
- `pardon <player>`
- `pardon-ip <address>`
  - リテラルアドレスのみ受け付けます
- `banlist`
- `tp <player> <target>` / `teleport`
- `gamemode <survival|creative|0|1> [player]` / `gm`

CLI 挙動の補足:
- `cmd` と `/cmd` の両方を受け付けます
- `ServerCliParser` により引用符付き引数を扱えます
- 補完は各コマンドの `Complete(...)` で実装します

### 4.3 実行ファイル横に書かれるファイル
- `server.properties`
- `banned-players.json`
- `banned-ips.json`

これは `SetExeWorkingDirectory()` による挙動ですつまり、これらのファイルはシェル上の起動場所ではなく `Minecraft.Server.exe` 基準で解決されます

## 5. よくある開発作業

### 5.1 CLI コマンドを追加する

`/kick` や `/time` のようなコマンド追加時の基本手順:

1. `Console/commands/` にファイルを追加
   - `CliCommandYourCommand.h`
   - `CliCommandYourCommand.cpp`
2. `IServerCliCommand` を実装
   - `Name()`, `Usage()`, `Description()`, `Execute(...)`
   - 必要なら `Aliases()` と `Complete(...)`
3. `ServerCliEngine::RegisterDefaultCommands()` に登録する
4. ビルド定義に追加する
   - `CMakeLists.txt` (`MINECRAFT_SERVER_SOURCES`)
   - `Minecraft.Server/Minecraft.Server.vcxproj` (`<ClCompile>` / `<ClInclude>`)
5. 手動確認
   - `help` に表示される
   - 実行結果が期待通り
   - 補完が `cmd` と `/cmd` の両方で動く
   - 引用符付き引数が期待通り処理される

参考実装:
- `CliCommandHelp.cpp` (単純コマンド)
- `CliCommandTp.cpp` (複数引数 + 補完 + 実行時チェック)
- `CliCommandGamemode.cpp` (引数解釈 + エイリアス)
- `CliCommandBanIp.cpp` (接続メタデータを使うアクセス制御系コマンド)

### 5.2 `server.properties` キーを追加/変更する

1. `ServerProperties.h` の `ServerPropertiesConfig` にフィールドを追加/更新する
2. `ServerProperties.cpp` の `kServerPropertyDefaults` に既定値を追加する
3. `LoadServerPropertiesConfig()` で読み込みと正規化を実装する
   - bool/int/string/int64/log level/level type 用の既存 helper を使う
4. 書き戻し対象にしたいなら `SaveServerPropertiesConfig()` を更新する
   - ただし現状この関数は、意図的にワールド識別情報だけを永続化します
5. 実行時反映箇所を更新する:
   - `ApplyServerPropertiesToDedicatedConfig(...)`
   - `ServerMain.cpp` の `app.SetGameHostOption(...)`
   - CLI 上書きも持たせるなら `PrintUsage()` / `ParseCommandLine()`
6. 手動確認:
   - 欠損キーの自動補完
   - 不正値の正規化
   - clamp 範囲が妥当か
   - 実行時挙動に反映されるか

> 覚えておくと良い正規化ポイント
- `level-id` は安全な save ID に正規化され、長さ制限も掛かる
- `server-name` は実行時 16 文字まで
- `max-players` は `1..8` に clamp される(あとで増やす必要あり)
- `autosave-interval` は `5..3600` に clamp される
- `level-type` は `default` または `flat` に正規化される

### 5.3 BAN / アクセス制御挙動を変更する

主な実装は `Access/Access.cpp`, `Access/BanManager.cpp`, `ServerLogManager.cpp` にあります

変更時の注意:
- `BanManager` は storage/caching に責務を寄せ、 live-network policy を持ち込みすぎない
- `Access.cpp` の clone-and-publish スナップショット方式を保ち、読取側がディスク I/O で止まらないようにする
- `ban-ip <player>` は `ServerLogManager::TryGetConnectionRemoteIp(...)` に依存することを忘れない
- `SnapshotBannedPlayers()` / `SnapshotBannedIps()` には期限切れエントリを混ぜない
- 確認項目:
  - 欠損時に空の BAN ファイルが初回起動で生成される
  - `ban`, `ban-ip`, `pardon`, `pardon-ip`, `banlist` が動く
  - オンライン対象の BAN が即時切断まで到達する
  - 将来 reload 経路を増やしても手動編集が安全に再読込できる

### 5.4 ワールドロード/新規作成ロジックを変更する

主な実装は `WorldManager.cpp` にあります

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
  - 新規ワールド時の明示的初回保存が `ServerMain.cpp` から維持されているか

### 5.5 ログを追加する

`ServerLogger` の API を利用:
- `LogDebug`, `LogInfo`, `LogWarn`, `LogError`
- フォーマット付きは `LogDebugf`, `LogInfof` など

transport/login/disconnect ライフサイクルに属するイベントなら `ServerLogManager` 側を使います

推奨カテゴリ:
- `startup`: 起動ライフサイクル
- `shutdown`: 停止ライフサイクル
- `world-io`: ワールド/保存処理
- `console`: CLI コマンド処理
- `access`: BAN/アクセス制御状態
- `network`: 接続/ログイン監査

## 6. ビルドと実行

リポジトリルートで実行:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --target MinecraftServer
cd .\build\Debug
.\Minecraft.Server.exe -port 25565 -bind 0.0.0.0 -name DedicatedServer
```

補足:
- プロセスは起動時にカレントディレクトリを実行ファイルの場所へ切り替えます
- `server.properties`, `banned-players.json`, `banned-ips.json` はそのため実行ファイル横に読み書きされます
- Visual Studio ワークフローはルートの `COMPILE.md` を参照してください

## 7. 変更前チェックリスト

- `server.properties` が欠損または疎でもクラッシュせず起動できる
- 欠損したアクセス制御ファイルがクリーンブート時に再生成される
- 既存ワールドが期待した `level-id` でロードされる
- 新規ワールド作成時の明示的初回保存が維持される
- CLI 入力と補完が引き続き応答する
- `banlist` 出力が BAN 追加/解除後も破綻しない
- 非同期待機ループから `TickCoreSystems()` など busy-wait 防止用ティックを消していない
- 新規追加したソースが CMake と `.vcxproj` の両方に入っている

## 8. クイックトラブルシュート

- コマンドが認識されない:
  - `RegisterDefaultCommands()` とビルド定義を確認する
- `server.properties` や BAN ファイルの読込先が想定と違う:
  - `SetExeWorkingDirectory()` により実行ファイルのフォルダへ移動していることを確認する
- オートセーブ/終了時保存がタイムアウトする:
  - 待機ループ内で `TickCoreSystems()` と `HandleXuiActions()` を回しているか確認する
- 再起動時に同じワールドを使わない:
  - `level-id` の正規化と `WorldManager.cpp` の一致判定を確認する
- `ban-ip <player>` で IP を解決できない:
  - 対象プレイヤーがオンラインで、`ServerLogManager` に接続 IP がキャッシュされているか確認する
- 設定変更が効かない:
  - 値が `ServerPropertiesConfig` にロードされ、必要なら `DedicatedServerConfig` にコピーされ、その後 `ServerMain.cpp` で反映されているか確認する
