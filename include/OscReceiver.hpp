#pragma once

#include <string>
#include <lo/lo.h>



class OscReceiver {
    public:
        explicit OscReceiver(int port);
        ~OscReceiver();

        bool start();
        void run();

    private:
        int port;
        lo_server server;
};
