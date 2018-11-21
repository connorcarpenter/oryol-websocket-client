//
// Created by Connor Carpenter on 12/14/17.
//
#include "WebSocketClient.h"

namespace Oryol {

    WebSocketClient::WebSocketClient() {
        this->messageFunc = [](Oryol::String msg){
            Log::Info("DEFAULT MESSAGE METHOD. Received: %s\n", msg.AsCStr());
        };
        this->errorFunc = [](Oryol::String msg){
            Log::Info("DEFAULT ERROR METHOD. Received: %s\n", msg.AsCStr());
        };
        this->openFunc = [](){
            Log::Info("DEFAULT OPEN METHOD.\n");
        };
        this->closeFunc = [](Oryol::String msg){
            Log::Info("DEFAULT CLOSE METHOD. Received: %s\n", msg.AsCStr());
        };
    }

    void WebSocketClient::connect(Oryol::String serverUrl) {
        Log::Info("WebSocketClient Connecting: %s\n", useSocketClient ? "s" : "ws");
        this->serverUrl = serverUrl;

        if (this->useSocketClient) {
            BaseSocketClientSetup ncs;
            ncs.ServerUrl = this->serverUrl;
            ncs.MessageFunc = this->messageFunc;
            ncs.ErrorFunc = this->errorFunc;
            ncs.OpenFunc = this->openFunc;
            ncs.CloseFunc = this->closeFunc;
            this->baseSocketClient.Setup(ncs);
        } else {
            BaseWebSocketClientSetup ncs;
            ncs.ServerUrl = this->serverUrl;
            ncs.MessageFunc = this->messageFunc;
            ncs.ErrorFunc = this->errorFunc;
            ncs.OpenFunc = this->openFunc;
            ncs.CloseFunc = this->closeFunc;
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

    void WebSocketClient::close(){
        if (this->useSocketClient) {
            this->baseSocketClient.Close();
        } else {
            this->baseWebSocketClient.Close();
        }
    }

    int WebSocketClient::getReadyState(){
        if (this->useSocketClient) {
            return this->baseSocketClient.GetReadyState();
        } else {
            return this->baseWebSocketClient.GetReadyState();
        }
    }

    void WebSocketClient::onMessage(std::function<void(Oryol::String msg)> messageFunction){
        this->messageFunc = messageFunction;
    }

    void WebSocketClient::onError(std::function<void(Oryol::String msg)> errorFunction){
        this->errorFunc = errorFunction;
    }

    void WebSocketClient::onOpen(std::function<void()> openFunction){
        this->openFunc = openFunction;
    }

    void WebSocketClient::onClose(std::function<void(Oryol::String msg)> closeFunction){
        this->closeFunc = closeFunction;
    }

    void WebSocketClient::setEscapeCharacter(uint8_t c)
    {
        this->baseSocketClient.setEscapeCharacter(c);
    }
}