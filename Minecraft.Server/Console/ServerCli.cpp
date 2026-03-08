#include "stdafx.h"

#include "ServerCli.h"

#include "ServerCliEngine.h"
#include "ServerCliInput.h"

namespace ServerRuntime
{
	ServerCli::ServerCli()
		: m_engine(new ServerCliEngine())
		, m_input(new ServerCliInput())
	{
	}

	ServerCli::~ServerCli()
	{
		Stop();
	}

	void ServerCli::Start()
	{
		if (m_input && m_engine)
		{
			m_input->Start(m_engine.get());
		}
	}

	void ServerCli::Stop()
	{
		if (m_input)
		{
			m_input->Stop();
		}
	}

	void ServerCli::Poll()
	{
		if (m_engine)
		{
			m_engine->Poll();
		}
	}
}
