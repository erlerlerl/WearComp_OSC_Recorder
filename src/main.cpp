#include "OscReceiver.hpp"

#include <iostream>


auto main(int argc, char *argv[]) -> int {

    OscReceiver receiver(9000);


    if (!receiver.start()) {
        return 1;
    }

    receiver.run();

    return 0;
}
