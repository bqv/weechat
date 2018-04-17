#include <cstddef>
#include <sstream>

#include "Handler.h"

static const char MSG_QUIT[] = "quit\n";

Handler::Handler(ISocket* pSocket, IRelayClient* pClient)
    : mSocket(pSocket), mClient(pClient)
{
}

Handler::~Handler()
{
}

void Handler::init(const char* pPassword, bool pCompression)
{
    mIdx = 0;
    mPassword = pPassword;
    mCompression = pCompression;

    for (size_t i = 0; i < mPassword.size(); ++i) {
        if (mPassword[i] == ',') {
            mPassword.replace(i, 1, "\\,");
        }
    }
}

void Handler::onConnect()
{
    std::ostringstream ss_msg;
    ss_msg << "(" << mIdx << ") init ";
    ss_msg << "password=" << mPassword << ",";
    ss_msg << "compression=" << (mCompression ? "on" : "off") << "\n";
    std::string msg_init = ss_msg.str();
    mSocket->Send(msg_init.c_str(), msg_init.size(), NULL);
    mClient->OnConnected();
}

void Handler::onClose()
{
    mSocket->Send(MSG_QUIT, sizeof(MSG_QUIT));
}

void Handler::onReceive(const void* pBuffer, int pLength)
{
    std::string data((char*)pBuffer, pLength);
}
