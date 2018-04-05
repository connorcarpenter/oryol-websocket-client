//
// Created by Connor Carpenter on 12/14/17.
//

#include "Core/Log.h"
#include "BaseSocketClient.h"
#include "BaseWebSocketClient.h"

namespace Oryol {

    class WebSocketClient {
    public:
        WebSocketClient();
        void connect(Oryol::String serverUrl);
        void update();
        void receive(std::function<void(Oryol::String msg)> receiveFunction);
        void send(Oryol::String msg);
    private:
        Oryol::BaseSocketClient baseSocketClient;
        Oryol::BaseWebSocketClient baseWebSocketClient;
        Oryol::String serverUrl;
        std::function<void(Oryol::String msg)> receiveFunc;

        #if ORYOL_EMSCRIPTEN
        bool useSocketClient = true;
        #else
        bool useSocketClient = false;
        #endif
    };
}