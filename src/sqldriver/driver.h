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
	Pacient()
		: m_id(-1), m_ime(""), m_priimek(""), m_naslov(""), m_tel_st("")
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
		default: return "";
		} 
	}
};

class Termin
{
public:
	int m_id;
	string m_cas_datum;
	int m_doktor_id;
	int m_pacient_id;

	Termin(int m_id, const string& m_cas_datum, int m_doktor_id, int m_pacient_id)
		: m_id(m_id), m_cas_datum(m_cas_datum), m_doktor_id(m_doktor_id), m_pacient_id(m_pacient_id)
	{
	}

	Termin()
		: m_id(-1), m_cas_datum(""), m_doktor_id(-1), m_pacient_id(-1)
	{
	}


	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_cas_datum;
		case 2: return to_string(m_doktor_id);
		case 3: return to_string(m_pacient_id);
		default: return "";
		}
	}
};

class Doktor
{
public:
	int m_id;
	string m_ime;
	string m_priimek;
	string m_tel_st;
	int m_oddelek_id;

	Doktor(int m_id, const string& m_ime, const string& m_priimek, const string& m_tel_st, int m_oddelek_id)
		: m_id(m_id), m_ime(m_ime), m_priimek(m_priimek), m_tel_st(m_tel_st), m_oddelek_id(m_oddelek_id)
	{
	}
	Doktor()
		: m_id(-1), m_ime(""), m_priimek(""), m_tel_st(""), m_oddelek_id(-1)
	{
	}

	string get_ime_priimek()
	{
		return m_ime + " " + m_priimek;
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_ime;
		case 2: return m_priimek;
		case 3: return m_tel_st;
		case 4: return to_string(m_oddelek_id);
		default: return "";
		}
	}
};

class Zapisnik
{
public:
	int m_id;
	string m_naslov;
	string m_opis;
	string m_datum;
	int m_pacient_id;

	Zapisnik(int m_id, const string& m_naslov, const string& m_opis, const string& m_datum, int m_pacient_id)
		: m_id(m_id), m_naslov(m_naslov), m_opis(m_opis), m_datum(m_datum), m_pacient_id(m_pacient_id)
	{
	}
	Zapisnik()
		: m_id(-1), m_naslov(""), m_opis(""), m_datum(""), m_pacient_id(-1)
	{
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_naslov;
		case 2: return m_opis;
		case 3: return m_datum;
		case 4: return to_string(m_pacient_id);
		default: return "";
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

	string date_to_sql(tm& date, bool include_time = true);

	bool GetDatabaseVariables();

	vector<Pacient> m_pacienti;
	vector<Pacient> m_filtered_pacienti;

	vector<Termin> m_termini;

	vector<Doktor> m_doktroji;

	vector<Zapisnik> m_zapisniki;
	vector<string> m_zapisniki_naslovi;

private:
	sql::Driver* g_sqlDriver = nullptr;
	sql::Connection* g_sqlConnection = nullptr;

	bool m_logged_in = false;
	bool m_register_prompt = false;
	string m_login_error = "";
	ImVec2 m_screen_size = ImVec2(0, 0);
};

extern Driver driver;