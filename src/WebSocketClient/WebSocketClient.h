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
        void onMessage(std::function<void(Oryol::String msg)> receiveFunction);
        void onError(std::function<void(Oryol::String msg)> errorFunction);
        void onOpen(std::function<void()> openFunction);
        void onClose(std::function<void(Oryol::String msg)> closeFunction);
        void send(Oryol::String msg);
        void close();
        int getReadyState();
    private:
        Oryol::BaseSocketClient baseSocketClient;
        Oryol::BaseWebSocketClient baseWebSocketClient;
        Oryol::String serverUrl;
        std::function<void(Oryol::String msg)> messageFunc;
        std::function<void(Oryol::String msg)> errorFunc;
        std::function<void(Oryol::String msg)> closeFunc;
        std::function<void()> openFunc;

        #if ORYOL_EMSCRIPTEN
        bool useSocketClient = true;
        #else
        bool useSocketClient = false;
        #endif

        void setEscapeCharacter(uint8_t c);
    };
}