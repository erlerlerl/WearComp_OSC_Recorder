#pragma once

#include "OscMessage.hpp"

#include <functional>
#include <lo/lo.h>



class OscReceiver {
    public:
        using MessageHandler = std::function<void(const OscMessage&)>;
        explicit OscReceiver(int port);
        ~OscReceiver();

        bool start();
        void run();

        void setMessageHandler(MessageHandler handler);

        MessageHandler messageHandler;
    private:
        int port;
        lo_server server;
};
