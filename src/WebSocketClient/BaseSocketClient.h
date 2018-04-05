
#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::BaseSocketClient
    @brief WebSocket client wrapper
*/
#include "Core/Types.h"
#include "Core/Containers/Buffer.h"
#include "IO/IOTypes.h"
#include "Core/Time/Clock.h"
#include <functional>

namespace Oryol {

    class BaseSocketClientSetup {
    public:
        Oryol::URL ServerUrl;
        std::function<void(Oryol::String msg)> ReceiveFunc;
    };

    class BaseSocketClient {
    public:
        enum State {
            Disconnected,
            Connected,
        };
        static const int MaxSendBufferSize = 16 * 1024;

        /// setup the client, and start connecting to the server
        void Setup(const BaseSocketClientSetup& setup);
        /// shutdown the client, disconnect if currently connected
        void Discard();
        /// connect to a different server address
        void Connect(const Oryol::URL& url);
        /// initiate a disconnect from server
        void Disconnect(int waitFramesUntilReconnect);
        /// do per-frame-update, advance state, receive and send messages as needed
        void Update();
        /// asynchronously send a message, return false if send buffer was full
        bool Send(const Oryol::String& msg);

        /// get the server address
        const char* ServerAddress() const {
            return this->setup.ServerUrl.AsCStr();
        }
        /// return true if currently connected
        bool IsConnected() const {
            return Connected == this->state;
        }
        /// return true if currently disconnected
        bool IsDisconnected() const {
            return Disconnected == this->state;
        }
        /// get time since last message was received
        Oryol::Duration HeartbeatTime() const {
            return Oryol::Clock::Since(this->lastRecvTime);
        }


    private:
        /// create the client socket
        void createSocket();
        /// destroy the client socket
        void destroySocket();
        /// start connecting, called when currently disconnected
        void doConnect();
        /// called while in connecting state, handle connection handshake
        void onConnecting();
        /// called while in connected state, send and receive messages
        void onConnected();
        /// send the next chunk of data from the send buffer
        bool sendNextChunk();
        /// receive the next chunk of data and write to recv buffer
        bool recvNextChunk();
        /// scan receive buffer for complete messages, and emit them
        void scanMessages();

        BaseSocketClientSetup setup;
        Oryol::TimePoint lastRecvTime;
        State state = Disconnected;

        Oryol::Buffer sendBuffer;
        Oryol::Buffer recvBuffer;

        int sock = 0;

        int waitFrames = 0;
    };

} // namespace Oryol