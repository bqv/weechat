#pragma once

#include <string>

#include "IRelayClient.h"
#include "ISocket.h"

class Handler
{
public:
    Handler(ISocket* pSocket, IRelayClient* pClient);
    ~Handler();

    ISocket* mSocket;
    IRelayClient* mClient;

    unsigned long mIdx;
    bool mCompression;
    std::string mPassword;

    void init(const char* pPassword, bool pCompression = false);

    void onConnect();
    void onClose();
    void onReceive(const void* pBuffer, int pLength);
};
