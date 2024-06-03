#include "includes.h"
#include "handler/handler.h"
#include "sqldriver/driver.h"
#include "encryption/encryption.h"

int main(int, char**)
{
    // poveži z bazo, pridobi vse tabele, podatke
    if (!driver.Run())
        return EXIT_FAILURE;

    std::unique_ptr<sql::ResultSet> results_pacient(driver.ExecuteQuery("SELECT * FROM pacient"));
    if (results_pacient == nullptr)
        return EXIT_FAILURE;
    while (results_pacient->next()) // row
        driver.m_pacienti.push_back(Pacient(results_pacient->getInt(1), string(results_pacient->getString(2)), string(results_pacient->getString(3)), string(results_pacient->getString(4)), string(results_pacient->getString(5))));

    std::unique_ptr<sql::ResultSet> results_termin(driver.ExecuteQuery("SELECT * FROM termin"));
    if (results_termin == nullptr)
        return EXIT_FAILURE;
    while (results_termin->next()) // row
        driver.m_termini.push_back(Termin(results_termin->getInt(1), string(results_termin->getString(2)), results_termin->getInt(3), results_termin->getInt(4)));

    std::unique_ptr<sql::ResultSet> results_doktor(driver.ExecuteQuery("SELECT * FROM doktor"));
    if (results_doktor == nullptr)
        return EXIT_FAILURE;
    while (results_doktor->next()) // row
        driver.m_doktroji.push_back(Doktor(results_doktor->getInt(1), string(results_doktor->getString(2)), string(results_doktor->getString(3)), string(results_doktor->getString(4)), results_doktor->getInt(5)));

    // glavni loop
    return handler.Run(
        []{
            driver.MainLoop();
        }
    );
}