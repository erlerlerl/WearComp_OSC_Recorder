#include "OscReceiver.hpp"
#include "OscMessage.hpp"

#include <iostream>
#include <lo/lo.h>
#include <chrono>

namespace {

int oscHandler (const char* path, const char* types, lo_arg** argv, int argc, lo_message msg, void* user_data
){
    // std::cout << "handler called" << std::endl;
    // std::cout << "path: " << path << std::endl;
    // std::cout << "argc: " << argc << std::endl;
    // std::cout << "types: " << types << std::endl;

    auto * receiver = static_cast<OscReceiver*>(user_data);
    auto timestamp = std::chrono::duration<double>(std::chrono::steady_clock::now() - receiver->startTime);

    std::vector<OscMessage::Argument> arguments;
    // std::cout << "before arguments" << std::endl;

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
    // std::cout << "after arguments" << std::endl;

    OscMessage message(timestamp, std::string(path), std::move(arguments));

    // std::cout << "message created" << std::endl;

    if (receiver->messageHandler) {
        // std::cout << "calling callback" << std::endl;
        receiver->messageHandler(message);
    }

    // std::cout << "callback finished" << std::endl;

    return 0;

}
}

OscReceiver::OscReceiver(int port) : port(port), server(nullptr), messageHandler(nullptr) {

}

OscReceiver::~OscReceiver() {
    if (server) {
        lo_server_free(server);
  }
}

bool OscReceiver::start() {
    startTime = std::chrono::steady_clock::now();
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
                         this
    );

    std::cout << "OSC server listening on port " << port << std::endl;

    return true;
}

void OscReceiver::run() {
    while (true) {
        lo_server_recv(server);
  }
}

void OscReceiver::setMessageHandler(MessageHandler handler) {
    messageHandler = std::move(handler);
}
