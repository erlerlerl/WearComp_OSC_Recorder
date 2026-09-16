#include "OscReceiver.hpp"

#include <iostream>


auto main(int argc, char *argv[]) -> int {

    OscReceiver receiver(30181);


    receiver.setMessageHandler([](const OscMessage &message) {

        std::cout << "OSC received: " << message.address();

        std::cout << " at " << message.timestamp().count();

        for (const auto &argument : message.arguments()) {
            std::visit([](const auto& value) {std::cout << " " << value;}, argument);
        }

        std::cout << std::endl;
    });

    if (!receiver.start()) {
        return 1;
    }

    receiver.run();

    return 0;
}
    // std::cout << "OSC received: " << message.address();

    // for (const auto &argument : message.arguments()) {
    //     std::visit([](const auto& value) {std::cout << " " << value;}, argument);
    // }

    // std::cout << std::endl;
