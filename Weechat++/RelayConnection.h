
#pragma once

#include "RelaySocket.h"
#include "RelaySslSocket.h"

class CRelayConnection
{
public:
    CRelayConnection(CWnd* parent);
    virtual ~CRelayConnection();

    CWnd* m_pWnd;
    enum { PLAIN, SSL } m_type;
    union {
        CRelaySocket* m_plainSock;
        CRelaySslSocket* m_sslSock;
    };

    bool Plain(CString host, UINT port);
    bool Ssl(CString host, UINT port);

    void OnConnect(int nErrorCode);
    void OnClose(int nErrorCode);
    void OnReceive(int nErrorCode);

    int Receive(void* lpBuf, int nBufLen, int nFlags = 0);
    int Send(const void* lpBuf, int nBufLen, int nFlags = 0);
    void Close();
};

