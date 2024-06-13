#include "driver.h"
#include "../imgui/imgui_stdlib.h"
#include "../menu/menu.h"

bool Driver::Run()
{
	try {
		sql::SQLString url("jdbc:mariadb://localhost:3306/zdravstvena");
		sql::Properties properties({ {"user", "root"}, {"password", "bababoey"} });
		g_sqlDriver = sql::mariadb::get_driver_instance();
		g_sqlConnection = g_sqlDriver->connect("tcp://127.0.0.1:3306", "root", "bababoey");
		g_sqlConnection->setSchema("zdravstvena");

		if (!g_sqlConnection->isValid())
			return false;

		return true;
	}
	catch (sql::SQLException& e)
	{
#ifdef _DEBUG
		cerr << "Error connecting to database: " << e.what() << endl;
#endif
	}

	return false;
}

int Driver::input_filter_numbers_only(ImGuiInputTextCallbackData* data)
{
	auto c = data->EventChar;
	if (c >= '0' && c <= '9')
		return 0;
	return 1;
}

bool Driver::emso_verify(string emso)
{
	return true;

	if (emso.empty()) // 13 mestno
		return false;

	for (auto& c : emso)
		if (!(c >= '0' && c <= '9')) // samo številke
			return false;

	static int factor_map[] = { 7, 6, 5, 4, 3, 2, 7, 6, 5, 4, 3, 2 };
	short control_digit = 0, sum = 0;
	for (int i = 0; i < 12; ++i)
		sum += (int)(emso[i] - '0') * factor_map[i];

	control_digit = sum % 11 == 0 ? 0 : 11 - (sum % 11);// Seštevek se deli z enajst (delitev se omeji na celo število).
														// Ostanek pri delitvi se odšteje od števila 11, razlika je kontrolna številka.Kontrolna številka je enomestna, ima lahko vrednost od 0 do 9.
														// Če je ostanek pri delitvi O je kontrolna številka 0.
	return control_digit == (int)(emso[12] - '0');
}

void Driver::MainLoop()
{
	menu.Draw();
}

sql::ResultSet* Driver::ExecuteQuery(const string& query)
{
	try {
		sql::Statement* stmt;
		sql::ResultSet* results;

		stmt = g_sqlConnection->createStatement();
		results = stmt->executeQuery(query.c_str());

		return results;
	}
	catch (sql::SQLException& e)
	{
#ifdef _DEBUG
		cerr << "Error executing sql query (" << query << "): " << e.what() << endl;
#endif
	}
	return nullptr;
}

int Driver::ExecuteUpdate(const string& query)
{
	try {
		sql::Statement* stmt;
		stmt = g_sqlConnection->createStatement();
		auto res = stmt->executeUpdate(query.c_str());

		GetDatabaseVariables();

		return res;
	}
	catch (sql::SQLException& e)
	{
#ifdef _DEBUG
		cerr << "Error executing sql update (" << query << "): " << e.what() << endl;
#endif
		return e.getErrorCode();
	}
}

string Driver::date_to_sql(tm date, bool include_time)
{
	char buffer[25];
	date.tm_sec = 0;
	if (include_time)
		std::strftime(buffer, sizeof(buffer), "%F %T", &date);
	else
		std::strftime(buffer, sizeof(buffer), "%F", &date);
	return string(buffer);
}

tm Driver::sql_to_date(string date, bool include_time)
{
	std::tm time = {};
	std::istringstream ss(date);

	ss.imbue(std::locale("C"));

	if (include_time)
		ss >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
	else
		ss >> std::get_time(&time, "%Y-%m-%d");

	return time;
}

