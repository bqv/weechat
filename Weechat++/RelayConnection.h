
#pragma once

#include "RelaySocket.h"
#include "RelaySslSocket.h"
#include "Relay/Handler.h"
#include "Relay/ISocket.h"

class CRelayConnection : public ISocket
{
public:
    CRelayConnection(Handler* handler, CWnd* parent);
    virtual ~CRelayConnection();

    Handler* m_pHandler;
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
