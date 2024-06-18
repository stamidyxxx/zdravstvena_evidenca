#pragma once
#include "../includes.h"
#include "../encryption/encryption.h"
#include <deque>

class Bolnica
{
public:
	int m_id;
	string m_ime;
	string m_naslov;
	string m_tel_st;
	unsigned short m_postna_st;

	Bolnica(int m_id, const string& m_ime, const string& m_naslov, const string& m_tel_st, unsigned short m_postna_st)
		: m_id(m_id), m_ime(m_ime), m_naslov(m_naslov), m_tel_st(m_tel_st), m_postna_st(m_postna_st)
	{
	}

	Bolnica()
		: m_id(-1), m_ime(""), m_naslov(""), m_tel_st(""), m_postna_st(0)
	{
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_ime;
		case 2: return m_naslov;
		case 3: return m_tel_st;
		case 4: return to_string(m_postna_st);
		default: return "";
		}
	}
};

class Pacient
{
public:
	int m_id;
	string m_ime;
	string m_priimek;
	string m_naslov;
	string m_tel_st;
	string m_emso;

	Pacient(int m_id, const string& m_ime, const string& m_priimek, const string& m_naslov, const string& m_tel_st, const string& m_emso)
		: m_id(m_id), m_ime(m_ime), m_priimek(m_priimek), m_naslov(m_naslov), m_tel_st(m_tel_st), m_emso(m_emso)
	{
	}
	Pacient()
		: m_id(-1), m_ime(""), m_priimek(""), m_naslov(""), m_tel_st(""), m_emso("")
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
		case 3: return m_naslov;
		case 4: return m_tel_st;
		case 5: return m_emso;
		default: return "";
		} 
	}
};

class Oddelek
{
public:
	int m_id;
	string m_ime;
	Bolnica* m_bolnica;

	Oddelek(int m_id, const string& m_ime, Bolnica* m_bolnica)
		: m_id(m_id), m_ime(m_ime), m_bolnica(m_bolnica)
	{
	}
	Oddelek()
		: m_id(-1), m_ime(""), m_bolnica(nullptr)
	{
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_ime;
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
	Oddelek* m_oddelek;

	Doktor(int m_id, const string& m_ime, const string& m_priimek, const string& m_tel_st, Oddelek* m_oddelek)
		: m_id(m_id), m_ime(m_ime), m_priimek(m_priimek), m_tel_st(m_tel_st), m_oddelek(m_oddelek)
	{
	}
	Doktor()
		: m_id(-1), m_ime(""), m_priimek(""), m_tel_st(""), m_oddelek(nullptr)
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
		default: return "";
		}
	}
};

class Termin
{
public:
	int m_id;
	string m_cas_datum;
	Doktor* m_doktor;
	Pacient* m_pacient;

	Termin(int m_id, const string& m_cas_datum, Doktor* m_doktor, Pacient* m_pacient)
		: m_id(m_id), m_cas_datum(m_cas_datum), m_doktor(m_doktor), m_pacient(m_pacient)
	{
	}

	Termin()
		: m_id(-1), m_cas_datum(""), m_doktor(nullptr), m_pacient(nullptr)
	{
	}

	string get_text()
	{
		return m_cas_datum + " - " + m_doktor->get_ime_priimek() + " - " + m_pacient->get_ime_priimek();
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_cas_datum;
		default: return "";
		}
	}
};

class Zapisnik
{
public:
	int m_id;
	string m_naslov;
	string m_simptomi;
	string m_znaki;
	string m_diagnoza;
	string m_zdravljenje;
	string m_datum;
	Pacient* m_pacient;

	Zapisnik(int m_id, const string& m_naslov, const string& m_simptomi, const string& m_znaki, const string& m_diagnoza, const string& m_zdravljenje, const string& m_datum, Pacient* m_pacient)
		: m_id(m_id), m_naslov(m_naslov), m_simptomi(m_simptomi), m_znaki(m_znaki), m_diagnoza(m_diagnoza), m_zdravljenje(m_zdravljenje), m_datum(m_datum), m_pacient(m_pacient)
	{
	}
	Zapisnik()
		: m_id(-1), m_naslov(""), m_simptomi(""), m_znaki(""), m_diagnoza(""), m_zdravljenje(""), m_datum(""), m_pacient(nullptr)
	{
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_naslov;
		case 2: return m_simptomi;
		case 3: return m_znaki;
		case 5: return m_diagnoza;
		case 6: return m_zdravljenje;
		case 7: return m_datum;
		default: return "";
		}
	}
};

