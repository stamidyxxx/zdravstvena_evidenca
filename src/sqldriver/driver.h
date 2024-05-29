#pragma once
#include "../includes.h"
#include "../encryption/encryption.h"

class Driver
{
public:
	bool Run();
	void MainLoop();
	sql::ResultSet* ExecuteQuery(const string& query);
	template<typename... Args>
	sql::ResultSet* ExecuteQuery(const string& query, Args&&... args);
	int ExecuteUpdate(const string& query);
	template<typename... Args>
	int ExecuteUpdate(const string& query, Args&&... args);

private:
	sql::Driver* g_sqlDriver;
	sql::Connection* g_sqlConnection;

	bool m_logged_in = false;
	bool m_register_prompt = false;
	string m_login_error = "";
	ImVec2 m_screen_size = ImVec2(0, 0);
};

extern Driver driver;