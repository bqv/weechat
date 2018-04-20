#include <cstddef>

#include "Types.h"
#include "Handler.h"
#include "Packet.h"
#include "Util.h"

using event_buffer_opened = hda_t;
using event_buffer_moved = hda_t;
using event_buffer_merged = hda_t;
using event_buffer_unmerged = hda_t;
using event_buffer_hidden = hda_t;
using event_buffer_unhidden = hda_t;
using event_buffer_renamed = hda_t;
using event_buffer_title_changed = hda_t;
using event_buffer_cleared = hda_t;
using event_buffer_type_changed = hda_t;
using event_buffer_localvar_added = hda_t;
using event_buffer_localvar_changed = hda_t;
using event_buffer_localvar_removed = hda_t;
using event_buffer_line_added = hda_t;
using event_buffer_closing = hda_t;
using event_nicklist = hda_t;
using event_nicklist_diff = hda_t;
using event_pong = str_t;

const std::map<std::string, char> Handler::RESERVED = {
    {"_buffer_opened", EVENT_BUFFER_OPENED},
    {"_buffer_type_changed", EVENT_BUFFER_TYPE_CHANGED},
    {"_buffer_moved", EVENT_BUFFER_MOVED},
    {"_buffer_merged", EVENT_BUFFER_MERGED},
    {"_buffer_unmerged", EVENT_BUFFER_UNMERGED},
    {"_buffer_hidden", EVENT_BUFFER_HIDDEN},
    {"_buffer_unhidden", EVENT_BUFFER_UNHIDDEN},
    {"_buffer_renamed", EVENT_BUFFER_RENAMED},
    {"_buffer_title_changed", EVENT_BUFFER_TITLE_CHANGED},
    {"_buffer_localvar_added", EVENT_BUFFER_LOCALVAR_ADDED},
    {"_buffer_localvar_changed", EVENT_BUFFER_LOCALVAR_CHANGED},
    {"_buffer_localvar_removed", EVENT_BUFFER_LOCALVAR_REMOVED},
    {"_buffer_closing", EVENT_BUFFER_CLOSING},
    {"_buffer_cleared", EVENT_BUFFER_CLEARED},
    {"_buffer_line_added", EVENT_BUFFER_LINE_ADDED},
    {"_nicklist", EVENT_NICKLIST},
    {"_nicklist_diff", EVENT_NICKLIST_DIFF},
    {"_pong", EVENT_PONG},
    {"_upgrade", EVENT_UPGRADE},
    {"_upgrade_ended", EVENT_UPGRADE_ENDED}
};

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
    if (pPacket.id && pPacket.id.value().compare(0,1,"_") != 0)
    {
        unsigned long id;
        try
        {
            id = std::stoul(pPacket.id.value());
        }
        catch (std::invalid_argument&)
        {
            mClient->OnMessage(pPacket);
            return;
        }
        try
        {
            std::function<void(Packet* p)> callback = mCallbacks.at(id);
            callback(&pPacket);
        }
        catch (std::out_of_range&)
        {
            mClient->OnMessage(pPacket);
        }
    }
    else
    {
        mClient->OnMessage(pPacket);
    }
}

void Handler::registerCallback(unsigned long pIdx, std::function<void(Packet* p)> pCallback)
{
    mCallbacks.insert({ pIdx, pCallback });
}

void Handler::onConnect()
{
    std::ostringstream ss_msg;
    ss_msg << '(' << mIdx++ << ") init ";
    ss_msg << "password=" << mPassword << ",";
    ss_msg << "compression=" << (mCompression ? "on" : "off") << "\n";
    std::string msg_init = ss_msg.str();
    mSocket->Send(msg_init.c_str(), msg_init.size());
    info("version", [this](Packet* p) {
        inf_t info = std::get<inf_t>(p->objects[0]);
        mClient->OnConnected(info.value.c_str());
    });
}

void Handler::onClose()
{
    std::string msg_quit("quit\n");
    mSocket->Send(msg_quit.c_str(), msg_quit.size());
}

void Handler::onReceive(const void* pBuffer, int pLength)
{
    std::string data((char*)pBuffer, pLength);
    mStream.write(data.c_str(), data.length());
    const std::stringstream::pos_type start = mStream.tellg();
    char bytes[4];
    mStream.read(bytes, 4);
    mStream.seekg(start);
    uint32_t length = static_cast<uint32_t>(static_cast<unsigned char>(bytes[0]) << 24 |
        static_cast<unsigned char>(bytes[1]) << 16 |
        static_cast<unsigned char>(bytes[2]) << 8 |
        static_cast<unsigned char>(bytes[3]));
    uint32_t bufLength = (uint32_t)(mStream.tellp() - mStream.tellg());
    if (length <= bufLength && mStream.gcount() == 4)
    {
        char* bytes = new char[length];
        mStream.read(bytes, length);

        if (mStream.gcount() == length)
        {
            std::string bytestring(bytes, length);
            try
            {
                Packet packet(bytestring);
                handle(packet);
            }
            catch (std::invalid_argument&)
            {
                mClient->OnProtocolError();
                throw;
            }
        }
        else
        {
            mStream.seekg(start);
        }
        
        delete[] bytes;
    }
}

