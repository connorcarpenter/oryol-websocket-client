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

    this->ws = WebSocket::from_url(setup.ServerUrl.Get().AsCStr());

    this->setup = setup;
}

//------------------------------------------------------------------------------
void BaseWebSocketClient::Update() {
    if (this->ws) {
        if (ws->getReadyState() != WebSocket::CLOSED) {
            this->ws->poll();
            this->ws->dispatch([this](const std::string &message) {
                Oryol::String newstr(message.c_str());
                this->setup.MessageFunc(newstr);
            });
        }
    }
}

//------------------------------------------------------------------------------
bool BaseWebSocketClient::Send(const String& msg) {
    if (this->ws) {
        if (ws->getReadyState() != WebSocket::CLOSED) {
            this->ws->send(msg.AsCStr());
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
