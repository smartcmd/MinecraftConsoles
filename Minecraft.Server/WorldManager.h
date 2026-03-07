#pragma once

#include <string>
#include <windows.h>

#include "ServerProperties.h"

struct _LoadSaveDataThreadParam;
typedef struct _LoadSaveDataThreadParam LoadSaveDataThreadParam;

namespace ServerRuntime
{
	/** Tick callback used while waiting on async storage/network work */
	typedef void (*WorldManagerTickProc)();
	/** Optional action handler used while waiting for server actions */
	typedef void (*WorldManagerHandleActionsProc)();

	/**
	 * **World Bootstrap Status**
	 *
	 * Result type for world startup preparation, either loading an existing world or creating a new one
	 * ワールド起動準備の結果種別
	 */
	enum EWorldBootstrapStatus
	{
		/** Found and loaded an existing world */
		eWorldBootstrap_Loaded,
		/** No matching save was found, created a new world context */
		eWorldBootstrap_CreatedNew,
		/** Bootstrap failed and server startup should be aborted */
		eWorldBootstrap_Failed
	};

	/**
	 * **World Bootstrap Result**
	 *
	 * Output payload returned by world startup preparation
	 * ワールド起動準備の出力データ
	 */
	struct WorldBootstrapResult
	{
		/** Bootstrap status */
		EWorldBootstrapStatus status;
		/** Save data used for server initialization, `NULL` when creating a new world */
		LoadSaveDataThreadParam *saveData;
		/** Save ID that was actually selected */
		std::string resolvedSaveId;

		WorldBootstrapResult()
			: status(eWorldBootstrap_Failed)
			, saveData(NULL)
		{
		}
	};

	/**
	 * **Bootstrap Target World For Server Startup**
	 *
	 * Resolves whether the target world should be loaded from an existing save or created as new
	 * - Applies `level-name` and `level-id` from `server.properties`
	 * - Loads when a matching save exists
	 * - Creates a new world context only when no save matches
	 * サーバー起動時のロードか新規作成かを確定する
	 *
	 * @param config Normalized `server.properties` values
	 * @param actionPad padId used by async storage APIs
	 * @param tickProc Tick callback run while waiting for async completion
	 * @return Bootstrap result including whether save data was loaded
	 */
	WorldBootstrapResult BootstrapWorldForServer(
		const ServerPropertiesConfig &config,
		int actionPad,
		WorldManagerTickProc tickProc);

	/**
	 * **Wait Until Server Action Returns To Idle**
	 *
	 * Waits until server action state reaches `Idle`
	 * サーバーアクションの待機処理
	 *
	 * @param actionPad padId to monitor
	 * @param timeoutMs Timeout in milliseconds
	 * @param tickProc Tick callback run inside the wait loop
	 * @param handleActionsProc Optional action handler callback
	 * @return `true` when `Idle` is reached before timeout
	 */
	bool WaitForWorldActionIdle(
		int actionPad,
		DWORD timeoutMs,
		WorldManagerTickProc tickProc,
		WorldManagerHandleActionsProc handleActionsProc);
}
