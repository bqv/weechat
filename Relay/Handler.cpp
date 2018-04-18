#include <cstddef>

#include "Handler.h"
#include "Types.h"
#include "Packet.h"

static const char MSG_QUIT[] = "quit\n";
static const char MSG_INFOVER[] = "info version\n";

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

void Handler::handle(Packet pPacket)
{
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
    mSocket->Send(MSG_INFOVER, sizeof(MSG_INFOVER));
}

void Handler::onClose()
{
    mSocket->Send(MSG_QUIT, sizeof(MSG_QUIT));
}

void Handler::onReceive(const void* pBuffer, int pLength)
{
    std::string data((char*)pBuffer, pLength);
    mStream.write(data.c_str(), data.length());
    std::string len_str = mStream.str().substr(0, 4);
    if (len_str.size() == 4)
    {
        std::istringstream in(len_str);
        unsigned int len = int_t::read(in).data;

        if (mStream.str().size() >= len)
        {
            char* bytes = new char[len];

            mStream.read(bytes, len);
            std::string bytestring(bytes, len);
            Packet packet(bytestring);

            delete[] bytes;

            handle(packet);
        }
    }
}