void Handler::hdatasync(std::string pHdata, std::string pPointer, std::vector<std::string> pVars, std::vector<std::string> pKeys, std::vector<std::string> pBuffers, std::vector<std::string> pOptions, std::function<void(Packet*p)> pCallback)
{
    registerCallback(mIdx, pCallback);
    std::ostringstream ss_msg;
    ss_msg << '(' << mIdx++ << ") hdata " << pHdata << ':' << pPointer;
    for (const std::string& var : pVars)
    {
        ss_msg << '/' << var;
    }
    if (!pKeys.empty())
    {
        ss_msg << ' ' << Util::join(pKeys, ",");
    }
    ss_msg << "\nsync";
    if (!pBuffers.empty())
    {
        ss_msg << ' ' << Util::join(pBuffers, ",");
    }
    else if (!pOptions.empty())
    {
        ss_msg << " *";
    }
    if (!pOptions.empty())
    {
        ss_msg << ' ' << Util::join(pOptions, ",");
    }
    ss_msg << "\n";
    std::string msg_hdatasync = ss_msg.str();
    mSocket->Send(msg_hdatasync.c_str(), msg_hdatasync.size());
}

void Handler::hdata(std::string pHdata, std::string pPointer, std::vector<std::string> pVars, std::vector<std::string> pKeys, std::function<void(Packet* p)> pCallback)
{
    registerCallback(mIdx, pCallback);
    std::ostringstream ss_msg;
    ss_msg << '(' << mIdx++ << ") hdata " << pHdata << ':' << pPointer;
    for (const std::string& var : pVars)
    {
        ss_msg << '/' << var;
    }
    if (!pKeys.empty())
    {
        ss_msg << ' ' << Util::join(pKeys, ",");
    }
    ss_msg << "\n";
    std::string msg_hdata = ss_msg.str();
    mSocket->Send(msg_hdata.c_str(), msg_hdata.size());
}

void Handler::info(std::string pName, std::function<void(Packet* p)> pCallback)
{
    registerCallback(mIdx, pCallback);
    std::ostringstream ss_msg;
    ss_msg << '(' << mIdx++ << ") info " << pName << "\n";
    std::string msg_info = ss_msg.str();
    mSocket->Send(msg_info.c_str(), msg_info.size());
}

void Handler::infolist(std::string pName, std::string pPointer, std::vector<std::string> pArgs, std::function<void(Packet* p)> pCallback)
{
    registerCallback(mIdx, pCallback);
    std::ostringstream ss_msg;
    ss_msg << '(' << mIdx++ << ") infolist " << pName;
    if (!pPointer.empty())
    {
        ss_msg << ' ' << pPointer;
        
        if (!pArgs.empty())
        {
            ss_msg << ' ' << Util::join(pArgs, ",");
        }
    }
    ss_msg << "\n";
    std::string msg_infolist = ss_msg.str();
    mSocket->Send(msg_infolist.c_str(), msg_infolist.size());
}

void Handler::nicklist(std::string pBuffer, std::function<void(Packet* p)> pCallback)
{
    registerCallback(mIdx, pCallback);
    std::ostringstream ss_msg;
    ss_msg << '(' << mIdx++ << ") nicklist";
    if (!pBuffer.empty())
    {
        ss_msg << ' ' << pBuffer;
    }
    ss_msg << "\n";
    std::string msg_nicklist = ss_msg.str();
    mSocket->Send(msg_nicklist.c_str(), msg_nicklist.size());
}

void Handler::input(std::string pBuffer, std::string pData)
{
    std::ostringstream ss_msg;
    ss_msg << "input " << pBuffer << ' ' << pData << "\n";
    std::string msg_input = ss_msg.str();
    mSocket->Send(msg_input.c_str(), msg_input.size());
}

void Handler::sync(std::vector<std::string> pBuffers, std::vector<std::string> pOptions)
{
    std::ostringstream ss_msg;
    ss_msg << "sync";
    if (!pBuffers.empty())
    {
        ss_msg << ' ' << Util::join(pBuffers, ",");
    }
    else if (!pOptions.empty())
    {
        ss_msg << " *";
    }
    if (!pOptions.empty())
    {
        ss_msg << ' ' << Util::join(pOptions, ",");
    }
    ss_msg << "\n";
    std::string msg_sync = ss_msg.str();
    mSocket->Send(msg_sync.c_str(), msg_sync.size());
}

void Handler::desync(std::vector<std::string> pBuffers, std::vector<std::string> pOptions)
{
    std::ostringstream ss_msg;
    ss_msg << "desync";
    if (!pBuffers.empty())
    {
        ss_msg << ' ' << Util::join(pBuffers, ",");
    }
    else if (!pOptions.empty())
    {
        ss_msg << " *";
    }
    if (!pOptions.empty())
    {
        ss_msg << ' ' << Util::join(pOptions, ",");
    }
    ss_msg << "\n";
    std::string msg_desync = ss_msg.str();
    mSocket->Send(msg_desync.c_str(), msg_desync.size());
}

void Handler::test()
{
    std::string msg_test("test\n");
    mSocket->Send(msg_test.c_str(), msg_test.size());
}

void Handler::ping(std::string args)
{
    std::ostringstream ss_msg;
    ss_msg << "ping " << args << "\n";
    std::string msg_ping = ss_msg.str();
    mSocket->Send(msg_ping.c_str(), msg_ping.size());
}

void Handler::quit()
{
    std::string msg_quit("quit\n");
    mSocket->Send(msg_quit.c_str(), msg_quit.size());
    mSocket->Close();
}
