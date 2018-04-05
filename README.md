# oryol-websocket-client
A WebSocket Client built from &amp; written for use with Oryol https://github.com/floooh/oryol

Currently confirmed working on Native Linux and compiled to Wasm

How to Use
==========

```1
class App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    
    WebSocketClient webSocketClient;
}

AppState::Code
App::OnInit() {
    //connect to websocket URL
    this->webSocketClient.connect("ws://localhost:3001");
    
    //set up receive method
    this->webSocketClient.receive([](Oryol::String msg){
        Log::Info("App received message from server: %s\n", msg.AsCStr());
    });
}

AppState::Code
App::OnRunning() {
    //this polls the server for new messages, and sends out the app's message.
    this->webSocketClient.update();
    
    //send out a new message to server
    this->webSocketClient.send("this is the client");
}
```

My goal with this was to have an easy abstraction of a websocket client that could connect to my server from both:

1. The Emscripten-compiled app running in the browser
2. The natively compilex Linux app

This was a challenge because in order to connect to the Websocket Server from the browser, you need to write as if you are connecting to standard sockets, and Emscripten changes them to websockets on compile.
So you'll see that the WebSocketClient class makes calls through BaseSocketClient when compiled using Emscripten, and BaseWebSocketClient when compiled natively.

This is a plus if you want your app to work in the browser and in native Linux, yeah, but more importantly, this allows you to debug your app as you normally would when compiled natively (because the best you can do at debugging code after having compiled to WebAssembly is print logging messages). So with this, you should be able to safely step through your code as your client connects to your server when you're debugging AND when you've deployed to the browser.

Any suggestions welcome! Thanks