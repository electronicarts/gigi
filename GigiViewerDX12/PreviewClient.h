///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>

#include <queue>
#include <string>
#include <chrono>
#include <stdarg.h>

#include "Schemas/types.h"
#include "Schemas/JSON.h"
// clang-format on

class CPreviewClient
{
public:
	CPreviewClient();
	~CPreviewClient();

	int Start(const std::string& serverIP, const std::string& serverPort);
	void Shutdown();

	bool Tick();

	bool PopMessage(PreviewMsgServerToClient& message)
	{
		if (m_state != State::Connected)
			return false;
		return PopMessageInternal(message);
	}

	bool Send(const PreviewMsgClientToServer& msg);

	bool IsFullyConnected() const { return m_state == State::Connected; }

private:
	enum State
	{
		NotConnected,
		SentVersion,
		Connected
	};

	bool Send(const char* msg);

	bool PopMessageInternal(PreviewMsgServerToClient& message)
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
	SOCKET m_serverSocket = INVALID_SOCKET;

	std::string m_messageRcvdPartial;
	std::queue<std::string> m_messagesRcvd;

	std::chrono::high_resolution_clock::time_point m_lastPingSent;
};