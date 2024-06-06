#include "includes.h"
#include "handler/handler.h"
#include "sqldriver/driver.h"
#include "encryption/encryption.h"
#include "../resource.h"


int main(int, char**)
{
    // poveži z bazo, pridobi vse tabele, podatke
    if (!driver.Run())
        return EXIT_FAILURE;

    if (!driver.GetDatabaseVariables())
        return EXIT_FAILURE;

    // glavni loop
    return handler.Run(
        []{
            driver.MainLoop();
        }
    );

    return EXIT_SUCCESS;
}