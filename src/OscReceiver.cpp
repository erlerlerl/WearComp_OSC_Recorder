#include "OscReceiver.hpp"

#include <iostream>
#include <lo/lo.h>

namespace {

int oscHandler (const char* path, const char* types, lo_arg** argv, int argc, lo_message msg, void* user_data
){

    std::cout << "OSC received " << path;


    for (int i = 0; i < argc; i++) {
        switch (types[i]) {
            case 'f':
                std::cout << " " << argv[i]->f;
                break;
            case 'i':
                std::cout << " " << argv[i]->i;
                break;
            case 's':
                std::cout << " " << &argv[i]->s;
                break;
        }
    }
    std::cout << std::endl;
    return 0;

}
}

OscReceiver::OscReceiver(int port) : port(port), server(nullptr) {

}

OscReceiver::~OscReceiver() {
    if (server) {
        lo_server_free(static_cast<lo_server>(server));
  }
}

bool OscReceiver::start() {
    server = lo_server_new(
        std::to_string(port).c_str(), nullptr
    );

    if (!server) {
        std::cerr << "Could not create OSC server on port " << port << std::endl;
        return false;
    }

    lo_server_add_method(static_cast<lo_server>(server),     // server
                         nullptr,    // alle OSC Paths
                         nullptr,    // ally type tags
                         oscHandler, // Methode
                         nullptr
    );

    std::cout << "OSC server listening on port " << port << std::endl;

    return true;
}

void OscReceiver::run() {
    while (true) {
        lo_server_recv(static_cast<lo_server>(server));
  }
}
