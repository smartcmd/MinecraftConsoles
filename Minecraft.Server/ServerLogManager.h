#pragma once

#include <string>
#include <stdarg.h>

#include "..\Minecraft.World\DisconnectPacket.h"

namespace ServerRuntime
{
    namespace ServerLogManager
    {
        /**
         * Identifies why the dedicated server rejected a TCP connection before login completed
         * ログイン完了前にTCP接続を拒否した理由
         */
        enum ETcpRejectReason
        {
            eTcpRejectReason_BannedIp = 0,
            eTcpRejectReason_GameNotReady,
            eTcpRejectReason_ServerFull
        };

        /**
         * Identifies why the dedicated server rejected a player during login validation
         * ログイン検証中にプレイヤーを拒否した理由
         */
        enum ELoginRejectReason
        {
            eLoginRejectReason_BannedXuid = 0,
            eLoginRejectReason_NotWhitelisted,
            eLoginRejectReason_DuplicateXuid,
            eLoginRejectReason_DuplicateName
        };

        /**
         * Returns `true` when client-side debug logs should be redirected into the dedicated server logger
         * dedicated server時にclient側デバッグログを転送すかどうか
         */
        bool ShouldForwardClientDebugLogs();

        /**
         * Formats and forwards `CMinecraftApp::DebugPrintf` output through the dedicated server logger
         * CMinecraftApp::DebugPrintf の出力を専用サーバーロガーへ転送
         */
        void ForwardClientAppDebugLogV(const char *format, va_list args);

        /**
         * Formats and forwards `CMinecraftApp::DebugPrintf(int user, ...)` output through the dedicated server logger
         * CMinecraftApp::DebugPrintf(int user, ...) の出力を専用サーバーロガーへ転送
         */
        void ForwardClientUserDebugLogV(int user, const char *format, va_list args);

        /**
         * Formats and forwards legacy `DebugSpew` output through the dedicated server logger
         * 従来の DebugSpew 出力を専用サーバーロガーへ転送
         */
        void ForwardClientDebugSpewLogV(const char *format, va_list args);

        /**
         * Clears cached connection metadata before the dedicated server starts accepting players
         * 接続ログ管理用のキャッシュを初期化
         */
        void Initialize();

        /**
         * Releases cached connection metadata after the dedicated server stops
         * 接続ログ管理用のキャッシュを停止時に破棄
         */
        void Shutdown();

        /**
         * **Log Incoming TCP Connection**
         *
         * Emits a named log for a raw TCP accept before smallId assignment finishes
         * smallId割り当て前のTCP接続を記録
         */
        void OnIncomingTcpConnection(const char *ip);

        /**
         * Emits a named log for a TCP connection rejected before login starts
         * ログイン開始前に拒否したTCP接続を記録
         */
        void OnRejectedTcpConnection(const char *ip, ETcpRejectReason reason);

        /**
         * Stores the remote IP for the assigned smallId and logs the accepted transport connection
         * 割り当て済みsmallIdに対接続IPを保存して記録
         */
        void OnAcceptedTcpConnection(unsigned char smallId, const char *ip);

        /**
         * Associates a player name with the connection and emits the accepted login log
         * 接続にプレイヤー名を関連付けてログイン成功を記録
         */
        void OnAcceptedPlayerLogin(unsigned char smallId, const std::wstring &playerName);

        /**
         * Emits a named login rejection log and clears cached metadata for that smallId
         * ログイン拒否を記録し対象smallIdのキャッシュを破棄
         */
        void OnRejectedPlayerLogin(unsigned char smallId, const std::wstring &playerName, ELoginRejectReason reason);

        /**
         * Emits a named disconnect log using cached connection metadata and then clears that entry
         * 接続キャッシュを使って切断ログを出しその後で破棄
         */
        void OnPlayerDisconnected(
            unsigned char smallId,
            const std::wstring &playerName,
            DisconnectPacket::eDisconnectReason reason,
            bool initiatedByServer);

        /**
         * Reads the cached remote IP for a live smallId without consuming the entry
         * Eventually, this should be implemented in a separate class or on the `Minecraft.Client` side instead.
         * 
         * 指定smallIdの接続IPをキャッシュから参照する
         */
        bool TryGetConnectionRemoteIp(unsigned char smallId, std::string *outIp);

        /**
         * Removes any remembered IP or player name for the specified smallId
         * 指定smallIdに紐づく接続キャッシュを消去
         */
        void ClearConnection(unsigned char smallId);
    }
}
