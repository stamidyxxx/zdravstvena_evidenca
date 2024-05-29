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
	m_screen_size = ImGui::GetIO().DisplaySize;
	ImVec2 container_size = m_screen_size / ImVec2(3, 3);
	ImVec2 center(m_screen_size.x * 0.5f, m_screen_size.y * 0.5f);
	ImVec2 startPos(center.x - container_size.x * 0.5f, center.y - container_size.y * 0.5f);

	if (!m_logged_in)
	{
		if (m_register_prompt)
		{
			ImGui::SetCursorScreenPos(startPos);
			if (ImGui::BeginChild("register_child", container_size, ImGuiChildFlags_Border, ImGuiWindowFlags_NoCollapse & ImGuiWindowFlags_NoMove & ImGuiWindowFlags_NoScrollbar & ImGuiWindowFlags_NoTitleBar & ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + container_size.y * 0.10f));
				static char username[32] = "";
				ImGui::Text("Username:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##Username", NULL, username, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_None);

				static char password[32] = "";
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				ImGui::Text("Password:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##Password", NULL, password, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password);

				static char confirmpassword[32] = "";
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				ImGui::Text("Confirm password:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##ConfirmPassword", NULL, confirmpassword, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password);

				if (m_login_error != "")
				{
					ImVec2 text_size = ImGui::CalcTextSize(m_login_error.c_str());
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + text_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
					ImGui::Text(m_login_error.c_str());
				}

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("Register", ImVec2(container_size.x / 2, 20)))
				{
					if (strlen(username) < 4)
					{
						m_login_error = "Username is too short!";
						goto register_end;
					}
					if (strlen(password) < 8)
					{
						m_login_error = "Password is too short!";
						goto register_end;
					}
					if (strcmp(confirmpassword, password) != 0)
					{
						m_login_error = "Passwords do not match!";
						goto register_end;
					}
					string encrypted_password = encryption.Encrypt(password, username);
					if (driver.ExecuteUpdate("INSERT INTO uporabnik (ime, geslo) VALUES ('{}', '{}')", username, encrypted_password) > 0) // returns rows affected
					{
						m_register_prompt = false;
						m_login_error = "";
					}
					else
						m_login_error = "Database error, try again";
				}

register_end:
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("Back to login", ImVec2(container_size.x / 2, 20)))
					m_register_prompt = false;

				ImGui::EndChild();
			}
		}
		else
		{
			ImGui::SetCursorScreenPos(startPos);

			if (ImGui::BeginChild("login_child", container_size, ImGuiChildFlags_Border, ImGuiWindowFlags_NoCollapse & ImGuiWindowFlags_NoMove & ImGuiWindowFlags_NoScrollbar & ImGuiWindowFlags_NoTitleBar & ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + container_size.y * 0.10f));
				static char username[32] = "";
				ImGui::Text("Username:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##Username", NULL, username, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_None);

				static char password[32] = "";
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				ImGui::Text("Password:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##Password", NULL, password, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password);


				if (m_login_error != "")
				{
					ImVec2 text_size = ImGui::CalcTextSize(m_login_error.c_str());
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + text_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
					ImGui::Text(m_login_error.c_str());
				}

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("Log in", ImVec2(container_size.x / 2, 20)))
				{
					if (strlen(username) < 4)
					{
						m_login_error = "Username is too short!";
						goto login_end;
					}
					if (strlen(password) < 8)
					{
						m_login_error = "Password is too short!";
						goto login_end;
					}

					std::unique_ptr<sql::ResultSet> results(driver.ExecuteQuery("SELECT * FROM uporabnik WHERE ime = '{}'", username));
					if (results->rowsCount() > 0)
					{
						while (results->next())
						{
							string encrypted_password = encryption.Encrypt(password, username);
							if (results->getString("geslo") == encrypted_password)
							{
								m_logged_in = true;
								m_login_error = "";
							}
							else
								m_login_error = "Invalid Password provided for user (" + string(username) + ")";
						}
					}
					else
						m_login_error = "Invalid Username";
				}
login_end:
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("New account", ImVec2(container_size.x / 2, 20)))
					m_register_prompt = true;

				ImGui::EndChild();
			}
		}
	}

	std::unique_ptr<sql::ResultSet> results(driver.ExecuteQuery("SELECT * FROM oddelek"));
	if (results == nullptr)
		return;


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

template<typename... Args>
sql::ResultSet* Driver::ExecuteQuery(const string& query, Args&&... args)
{
	return ExecuteQuery(std::vformat(query, std::make_format_args(std::forward<Args>(args)...)));
}

int Driver::ExecuteUpdate(const string& query)
{
	try {
		sql::Statement* stmt;
		stmt = g_sqlConnection->createStatement();
		return stmt->executeUpdate(query.c_str());
	}
	catch (sql::SQLException& e)
	{
#ifdef _DEBUG
		cerr << "Error executing sql update (" << query << "): " << e.what() << endl;
#endif
		return e.getErrorCode();
	}
}

template<typename... Args>
int Driver::ExecuteUpdate(const string& query, Args&&... args)
{
	return ExecuteUpdate(std::vformat(query, std::make_format_args(std::forward<Args>(args)...)));
}

Driver driver;