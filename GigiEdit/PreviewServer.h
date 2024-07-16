///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define NOMINMAX
// clang-format off
#include <winsock2.h>
#include <ws2tcpip.h>

#include <queue>
#include <string>
#include <chrono>

#include "Schemas/types.h"
#include "Schemas/JSON.h"
// clang-format on

class CPreviewServer
{
public:
	CPreviewServer();
	~CPreviewServer();

	int Start(int port);
	void Shutdown();

	bool Tick();

	bool PopMessage(PreviewMsgClientToServer &message)
	{
		if (m_state != State::Connected)
			return false;
		return PopMessageInternal(message);
	}

	bool Send(const PreviewMsgServerToClient& msg);

	bool IsFullyConnected() const { return m_state == State::Connected; }

	std::string GetClientVersion() const { return m_clientVersion; }

private:
	enum State
	{
		NotConnected,
		WaitingForVersion,
		Connected
	};

	bool Send(const char* msg);

	bool PopMessageInternal(PreviewMsgClientToServer& message)
	{
		if (m_messagesRcvd.empty())
			return false;

		bool couldRead = ReadFromJSONBuffer(message, m_messagesRcvd.front().c_str());

		m_messagesRcvd.pop();

		return couldRead;
	}

private:
	State m_state = State::NotConnected;
	WSADATA m_wsaData;
	SOCKET m_listenSocket = INVALID_SOCKET;
	SOCKET m_clientSocket = INVALID_SOCKET;

	std::string m_messageRcvdPartial;
	std::queue<std::string> m_messagesRcvd;
	std::string m_clientVersion;

	std::chrono::high_resolution_clock::time_point m_lastPingSent;
};