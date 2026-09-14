#include <iostream>
#include <lo/lo.h>

auto main(int argc, char *argv[]) -> int {

    lo_server server = lo_server_new("9000", nullptr);

    if (!server) {
        std::cerr << "Could not create OSC server \n";
        return 1;
    }

    std::cout << "liblo is linked successfully.\n";
    std::cout << "OSC server created on port 9000.\n";

    lo_server_free(server);

    return 0;
}
