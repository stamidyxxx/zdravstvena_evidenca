#include "includes.h"
#include "handler/handler.h"
#include "sqldriver/driver.h"
#include "encryption/encryption.h"

int main(int, char**)
{
    // poveži z bazo, pridobi vse tabele, podatke
    if (!driver.Run())
        return EXIT_FAILURE;

    std::unique_ptr<sql::ResultSet> results(driver.ExecuteQuery("SELECT * FROM pacient"));
    if (results == nullptr)
        return EXIT_FAILURE;

    while (results->next()) // row
    {
        driver.m_pacienti.push_back(Pacient(results->getInt(1), string(results->getString(2)), string(results->getString(3)), string(results->getString(4)), string(results->getString(5))));
    }

    // glavni loop
    return handler.Run(
        []{
            driver.MainLoop();
        }
    );
}