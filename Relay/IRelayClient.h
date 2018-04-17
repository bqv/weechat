#pragma once

class IRelayClient
{
public:
    IRelayClient() { }
    virtual ~IRelayClient() { }

    virtual void OnConnected() = 0;
};
