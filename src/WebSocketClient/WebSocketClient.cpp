//
// Created by Connor Carpenter on 12/14/17.
//
#include "WebSocketClient.h"

namespace Oryol {

    WebSocketClient::WebSocketClient() {
        this->receiveFunc = [](Oryol::String msg){
            Log::Info("DEFAULT RECEIVE METHOD. Received: %s\n", msg.AsCStr());
        };
    }

    void WebSocketClient::connect(Oryol::String serverUrl) {
        Log::Info("WebSocketClient Connecting: %s", useSocketClient ? "s" : "ws");
        this->serverUrl = serverUrl;

        if (this->useSocketClient) {
            BaseSocketClientSetup ncs;
            ncs.ServerUrl = this->serverUrl;
            ncs.ReceiveFunc = this->receiveFunc;
            this->baseSocketClient.Setup(ncs);
        } else {
            BaseWebSocketClientSetup ncs;
            ncs.ServerUrl = this->serverUrl;
            ncs.ReceiveFunc = this->receiveFunc;
            this->baseWebSocketClient.Setup(ncs);
        }
    }

    void WebSocketClient::update() {
        if (this->useSocketClient) {
            this->baseSocketClient.Update();
        } else {
            this->baseWebSocketClient.Update();
        }
    }

    void WebSocketClient::send(Oryol::String msg){
        if (this->useSocketClient) {
            this->baseSocketClient.Send(msg);
        } else {
            this->baseWebSocketClient.Send(msg);
        }
    }

    void WebSocketClient::receive(std::function<void(Oryol::String msg)> receiveFunction){
        this->receiveFunc = receiveFunction;
        this->connect(this->serverUrl);
    }
}