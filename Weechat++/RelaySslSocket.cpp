// RelaySslSocket.cpp : implementation file
//

#include "stdafx.h"
#include "RelaySslSocket.h"
#include "RelayConnection.h"

// CRelaySslSocket

CRelaySslSocket::CRelaySslSocket()
    : CSslSocket(TRUE)
{
    m_pConn = nullptr;
}

CRelaySslSocket::~CRelaySslSocket()
{
}

void CRelaySslSocket::SetConnection(CRelayConnection* conn)
{
    m_pConn = conn;
}

void CRelaySslSocket::OnAccept(int nErrorCode)
{
    if (nErrorCode != 0)
    {
        LPTSTR messageBuffer = nullptr;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&messageBuffer, 0, NULL);
        CString message(messageBuffer, size);
        LocalFree(messageBuffer);
        CString text;
        text.Format(_T("%d: "), nErrorCode);
        text += message;
        AfxMessageBox(text);
    }

    CSslSocket::OnAccept(nErrorCode);
}

void CRelaySslSocket::OnClose(int nErrorCode)
{
    CSslSocket::OnClose(nErrorCode);

    m_pConn->OnClose(nErrorCode);
}

void CRelaySslSocket::OnConnect(int nErrorCode)
{
    CSslSocket::OnConnect(nErrorCode);

    m_pConn->OnConnect(nErrorCode);
}

void CRelaySslSocket::OnOutOfBandData(int nErrorCode)
{
    if (nErrorCode != 0)
    {
        LPTSTR messageBuffer = nullptr;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&messageBuffer, 0, NULL);
        CString message(messageBuffer, size);
        LocalFree(messageBuffer);
        CString text;
        text.Format(_T("%d: "), nErrorCode);
        text += message;
        AfxMessageBox(text);
    }

    CSslSocket::OnOutOfBandData(nErrorCode);
}

void CRelaySslSocket::OnReceive(int nErrorCode)
{
    CSslSocket::OnReceive(nErrorCode);

    m_pConn->OnReceive(nErrorCode);
}

void CRelaySslSocket::OnSend(int nErrorCode)
{
    if (nErrorCode != 0)
    {
        LPTSTR messageBuffer = nullptr;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&messageBuffer, 0, NULL);
        CString message(messageBuffer, size);
        LocalFree(messageBuffer);
        CString text;
        text.Format(_T("%d: "), nErrorCode);
        text += message;
        AfxMessageBox(text);
    }

    CSslSocket::OnSend(nErrorCode);
}


// CRelaySslSocket member functions