class Zdravilo
{
public:
	int m_id;
	string m_ime;
	int m_kolicina;
	string m_opis;



	Zdravilo(int m_id, const string& m_ime, int m_kolicina, const string& m_opis)
		: m_id(m_id), m_ime(m_ime), m_kolicina(m_kolicina), m_opis(m_opis)
	{
	}
	Zdravilo()
		: m_id(-1), m_ime(""), m_kolicina(-1), m_opis("")
	{
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_ime;
		case 2: return to_string(m_kolicina);
		case 3: return m_opis;
		default: return "";
		}
	}
};

class Recept
{
public:
	int m_id;
	string m_cas_datum;
	Pacient* m_pacient;

	string m_zdravila;
	unordered_map<int, string> m_selected_zdravila;

	Recept(int m_id, const string& m_cas_datum, Pacient* m_pacient)
		: m_id(m_id), m_cas_datum(m_cas_datum), m_pacient(m_pacient), m_zdravila(""), m_selected_zdravila()
	{
	}
	Recept()
		: m_id(-1), m_cas_datum(""), m_pacient(nullptr), m_zdravila(""), m_selected_zdravila()
	{
	}

	string get_text()
	{
		return m_cas_datum + " - " + m_pacient->get_ime_priimek();
	}

	string get(size_t idx) {
		switch (idx)
		{
		case 0: return to_string(m_id);
		case 1: return m_cas_datum;
		default: return "";
		}
	}
};

class Recepti_has_Zdravila
{
public:
	Zdravilo* m_zdravilo;
	Recept* m_recept;

	Recepti_has_Zdravila(Zdravilo* m_zdravilo, Recept* m_recept)
		: m_zdravilo(m_zdravilo), m_recept(m_recept)
	{
	}

	Recepti_has_Zdravila()
		: m_zdravilo(nullptr), m_recept(nullptr)
	{
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

	string date_to_sql(tm date, bool include_time = true);
	tm sql_to_date(string date, bool include_time = true);

	bool GetDatabaseVariables();

	bool emso_verify(string emso);
	static int input_filter_numbers_only(ImGuiInputTextCallbackData* data);

	vector<Pacient> m_pacienti;
	vector<Pacient> m_filtered_pacienti;

	vector<Doktor> m_doktroji;
	vector<Doktor> m_filtered_doktroji;
	vector<string> m_doktorji_imena;

	vector<Termin> m_termini;

	vector<Zapisnik> m_zapisniki;

	vector<Oddelek> m_oddelki;
	vector<Oddelek> m_filtered_oddeleki;
	vector<string> m_oddelki_imena;

	vector<Bolnica> m_bolnice;
	vector<string> m_bolnice_imena;

	vector<Zdravilo> m_zdravila;
	vector<Zdravilo> m_filtered_zdravila;
 	vector<string> m_zdravila_imena;

	vector<Recept> m_recepti;

	vector<Recepti_has_Zdravila> m_recepti_has_zdravila;

	bool m_logged_in = false;
	bool m_register_prompt = false;
	string m_login_error = "";
	ImVec2 m_screen_size = ImVec2(0, 0);

private:
	sql::Driver* g_sqlDriver = nullptr;
	sql::Connection* g_sqlConnection = nullptr;
};

template<typename... Args>
sql::ResultSet* Driver::ExecuteQuery(const string& query, Args&&... args)
{
	return ExecuteQuery(std::vformat(query, std::make_format_args(std::forward<Args>(args)...)));
}

template<typename... Args>
int Driver::ExecuteUpdate(const string& query, Args&&... args)
{
	return ExecuteUpdate(std::vformat(query, std::make_format_args(std::forward<Args>(args)...)));
}

extern Driver driver;