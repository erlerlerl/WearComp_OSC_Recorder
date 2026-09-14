#pragma once

#include <string>

class OscReceiver {
    public:
        explicit OscReceiver(int port);
        ~OscReceiver();

        bool start();
        void run();

    private:
        int port;
        void* server;
};
