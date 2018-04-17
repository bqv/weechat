#pragma once

class ISocket
{
public:
    ISocket() { }
    virtual ~ISocket() { }

    virtual int Receive(void* pBuf, int pBufLen, int pFlags = 0) = 0;
    virtual int Send(const void* pBuf, int pBufLen, int pFlags = 0) = 0;
    virtual void Close() = 0;
};
