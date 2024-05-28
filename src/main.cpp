#include "includes.h"
#include "handler/handler.h"
#include "sqldriver/driver.h"

int main(int, char**)
{
    // pove�i z bazo, pridobi vse tabele, podatke
    if (!driver.Run())
        return EXIT_FAILURE;

    // glavni loop
    return handler.Run(
        []{
            driver.MainLoop();
        }
    );
}