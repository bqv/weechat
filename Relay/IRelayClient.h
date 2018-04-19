#pragma once

class IRelayClient
{
public:
    IRelayClient() { }
    virtual ~IRelayClient() { }

    virtual void OnConnected(const char* version) = 0;
    virtual void OnProtocolError() = 0;
};