bool Driver::GetDatabaseVariables()
{
	m_pacienti.clear();
	m_termini.clear();
	m_doktroji.clear();
	m_zapisniki.clear();
	m_oddelki.clear();
	m_bolnice.clear();
	m_zdravila.clear();
	m_recepti.clear();
	m_recepti_has_zdravila.clear();

	std::unique_ptr<sql::ResultSet> results_pacient(driver.ExecuteQuery("SELECT * FROM pacient"));
	if (results_pacient == nullptr)
		return false;
	while (results_pacient->next()) // row
		m_pacienti.push_back(Pacient(results_pacient->getInt(1), string(results_pacient->getString(2)), string(results_pacient->getString(3)), string(results_pacient->getString(4)), string(results_pacient->getString(5)), string(results_pacient->getString(6))));

	std::unique_ptr<sql::ResultSet> results_bolnice(driver.ExecuteQuery("SELECT * FROM bolnica"));
	if (results_bolnice == nullptr)
		return false;
	while (results_bolnice->next()) // row
		m_bolnice.push_back(Bolnica(results_bolnice->getInt(1), string(results_bolnice->getString(2)), string(results_bolnice->getString(3)), string(results_bolnice->getString(4)), results_bolnice->getShort(5)));


	std::unique_ptr<sql::ResultSet> results_oddelki(driver.ExecuteQuery("SELECT * FROM oddelek"));
	if (results_oddelki == nullptr)
		return false;
	while (results_oddelki->next()) // row
	{
		auto bolnica_id = results_oddelki->getInt(3);
		for (auto& b : m_bolnice)
			if (bolnica_id == b.m_id)
				m_oddelki.push_back(Oddelek(results_oddelki->getInt(1), string(results_oddelki->getString(2)), &b));
	}

	std::unique_ptr<sql::ResultSet> results_doktor(driver.ExecuteQuery("SELECT * FROM doktor"));
	if (results_doktor == nullptr)
		return false;
	while (results_doktor->next()) // row
	{
		auto oddelek_id = results_doktor->getInt(5);
		for (auto& o : m_oddelki)
			if (oddelek_id == o.m_id)
				m_doktroji.push_back(Doktor(results_doktor->getInt(1), string(results_doktor->getString(2)), string(results_doktor->getString(3)), string(results_doktor->getString(4)), &o));
	}

	std::unique_ptr<sql::ResultSet> results_termin(driver.ExecuteQuery("SELECT * FROM termin"));
	if (results_termin == nullptr)
		return false;
	while (results_termin->next()) // row
	{
		Doktor* doktor = nullptr;
		Pacient* pacient = nullptr;
		auto doktor_id = results_termin->getInt(3);
		auto pacient_id = results_termin->getInt(4);
		for (auto& d : m_doktroji)
			if (doktor_id == d.m_id)
				doktor = &d;
		for (auto& p : m_pacienti)
			if (pacient_id == p.m_id)
				pacient = &p;
				
		if (!doktor || !pacient)
			return false;

		m_termini.push_back(Termin(results_termin->getInt(1), string(results_termin->getString(2)), doktor, pacient));
	}


	std::unique_ptr<sql::ResultSet> results_zapisnik(driver.ExecuteQuery("SELECT * FROM zapisnik"));
	if (results_zapisnik == nullptr)
		return false;
	while (results_zapisnik->next()) // row
	{
		auto pacient_id = results_zapisnik->getInt(5);
		for (auto& p : m_pacienti)
			if (pacient_id == p.m_id)
				m_zapisniki.push_back(Zapisnik(results_zapisnik->getInt(1), string(results_zapisnik->getString(2)), string(results_zapisnik->getString(3)), string(results_zapisnik->getString(4)), &p));
	}
	
	std::unique_ptr<sql::ResultSet> results_zdravilo(driver.ExecuteQuery("SELECT * FROM zdravilo"));
	if (results_zdravilo == nullptr)
		return false;
	while (results_zdravilo->next()) // row
		m_zdravila.push_back(Zdravilo(results_zdravilo->getInt(1), string(results_zdravilo->getString(2)), results_zdravilo->getInt(3), string(results_zdravilo->getString(4))));

	std::unique_ptr<sql::ResultSet> results_recept(driver.ExecuteQuery("SELECT * FROM recept"));
	if (results_recept == nullptr)
		return false;
	while (results_recept->next()) // row
	{
		auto pacient_id = results_recept->getInt(3);
		for (auto& p : m_pacienti)
			if (pacient_id == p.m_id)
				m_recepti.push_back(Recept(results_recept->getInt(1), string(results_recept->getString(2)), &p));
	}

	std::unique_ptr<sql::ResultSet> results_recepti_has_zdravila(driver.ExecuteQuery("SELECT * FROM recepti_has_zdravila"));
	if (results_recepti_has_zdravila == nullptr)
		return false;
	while (results_recepti_has_zdravila->next()) // row
	{
		auto zdravilo_id = results_recepti_has_zdravila->getInt(1);
		auto recept_id = results_recepti_has_zdravila->getInt(2);
		auto recept_it = std::find_if(m_recepti.begin(), m_recepti.end(), [recept_id](const Recept& r) {
			return r.m_id == recept_id;
			});
		if (recept_it == m_recepti.end())
			return false;

		auto zdravilo_it = std::find_if(m_zdravila.begin(), m_zdravila.end(), [zdravilo_id](const Zdravilo& z) {
			return z.m_id == zdravilo_id;
			});

		if (zdravilo_it == m_zdravila.end())
			return false;
		m_recepti_has_zdravila.emplace_back(&(*zdravilo_it), &(*recept_it));
	}

	return true;
}

Driver driver;