#pragma once
#include "../includes.h"
#include "../encryption/encryption.h"

class Pacient
{
public:
	int m_id;
	string m_ime;
	string m_priimek;
	string m_naslov;
	string m_tel_st;

	Pacient(int m_id, const string& m_ime, const string& m_priimek, const string& m_naslov, const string& m_tel_st)
		: m_id(m_id), m_ime(m_ime), m_priimek(m_priimek), m_naslov(m_naslov), m_tel_st(m_tel_st)
	{
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_ime;
		case 2: return m_priimek;
		case 3: return m_naslov;
		case 4: return m_tel_st;
		default: return 0;
		} 
	}
};

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

	vector<Pacient> m_pacienti;

private:
	sql::Driver* g_sqlDriver;
	sql::Connection* g_sqlConnection;

	bool m_logged_in = false;
	bool m_register_prompt = false;
	string m_login_error = "";
	ImVec2 m_screen_size = ImVec2(0, 0);
};

extern Driver driver;