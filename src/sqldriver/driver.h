#pragma once
#include "../includes.h"

using namespace std;

class Bolnica
{
public:
	int m_id;
	string m_ime;
	string m_naslov;
	string m_tel_st;

};

class Driver
{
public:
	bool Run();
	void MainLoop();
	sql::ResultSet* ExecuteQuery(string query);

	template<typename... Args>
	sql::ResultSet* ExecuteQuery(const std::string& query, Args&&... args);

private:
	sql::Driver* g_sqlDriver;
	sql::Connection* g_sqlConnection;
};

extern Driver driver;