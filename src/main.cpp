#include "includes.h"
#include "handler/handler.h"
#include "sqldriver/driver.h"
#include "encryption/encryption.h"
#include "../resource.h"


int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    // poveži z bazo, pridobi vse tabele, podatke
    if (!driver.Run())
        return EXIT_FAILURE;

    // glavni loop
    return handler.Run(hInst, cmdshow,
        []{
            driver.MainLoop();
        }
    );
}