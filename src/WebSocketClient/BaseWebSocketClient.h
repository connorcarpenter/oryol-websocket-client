
#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::BaseWebSocketClient
    @brief WebSocket client wrapper
*/

#include "easywsclient/easywsclient.hpp"
using easywsclient::WebSocket;

namespace Oryol {

    class BaseWebSocketClientSetup {
    public:
        Oryol::URL ServerUrl = "ws://localhost:3001";
        std::function<void(Oryol::String msg)> ReceiveFunc;
    };

    class BaseWebSocketClient {
    public:
        /// setup the client, and start connecting to the server
        void Setup(const BaseWebSocketClientSetup &setup);

        /// do per-frame-update, advance state, receive and send messages as needed
        void Update();

        /// asynchronously send a message, return false if send buffer was full
        bool Send(const Oryol::String &msg);

        WebSocket::pointer ws = nullptr;

        BaseWebSocketClientSetup setup;
    };
} // namespace Oryol