
#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::BaseWebSocketClient
    @brief WebSocket client wrapper
*/

#include "Core/Types.h"
#include "Core/Containers/Buffer.h"
#include "IO/IOTypes.h"
#include "Core/Time/Clock.h"
#include <functional>

#include "easywsclient/easywsclient.hpp"

using easywsclient::WebSocket;

namespace Oryol {

    class BaseWebSocketClientSetup {
    public:
        Oryol::URL ServerUrl;
        std::function<void(Oryol::String msg)> MessageFunc;
        std::function<void(Oryol::String msg)> ErrorFunc;
        std::function<void()> OpenFunc;
        std::function<void(Oryol::String msg)> CloseFunc;
    };

    class BaseWebSocketClient {
    public:
        /// setup the client, and start connecting to the server
        void Setup(const BaseWebSocketClientSetup &setup);

        /// do per-frame-update, advance state, receive and send messages as needed
        void Update();

        /// asynchronously send a message, return false if send buffer was full
        bool Send(const Oryol::String &msg);

        /// close connection
        void Close();

        WebSocket::pointer ws = nullptr;

        BaseWebSocketClientSetup setup;

        int GetReadyState();
    };
} // namespace Oryol