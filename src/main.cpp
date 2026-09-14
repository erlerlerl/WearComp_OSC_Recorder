#include <iostream>
#include <lo/lo.h>

int oscHandler (const char* path, const char* types, lo_arg** argv, int argc, lo_message msg, void* user_data
){

    std::cout << "OSC received " << path;

    for (int i = 0; i < argc; i++) {
        if (types[i] == 'f') {
            std::cout << " " << argv[i]->f;
        } else if (types[i] == 'i') {
            std::cout << " " << argv[i]->i;
        } else if (types[i] == 's') {
            std::cout << " " << &argv[i]->s;
        }

    }
    std::cout << std::endl;
    return 0;

}

auto main(int argc, char *argv[]) -> int {

    lo_server server = lo_server_new("9000", nullptr);

    if (!server) {
        std::cerr << "Could not create OSC server \n";
        return 1;
    }

    lo_server_add_method(server,  // server
                         nullptr, // alle OSC Paths
                         nullptr, // ally type tags
                         oscHandler, // Methode
                         nullptr);

    std::cout << "OSC server listening on port 9000...\n";

    while (true) {
        lo_server_recv(server);
    }

    lo_server_free(server);

    return 0;
}
