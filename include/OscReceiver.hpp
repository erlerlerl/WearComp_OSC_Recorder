#pragma once

#include "OscMessage.hpp"

#include <functional>
#include <lo/lo.h>
#include <chrono>



class OscReceiver {
    public:
        using MessageHandler = std::function<void(const OscMessage&)>;
        explicit OscReceiver(int port);
        ~OscReceiver();

        bool start();
        void run();

        void setMessageHandler(MessageHandler handler);

        MessageHandler messageHandler;
        std::chrono::steady_clock::time_point startTime;
    private:
        int port;
        lo_server server;

};
