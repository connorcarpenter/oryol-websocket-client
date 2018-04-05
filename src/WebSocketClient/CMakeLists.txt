fips_begin_module(WebSocketClient)
    fips_vs_warning_level(3)
    fips_files(
            BaseWebSocketClient.cpp BaseWebSocketClient.h
            BaseSocketClient.cpp BaseSocketClient.h
            WebSocketClient.cpp WebSocketClient.h
            easywsclient/easywsclient.cpp
            easywsclient/easywsclient.hpp)
    fips_deps(Core IO)
fips_end_module()
