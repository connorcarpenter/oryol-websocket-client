fips_begin_module(WebSocketClient)
    fips_vs_warning_level(3)
    fips_files(
            BaseSocketClient.cpp BaseSocketClient.h
            WebSocketClient.cc WebSocketClient.h)
fips_end_module()

