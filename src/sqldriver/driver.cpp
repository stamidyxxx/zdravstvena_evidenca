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

	m_logged_in = true;


	if (!m_logged_in)
	{
		ImVec2 container_size = m_screen_size / ImVec2(3, 3);
		ImVec2 center(m_screen_size.x * 0.5f, m_screen_size.y * 0.5f);
		ImVec2 startPos(center.x - container_size.x * 0.5f, center.y - container_size.y * 0.5f);

		if (m_register_prompt)
		{
			ImGui::SetCursorScreenPos(startPos);
			if (ImGui::BeginChild("##register_child", container_size, ImGuiChildFlags_Border, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse))
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
				bool skip_register_button = ImGui::InputTextEx("##ConfirmPassword", NULL, confirmpassword, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);

				if (m_login_error != "")
				{
					ImVec2 text_size = ImGui::CalcTextSize(m_login_error.c_str());
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + text_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
					ImGui::Text(m_login_error.c_str());
				}

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("Register", ImVec2(container_size.x / 2, 20)) || skip_register_button)
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
				if (ImGui::TextSelectable("Back to login"))
					m_register_prompt = false;

				ImGui::EndChild();
			}
		}
		else
		{
			ImGui::SetCursorScreenPos(startPos);

			if (ImGui::BeginChild("##login_child", container_size, ImGuiChildFlags_Border, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse))
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
				bool skip_login_button = ImGui::InputTextEx("##Password", NULL, password, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);


				if (m_login_error != "")
				{
					ImVec2 text_size = ImGui::CalcTextSize(m_login_error.c_str());
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + text_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
					ImGui::Text(m_login_error.c_str());
				}

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("Log in", ImVec2(container_size.x / 2, 20)) || skip_login_button)
				{
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
								m_login_error = "Invalid Password provided for " + string(username);
						}
					}
					else
						m_login_error = "Invalid Username";
				}
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::TextSelectable("New account"))
					m_register_prompt = true;

				ImGui::EndChild();
			}
		}
		return;
	}

	if (ImGui::BeginTabBar("##topnavbar"))
	{
		if (ImGui::BeginTabItem("Pacienti"))
		{
			ImGui::Spacing(10);
				if (ImGui::BeginTable("##pacienti", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("ID");
					ImGui::TableSetupColumn("ime");
					ImGui::TableSetupColumn("priimek");
					ImGui::TableSetupColumn("naslov");
					ImGui::TableSetupColumn("tel_st");
					ImGui::TableHeadersRow();
					for (auto& pacient : m_pacienti) // row
					{
						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						for (int column = 0; column < 5; column++) // column
						{
							ImGui::TableSetColumnIndex(column);
							ImGui::Text(pacient.get(column).c_str());
						}
					}

					if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
					{
						if (sortSpecs->SpecsDirty)
						{
							std::sort(
								m_pacienti.begin(), m_pacienti.end(),
								[&sortSpecs](const Pacient& lhs, const Pacient& rhs) -> bool {
									for (size_t i = 0; i < sortSpecs->SpecsCount; ++i)
									{
										const ImGuiTableColumnSortSpecs* currentSpecs = &sortSpecs->Specs[i];
										switch (currentSpecs->ColumnIndex)
										{
										case 0: {
											bool sort = lhs.m_id > rhs.m_id ? true : false;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 1: {
											bool sort = lhs.m_ime.compare(rhs.m_ime) <= 0 ? false : true;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 2: {
											bool sort = lhs.m_priimek.compare(rhs.m_priimek) <= 0 ? false : true;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 3: {
											bool sort = lhs.m_naslov.compare(rhs.m_naslov) <= 0 ? false : true;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 4: {
											bool sort = lhs.m_tel_st.compare(rhs.m_tel_st) <= 0 ? false : true;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;

										default: {
											return false;
											  }; break;
										}
									}
								});
						}

						sortSpecs->SpecsDirty = false;
					}

					ImGui::EndTable();
				}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
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