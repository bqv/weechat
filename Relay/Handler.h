#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <unordered_map>

#include "IRelayClient.h"
#include "ISocket.h"

#define HDATA_BUFFER "buffer"
#define HDATA_LINE_DATA "line_data"
#define GUI_BUFFERS "gui_buffers"
#define BUFFER_POINTER "pointer"
#define BUFFER_NUMBER "number"
#define BUFFER_NAME "name"
#define BUFFER_FULLNAME "full_name"
#define BUFFER_SHORTNAME "short_name"
#define BUFFER_NICKLIST "nicklist"
#define BUFFER_TITLE "title"
#define BUFFER_LOCALVARIABLES "localvariables"
#define BUFFER_PREVBUFFER "prevbuffer"
#define BUFFER_NEXTBUFFER "nextbuffer"
#define BUFFER_TYPE "type"
#define LINE_DATA_BUFFER "buffer"
#define LINE_DATA_DATE "date"
#define LINE_DATA_DATE_PRINTED "date_printed"
#define LINE_DATA_DISPLAYED "displayed"
#define LINE_DATA_HIGHLIGHT "highlight"
#define LINE_DATA_TAGS_ARRAY "tags_array"
#define LINE_DATA_PREFIX "prefix"
#define LINE_DATA_MESSAGE "message"
#define SYNC_BUFFERS "buffers"
#define SYNC_UPGRADE "upgrade"

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

    void onConnect();
    void onClose();
    void onReceive(const void* pBuffer, int pLength);

    void hdatasync(std::string pHdata, std::string pPointer, std::vector<std::string> pVars, std::vector<std::string> pKeys, std::vector<std::string> pBuffers, std::vector<std::string> pOptions, std::function<void(Packet* p)> pCallback = [](Packet* p){});
    void hdata(std::string pHdata, std::string pPointer, std::vector<std::string> pVars, std::vector<std::string> pKeys, std::function<void(Packet* p)> pCallback = [](Packet* p){});
    void info(std::string pName, std::function<void(Packet* p)> pCallback = [](Packet* p){});
    void infolist(std::string pName, std::string pPointer, std::vector<std::string> pArgs, std::function<void(Packet* p)> pCallback = [](Packet* p){});
    void nicklist(std::string pBuffer, std::function<void(Packet* p)> pCallback = [](Packet* p){});
    void input(std::string pBuffer, std::string pData);
    void sync(std::vector<std::string> pBuffers, std::vector<std::string> pOptions);
    void desync(std::vector<std::string> pBuffers, std::vector<std::string> pOptions);
    void test();
    void ping(std::string args);
    void quit();

private:
    std::unordered_map<unsigned long, std::function<void(Packet* p)>> mCallbacks;

    void handle(Packet pPacket);
    void registerCallback(unsigned long pIdx, std::function<void(Packet* p)> pCallback);
};
