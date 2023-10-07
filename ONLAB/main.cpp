#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "app.h"


    int main() {
        v::App app{};

        try {
            app.run();
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }


        return EXIT_SUCCESS;
    }

