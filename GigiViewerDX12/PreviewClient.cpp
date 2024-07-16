///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "PreviewClient.h"

#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

CPreviewClient::CPreviewClient()
{

}

CPreviewClient::~CPreviewClient()
{
    if (m_state != State::NotConnected)
        Shutdown();
}

int CPreviewClient::Start(const std::string& serverIP, const std::string& serverPort)
{
    // Initialize winsock
    int result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (result != 0)
        return result;

    // get our address info
    addrinfo* resulta = nullptr;
    addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    result = getaddrinfo(serverIP.c_str(), serverPort.c_str(), &hints, &resulta);
    if (result != 0) {
        WSACleanup();
        return result;
    }

    // Attempt to connect to an address until one succeeds
    for (addrinfo* ptr = resulta; ptr != nullptr; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        m_serverSocket = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);
        if (m_serverSocket == INVALID_SOCKET) {
            result = WSAGetLastError();
            WSACleanup();
            return result;
        }

        // Connect to server.
        result = connect(m_serverSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR) {
            closesocket(m_serverSocket);
            m_serverSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(resulta);

    if (m_serverSocket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    // make this socket non blocking
    unsigned long ul = 1;
    result = ioctlsocket(m_serverSocket, FIONBIO, (unsigned long*)&ul);
    if (result == SOCKET_ERROR) {
        result = WSAGetLastError();
        closesocket(m_serverSocket);
        WSACleanup();
        return result;
    }

    // send the Gigi version number
    m_state = State::SentVersion;
    PreviewMsgClientToServer msg;
    msg._index = PreviewMsgClientToServer::c_index_version;
    msg.version.version = GIGI_VERSION();
    if (!Send(msg))
    {
        closesocket(m_serverSocket);
        WSACleanup();
        return 1;
    }

    // we are started and everything is ok!
    return 0;
}

void CPreviewClient::Shutdown()
{
    if (m_state == State::NotConnected)
        return;

    if (m_serverSocket != INVALID_SOCKET)
    {
        closesocket(m_serverSocket);
        m_serverSocket = INVALID_SOCKET;
    }

    WSACleanup();
    m_state = State::NotConnected;

    m_messageRcvdPartial = "";
    std::queue<std::string> empty;
    std::swap(m_messagesRcvd, empty);
}

bool CPreviewClient::Tick()
{
    if (m_state == State::NotConnected)
        return true;

    // Try to recieve data from the server socket
    char buffer[1025]; // an extra so we can put a null at the end to strcpy the last bit more easily.
    int bytesRead = recv(m_serverSocket, buffer, 1024, 0);
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
    if (m_state == State::SentVersion)
    {
        PreviewMsgServerToClient msg;
        if (PopMessageInternal(msg))
        {
            // if we got an unexpected message, or the version was not ok, exit
            if (msg._index != PreviewMsgServerToClient::c_index_versionResponse || !msg.versionResponse.versionOK)
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
            PreviewMsgClientToServer ping;
            ping._index = PreviewMsgClientToServer::c_index_ping;
            if (!Send(ping))
            {
                Shutdown();
                return false;
            }
        }
    }

    return true;
}

bool CPreviewClient::Send(const PreviewMsgClientToServer& msg)
{
    std::string msgString;
    WriteToJSONBuffer(msg, msgString, true);
    return Send(msgString.c_str());
}

bool CPreviewClient::Send(const char* msg)
{
    if (m_state == State::NotConnected || m_serverSocket == INVALID_SOCKET)
        return true;

    int result = send(m_serverSocket, msg, (int)strlen(msg) + 1, 0);
    if (result == SOCKET_ERROR)
    {
        return false;
        Shutdown();
    }

    return true;
}
