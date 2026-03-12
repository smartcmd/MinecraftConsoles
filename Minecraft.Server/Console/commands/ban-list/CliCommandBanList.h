#pragma once

#include "..\IServerCliCommand.h"

namespace ServerRuntime
{
	/**
	 * **Ban List Command**
	 *
	 * Lists dedicated-server player bans and IP bans in a single command output
	 * 専用サーバーのプレイヤーBANとIP BANをまとめて表示する
	 */
	class CliCommandBanList : public IServerCliCommand
	{
	public:
		virtual const char *Name() const;
		virtual const char *Usage() const;
		virtual const char *Description() const;
		virtual bool Execute(const ServerCliParsedLine &line, ServerCliEngine *engine);
		virtual void Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const;
	};
}

