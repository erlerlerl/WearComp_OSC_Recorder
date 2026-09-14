#include "OscReceiver.hpp"
#include "OscMessage.hpp"

#include <iostream>
#include <lo/lo.h>

namespace {

int oscHandler (const char* path, const char* types, lo_arg** argv, int argc, lo_message msg, void* user_data
){

    std::vector<OscMessage::Argument> arguments;

    for (int i = 0; i < argc; i++) {
        switch (types[i]) {
            case 'f':
                arguments.emplace_back(argv[i]->f);
                break;
            case 'i':
                arguments.emplace_back(argv[i]->i);
                break;
            case 's':
                arguments.emplace_back(&argv[i]->s);
                break;
            default:
                std::cerr << "Unsupported OSC argument type " << types[i] << std::endl;
        }
    }

    OscMessage message(std::string(path), std::move(arguments));

    std::cout << "OSC received: " << message.address();

    for (const auto &argument : message.arguments()) {
        std::visit([](const auto& value) {std::cout << " " << value;}, argument);
    }

    std::cout << std::endl;
    return 0;

}
}

OscReceiver::OscReceiver(int port) : port(port), server(nullptr) {

}

OscReceiver::~OscReceiver() {
    if (server) {
        lo_server_free(server);
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

    lo_server_add_method(server,     // server
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
        lo_server_recv(server);
  }
}
