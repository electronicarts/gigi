///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "PreviewServer.h"

#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

CPreviewServer::CPreviewServer()
{

}

CPreviewServer::~CPreviewServer()
{
    if (m_state != State::NotConnected)
        Shutdown();
}

int CPreviewServer::Start(int port)
{
    // Initialize winsock
	int result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (result != 0)
		return result;

    // get our address info
    addrinfo* resulta = nullptr;
    addrinfo* ptr = nullptr;
    addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    char portString[256];
    sprintf_s(portString, "%i", port);
    result = getaddrinfo(NULL, portString, &hints, &resulta);
    if (result != 0) {
        WSACleanup();
        return result;
    }

    // Create a SOCKET for the server to listen for client connections
    m_listenSocket = socket(resulta->ai_family, resulta->ai_socktype, resulta->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET) {
        result = WSAGetLastError();
        freeaddrinfo(resulta);
        WSACleanup();
        return result;
    }

    // make this socket non blocking
    unsigned long ul = 1;
    result = ioctlsocket(m_listenSocket, FIONBIO, (unsigned long*)&ul);
    if (result == SOCKET_ERROR) {
        result = WSAGetLastError();
        freeaddrinfo(resulta);
        closesocket(m_listenSocket);
        WSACleanup();
        return result;
    }

    // Setup the TCP listening socket
    result = bind(m_listenSocket, resulta->ai_addr, (int)resulta->ai_addrlen);
    if (result == SOCKET_ERROR) {
        result = WSAGetLastError();
        freeaddrinfo(resulta);
        closesocket(m_listenSocket);
        WSACleanup();
        return result;
    }
    freeaddrinfo(resulta);

    // listen
    if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        result = WSAGetLastError();
        closesocket(m_listenSocket);
        WSACleanup();
        return result;
    }

    // we are started and everything is ok!
    m_state = State::WaitingForVersion;
	return 0;
}

void CPreviewServer::Shutdown()
{
    if (m_state == State::NotConnected)
        return;

    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
    }

    if (m_clientSocket != INVALID_SOCKET)
    {
        closesocket(m_clientSocket);
        m_clientSocket = INVALID_SOCKET;
    }

    WSACleanup();
    m_state = State::NotConnected;

    m_messageRcvdPartial = "";
    std::queue<std::string> empty;
    std::swap(m_messagesRcvd, empty);
}

bool CPreviewServer::Tick()
{
    if (m_state == State::NotConnected)
        return true;

    // If we don't yet have a connection, try to get one
    if (m_clientSocket == INVALID_SOCKET)
    {
        m_clientSocket = accept(m_listenSocket, nullptr, nullptr);
        if (m_clientSocket == INVALID_SOCKET)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                Shutdown();
                return false;
            }
            return true;
        }
    }

    // Try to recieve data from the client socket
    char buffer[1025]; // an extra so we can put a null at the end to strcpy the last bit more easily.
    int bytesRead = recv(m_clientSocket, buffer, 1024, 0);
    if (bytesRead == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
        {
            Shutdown();
            return false;
        }
    }
    else
    {
        // process the data we received
        int readStart = 0;
        int readEnd = 0;
        while (readEnd < bytesRead)
        {
            if (buffer[readEnd] == 0)
            {
                m_messagesRcvd.push(m_messageRcvdPartial + &buffer[readStart]);
                m_messageRcvdPartial = "";

                readEnd += 1;
                readStart = readEnd;
            }
            else
            {
                readEnd++;
            }
        }

        if (readStart < bytesRead)
        {
            buffer[bytesRead] = 0;
            m_messageRcvdPartial += &buffer[readStart];
        }
    }

    // Handle handshake
    if (m_state == State::WaitingForVersion)
    {
        PreviewMsgClientToServer msg;
        if (PopMessageInternal(msg))
        {
            // if we got an unexpected message, exit.
            if (msg._index != PreviewMsgClientToServer::c_index_version)
            {
                Shutdown();
                return false;
            }

            m_clientVersion = msg.version.version;

            // Tell the client whether the version is ok or not
            bool versionOk = (_stricmp(msg.version.version.c_str(), GIGI_VERSION()) == 0);
            PreviewMsgServerToClient versionResponse;
            versionResponse._index = PreviewMsgServerToClient::c_index_versionResponse;
            versionResponse.versionResponse.versionOK = versionOk;
            
            // if we couldn't send the message, or the version was bad, exit
            if (!Send(versionResponse) || !versionOk)
            {
                Shutdown();
                return false;
            }

            m_state = State::Connected;
            m_lastPingSent = std::chrono::high_resolution_clock::now();
        }
    }
    else
    {
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        float secondsSinceLastPing = (float)std::chrono::duration_cast<std::chrono::duration<double>>(now - m_lastPingSent).count();
        if (secondsSinceLastPing > 1.0f)
        {
            m_lastPingSent = now;
            PreviewMsgServerToClient ping;
            ping._index = PreviewMsgServerToClient::c_index_ping;
            if (!Send(ping))
            {
                Shutdown();
                return false;
            }
        }
    }

    return true;
}

bool CPreviewServer::Send(const PreviewMsgServerToClient& msg)
{
    std::string msgString;
    WriteToJSONBuffer(msg, msgString, true);
    return Send(msgString.c_str());
}

bool CPreviewServer::Send(const char* msg)
{
    if (m_state == State::NotConnected || m_clientSocket == INVALID_SOCKET)
        return true;

    int result = send(m_clientSocket, msg, (int)strlen(msg) + 1, 0);
    if (result == SOCKET_ERROR)
    {
        return false;
        Shutdown();
    }

    return true;
}
