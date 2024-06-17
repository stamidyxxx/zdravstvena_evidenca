#include "includes.h"
#include "handler/handler.h"
#include "sqldriver/driver.h"
#include "encryption/encryption.h"
#include "../resource.h"


static int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    // poveži z bazo, pridobi vse tabele, podatke
    if (!driver.Run())
        return EXIT_FAILURE;

    settings.Load();

    // glavni loop
    auto ret = handler.Run(hInst, cmdshow,
        []{
            driver.MainLoop();
        }
    );

    settings.Save();

    return ret;
}