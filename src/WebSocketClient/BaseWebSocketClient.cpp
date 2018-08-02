//------------------------------------------------------------------------------
//  BaseWebSocketClient.cc
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

#include "BaseWebSocketClient.h"

using namespace Oryol;

//------------------------------------------------------------------------------
void
BaseWebSocketClient::Setup(const BaseWebSocketClientSetup& setup) {
    if (this->ws != nullptr) {
        delete this->ws;
        this->ws = nullptr;
    }

    this->doConnect();

    this->setup = setup;
}

//------------------------------------------------------------------------------
void BaseWebSocketClient::Update()
{
    if (this->waitFrames > 0) {
        this->waitFrames--;
        return;
    }

    if (this->ws)
    {
        this->onConnected();
    }
    else
    {
        this->doConnect();
    }
}

//------------------------------------------------------------------------------
bool BaseWebSocketClient::Send(const String& msg) {
    if (this->ws) {
        if (ws->getReadyState() != WebSocket::CLOSED) {
            this->ws->sendBinary(msg.AsCStr());
        }
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
void BaseWebSocketClient::Close() {
    this->ws->close();
}

//------------------------------------------------------------------------------
int BaseWebSocketClient::GetReadyState() {
    return this->ws->getReadyState();
}

void BaseWebSocketClient::doConnect() {
    this->ws = WebSocket::from_url(setup.ServerUrl.Get().AsCStr());
}

void BaseWebSocketClient::onConnected()
{
    if (ws->getReadyState() == WebSocket::CLOSED)
    {
        delete this->ws;
        this->ws = nullptr;
        this->waitFrames = 300;
    }
    else
    {
        if (!this->opened)
        {
            this->setup.OpenFunc();
            this->opened = true;
        }
        this->ws->poll();
        this->ws->dispatch([this](const std::string &message){
            Oryol::String newstr(message.c_str());
            this->setup.MessageFunc(newstr);
        });
        if (this->ws->getReadyState() == WebSocket::CLOSED)
        {
            this->setup.CloseFunc("");
        }
    }
}
