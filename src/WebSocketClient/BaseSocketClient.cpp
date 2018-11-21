//------------------------------------------------------------------------------
//  BaseSocketClient.cc
//------------------------------------------------------------------------------

#if ORYOL_POSIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#define SOCKET_ERROR (-1)
#endif

#include <ostream>
#include <iostream>
#include "BaseSocketClient.h"

using namespace Oryol;

//------------------------------------------------------------------------------
static bool wouldBlockOrConnected() {

    return ((errno == EINPROGRESS) || (errno == EWOULDBLOCK) || (errno == EISCONN));

}


//------------------------------------------------------------------------------
void
BaseSocketClient::Setup(const BaseSocketClientSetup& setup) {
    o_assert(setup.MessageFunc);

    this->setup = setup;
    this->state = Disconnected;
}

//------------------------------------------------------------------------------
void
BaseSocketClient::Discard() {
    if (!this->IsDisconnected()) {
        this->Disconnect(0);
    }
}

//------------------------------------------------------------------------------
void
BaseSocketClient::Connect(const URL& serverUrl) {
    this->setup.ServerUrl = serverUrl;
    if (this->IsConnected()) {
        this->Disconnect(10);
    }
}

//------------------------------------------------------------------------------
void
BaseSocketClient::Disconnect(int waitFramesUntilReconnect) {
    this->destroySocket();
    this->state = Disconnected;
    // wait a few seconds before attempting reconnect
    this->waitFrames = waitFramesUntilReconnect;
}

//------------------------------------------------------------------------------
void
BaseSocketClient::Update() {
    // if Disconnect() was called, wait a bit before reconnect
    // attempt to not overwhelm the server
    if (this->waitFrames > 0) {
        o_assert(Disconnected == this->state);
        this->waitFrames--;
        return;
    }

    // normal connecting/connected loop
    switch (this->state) {
        case Disconnected:
            this->doConnect();
            break;
        case Connected:
            this->onConnected();
            break;
    }
}

//------------------------------------------------------------------------------
bool
BaseSocketClient::Send(const String& msg) {
    if ((this->sendBuffer.Size() + msg.Length()) < MaxSendBufferSize) {
        this->sendBuffer.Add((const uint8_t*)msg.AsCStr(), msg.Length());
        this->sendBuffer.Add((const uint8_t*)"\r", 1);
        return true;
    }
    else {
        // send buffer is full, drop the message
        return false;
    }
}

//------------------------------------------------------------------------------
void
BaseSocketClient::createSocket() {

    // create socket
    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    o_assert(SOCKET_ERROR != this->sock);

    // switch to non-blocking
    fcntl(this->sock, F_SETFL, O_NONBLOCK);
}

//------------------------------------------------------------------------------
void
BaseSocketClient::destroySocket() {
#if ORYOL_EMSCRIPTEN
    close(this->sock);
#elif ORYOL_POSIX
    shutdown(this->sock, SHUT_RDWR);
        close(this->sock);
#else
    shutdown(this->sock, SD_BOTH);
    closesocket(this->sock);
#endif
    this->sock = 0;
}

