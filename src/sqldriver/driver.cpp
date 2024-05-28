#include "driver.h"

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

void Driver::MainLoop()
{
	std::unique_ptr<sql::ResultSet> results(driver.ExecuteQuery("SELECT * FROM oddelek"));
	if (results == nullptr)
		return;

	try {
		ImGui::ShowStyleEditor();
		if (ImGui::BeginListBox("test"))
		{
			while (results->next())
			{
				if (ImGui::Selectable(results->getString("ime"), false))
				{
					// handle selection
				}
			}
			ImGui::EndListBox();
		}
	}
	catch (sql::SQLException& e)
	{
		cerr << e.what() << endl;
	}
}

sql::ResultSet* Driver::ExecuteQuery(string query)
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

template<typename... Args>
sql::ResultSet* Driver::ExecuteQuery(const std::string& query, Args&&... args)
{
	return std::vformat(query, std::make_format_args(std::forward<Args>(args)...));
}

Driver driver;