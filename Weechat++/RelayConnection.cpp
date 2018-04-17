
#include "stdafx.h"
#include "RelayConnection.h"
#include "MainFrm.h"

CRelayConnection::CRelayConnection(CWnd* parent)
    : m_pWnd(parent)
{
    m_plainSock = NULL;
    m_sslSock = NULL;
}


CRelayConnection::~CRelayConnection()
{
    switch (m_type)
    {
    case PLAIN:
        m_plainSock->Close();
        delete m_plainSock;
        break;
    case SSL:
        m_sslSock->Close();
        delete m_sslSock;
        break;
    default:
        ASSERT(0);
    }
}

bool CRelayConnection::Plain(CString host, UINT port)
{
    m_type = PLAIN;
    if (m_plainSock)
    {
        delete m_plainSock;
    }
    m_plainSock = new CRelaySocket();
    m_plainSock->SetConnection(this);
    if (m_plainSock->Create() == FALSE)
    {
        return FALSE;
    }
    if (m_plainSock->Connect(host, port) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

bool CRelayConnection::Ssl(CString host, UINT port)
{
    m_type = SSL;
    if (m_sslSock)
    {
        delete m_sslSock;
    }
    m_sslSock = new CRelaySslSocket();
    m_sslSock->SetConnection(this);
    if (m_sslSock->Create() == FALSE)
    {
        return FALSE;
    }
    if (m_sslSock->Connect(host, port) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

void CRelayConnection::OnConnect(int nErrorCode)
{
#ifdef DEBUG
    m_pWnd->SendMessage(WM_SOCK_CONNECT, (LPARAM)&nErrorCode);
#else
    m_pWnd->PostMessage(WM_SOCK_CONNECT, (LPARAM)&nErrorCode);
#endif
}

void CRelayConnection::OnClose(int nErrorCode)
{
#ifdef DEBUG
    m_pWnd->SendMessage(WM_SOCK_CLOSE, (LPARAM)&nErrorCode);
#else
    m_pWnd->PostMessage(WM_SOCK_CLOSE, (LPARAM)&nErrorCode);
#endif
}

void CRelayConnection::OnReceive(int nErrorCode)
{
#ifdef DEBUG
    m_pWnd->SendMessage(WM_SOCK_RECEIVE, (LPARAM)&nErrorCode);
#else
    m_pWnd->PostMessage(WM_SOCK_RECEIVE, (LPARAM)&nErrorCode);
#endif
}

int CRelayConnection::Receive(void* lpBuf, int nBufLen, int nFlags)
{
    switch (m_type)
    {
    case PLAIN:
        return m_plainSock->Receive(lpBuf, nBufLen, nFlags);
    case SSL:
        return m_sslSock->Receive(lpBuf, nBufLen, nFlags);
    default:
        ASSERT(0);
        return -1;
    }
}

int CRelayConnection::Send(const void* lpBuf, int nBufLen, int nFlags)
{
    switch (m_type)
    {
    case PLAIN:
        return m_plainSock->Send(lpBuf, nBufLen, nFlags);
    case SSL:
        return m_sslSock->Send(lpBuf, nBufLen, nFlags);
    default:
        ASSERT(0);
        return -1;
    }
}

void CRelayConnection::Close()
{
    switch (m_type)
    {
    case PLAIN:
        return m_plainSock->Close();
    case SSL:
        return m_sslSock->Close();
    default:
        ASSERT(0);
    }
}
