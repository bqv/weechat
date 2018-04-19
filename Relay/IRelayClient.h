#pragma once

#include "Packet.h"

class IRelayClient
{
public:
    IRelayClient() { }
    virtual ~IRelayClient() { }

    virtual void OnConnected(const char* version) = 0;
    virtual void OnProtocolError() = 0;
    virtual void OnMessage(Packet& p) = 0;
};