//------------------------------------------------------------------------------
void
BaseSocketClient::doConnect() {
    o_assert(0 == this->sock);
    o_assert(this->state == Disconnected);
    o_assert(this->setup.ServerUrl.HasHost());
    o_assert(this->setup.ServerUrl.HasPort());

    this->createSocket();

    String hostName = this->setup.ServerUrl.Host();
    const uint16_t port = atoi(this->setup.ServerUrl.Port().AsCStr());

    sockaddr_in addr;
    Memory::Clear(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    uint32_t ipAddr = 0;
    if (hostName.AsCStr()[0] >= '0' && hostName.AsCStr()[0] <= '9') {
        // an ip address
        ipAddr = inet_addr(hostName.AsCStr());
    }
    else {
        // a host name
        struct hostent* he = gethostbyname(hostName.AsCStr());
        if (he) {
            ipAddr = *(u_long *)he->h_addr_list[0];
        }
        else {
            o_error("Couldn't resolve host name '%s'\n", hostName.AsCStr());
        }
    }
    addr.sin_addr.s_addr = ipAddr;

    Log::Info("Connecting to '%s' => '%d.%d.%d.%d', port %d\n",
              hostName.AsCStr(),
              ipAddr & 0xFF,
              (ipAddr >> 8) & 0xFF,
              (ipAddr >> 16) & 0xFF,
              (ipAddr >> 24) & 0xFF,
              port);

    const int connectRes = connect(this->sock, (sockaddr*) &addr, sizeof(addr));
    if (SOCKET_ERROR == connectRes) {
        if (wouldBlockOrConnected()) {
            this->state = Connected;
            return;
        }
    }
    Log::Warn("Failed to initialize to '%s:%d'\n", hostName.AsCStr(), port);
    this->Disconnect(300);
}

//------------------------------------------------------------------------------
void
BaseSocketClient::onConnected() {
    o_assert(this->sock);

    // send and receivce data
    fd_set fdr, fdw;
    FD_ZERO(&fdr);
    FD_ZERO(&fdw);
    FD_SET(this->sock, &fdr);
    FD_SET(this->sock, &fdw);

    if (!this->opened) {
        this->setup.OpenFunc();
        this->opened = true;
    }

    struct timeval tv = { };
    const int selectRes = select(int(this->sock+1), &fdr, &fdw, 0, &tv);
    if (selectRes == SOCKET_ERROR) {
        o_error("select() failed.\n");
    }
    else if (selectRes > 0) {
        if (FD_ISSET(this->sock, &fdr)) {
            // recv the next chunk of data into the recv buffer
            if (!this->recvNextChunk()) {
                this->Disconnect(300);
                return;
            }
        }
        if (FD_ISSET(this->sock, &fdw)) {
            // send the next chunk of data from the send buffer
            if (!this->sendNextChunk()) {
                this->Disconnect(300);
                return;
            }
        }
    }

    // scan for complete received messages (separated by newline)
    this->scanMessages();
}

//------------------------------------------------------------------------------
bool
BaseSocketClient::sendNextChunk() {
    o_assert(this->sock);

    const int maxSendSize = 4096;
    int sendSize = this->sendBuffer.Size();
    if (sendSize > maxSendSize) {
        sendSize = maxSendSize;
    }
    if (sendSize > 0) {
        const uint8_t* sendData = this->sendBuffer.Data();
        const ssize_t sendRes = send(this->sock, (const char*)sendData, sendSize, 0);
        if (SOCKET_ERROR == sendRes) {
            // send error
            Log::Warn("Failed to send '%d' bytes, disconnecting!\n", sendSize);
            return false;
        }
        else {
            // res is number of bytes sent
            const int sentBytes = sendRes;
            this->sendBuffer.Remove(0, sentBytes);
        }
    }
    return true;
}

//------------------------------------------------------------------------------
bool
BaseSocketClient::recvNextChunk() {
    o_assert(this->sock);

    const int maxRecvSize = 4096;
    uint8_t recvBuf[maxRecvSize] = { };
    bool done = false;
    while (!done) {
        ssize_t recvRes = recv(this->sock, (char*) recvBuf, sizeof(recvBuf), 0);
        if (0 == recvRes) {
            std::cout << "Error in recv() (returned 0), disconnecting!" << std::endl;
            return false;
        }
        else if (recvRes > 0) {
            const int bytesReceived = recvRes;
            if (bytesReceived > 0) {
                this->recvBuffer.Add(recvBuf, bytesReceived);
                this->lastRecvTime = Clock::Now();
            }
        }
        else {
            if (wouldBlockOrConnected()) {
                done = true;
            }
            else {
                std::cout << "Error in recv() (returned error), disconnecting!" << std::endl;
                return false;
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
void
BaseSocketClient::scanMessages() {
    if (this->recvBuffer.Empty()) {
        return;
    }
    uint8_t* recvData = this->recvBuffer.Data();
    int recvSize = this->recvBuffer.Size();
    for (int scanPos = 0; scanPos < recvSize; scanPos++) {
        if (getEscapeCharacter() == recvData[scanPos]) {
            // found the end of a command, extract into string,
            // remove from recvBuffer and call handler
            String msg((const char*)recvData, 0, scanPos);
            this->recvBuffer.Remove(0, scanPos + 1);

            // reset variables for next command
            recvData = this->recvBuffer.Data();
            recvSize = this->recvBuffer.Size();
            scanPos = 0;

            // and call the message callback
            this->setup.MessageFunc(msg);
        }
    }
}

void BaseSocketClient::Close() {
    //not implemented!
}

int BaseSocketClient::GetReadyState() {
    switch (this->state){
        case Disconnected:
            return 3; // Equal to CLOSED state in https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/readyState
        case Connected:
            return 1; // Equal to OPEN state in https://developer.mozilla.org/en-US/docs/Web/API/WebSocket/readyState
    }
}

uint8_t BaseSocketClient::getEscapeCharacter() {
    return this->escapeCharacter;
}

void BaseSocketClient::setEscapeCharacter(uint8_t c) {
    this->escapeCharacter = c;
}
