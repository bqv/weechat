#pragma once

class CRelayConnection;

// CRelaySocket command target

class CRelaySocket : public CAsyncSocket
{
public:
	CRelaySocket();
	virtual ~CRelaySocket();

private:
    CRelayConnection* m_pConn;

public:
    void SetConnection(CRelayConnection *conn);

// Overrides
public:
    virtual void OnAccept(int nErrorCode);
    virtual void OnClose(int nErrorCode);
    virtual void OnConnect(int nErrorCode);
    virtual void OnOutOfBandData(int nErrorCode);
    virtual void OnReceive(int nErrorCode);
    virtual void OnSend(int nErrorCode);
};


