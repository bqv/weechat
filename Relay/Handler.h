#pragma once

#include <string>
#include <sstream>

#include "IRelayClient.h"
#include "ISocket.h"

struct Packet;

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

    std::stringstream mStream;

    void init(const char* pPassword, bool pCompression = false);
    void handle(Packet pPacket);

    void onConnect();
    void onClose();
    void onReceive(const void* pBuffer, int pLength);
};
