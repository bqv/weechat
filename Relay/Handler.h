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
#define HPATH_LINES "lines"
#define HPATH_LINE "line"
#define HPATH_LINE_DATA "line_data"
#define VAR_LINES "lines"
#define VAR_FIRST_LINE "first_line"
#define VAR_DATA "data"
#define PTR_GUI_BUFFERS "gui_buffers"
#define BUFFER_POINTER "pointer"
#define BUFFER_NUMBER "number"
#define BUFFER_NAME "name"
#define BUFFER_FULL_NAME "full_name"
#define BUFFER_SHORT_NAME "short_name"
#define BUFFER_NICKLIST "nicklist"
#define BUFFER_TITLE "title"
#define BUFFER_LOCAL_VARIABLES "local_variables"
#define BUFFER_PREV_BUFFER "prev_buffer"
#define BUFFER_NEXT_BUFFER "next_buffer"
#define BUFFER_TYPE "type"
#define LINE_DATA_PREFIX_LENGTH "prefix_length"
#define LINE_DATA_PREFIX "prefix"
#define LINE_DATA_REFRESH_NEEDED "refresh_needed"
#define LINE_DATA_STR_TIME "str_time"
#define LINE_DATA_TAGS_COUNT "tags_count"
#define LINE_DATA_TAGS_ARRAY "tags_array"
#define LINE_DATA_Y "y"
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
#define SYNC_BUFFER "buffer"
#define SYNC_NICKLIST "nicklist"

#define EVENT_BUFFER_OPENED 0
#define EVENT_BUFFER_TYPE_CHANGED 1
#define EVENT_BUFFER_MOVED 2
#define EVENT_BUFFER_MERGED 3
#define EVENT_BUFFER_UNMERGED 4
#define EVENT_BUFFER_HIDDEN 5
#define EVENT_BUFFER_UNHIDDEN 6
#define EVENT_BUFFER_RENAMED 7
#define EVENT_BUFFER_TITLE_CHANGED 8
#define EVENT_BUFFER_LOCALVAR_ADDED 9
#define EVENT_BUFFER_LOCALVAR_CHANGED 10
#define EVENT_BUFFER_LOCALVAR_REMOVED 11
#define EVENT_BUFFER_CLOSING 12
#define EVENT_BUFFER_CLEARED 13
#define EVENT_BUFFER_LINE_ADDED 14
#define EVENT_NICKLIST 15
#define EVENT_NICKLIST_DIFF 16
#define EVENT_PONG 17
#define EVENT_UPGRADE 18
#define EVENT_UPGRADE_ENDED 19

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

    static const std::map<std::string, char> RESERVED;
private:
    std::unordered_map<unsigned long, std::function<void(Packet* p)>> mCallbacks;

    void handle(Packet pPacket);
    void registerCallback(unsigned long pIdx, std::function<void(Packet* p)> pCallback);
};
