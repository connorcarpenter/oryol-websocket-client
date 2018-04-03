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

        BaseSocketClientSetup ncs;
        ncs.ServerUrl = this->serverUrl;
        ncs.ReceiveFunc = this->receiveFunc;
        this->BaseSocketClient.Setup(ncs);
    }

    void WebSocketClient::update() {
        this->BaseSocketClient.Update();
    }

    void WebSocketClient::receive(std::function<void(Oryol::String msg)> receiveFunction){
        this->receiveFunc = receiveFunction;
        this->connect(this->serverUrl);
    }

    void WebSocketClient::send(Oryol::String msg){
        this->BaseSocketClient.Send(msg);
    }
}