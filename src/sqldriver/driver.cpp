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

static const char* item_getter(const std::vector<std::string>& items, int index) {
	if (index >= 0 && index < (int)items.size()) {
		return items[index].c_str();
	}
	return "N/A";
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
					if (ExecuteUpdate("INSERT INTO uporabnik (ime, geslo, role) VALUES ('{}', '{}', 'U')", username, encrypted_password) > 0) // returns rows affected
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
					std::unique_ptr<sql::ResultSet> results(ExecuteQuery("SELECT * FROM uporabnik WHERE ime = '{}'", username));
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
		static Pacient selected_pacient;
		static int selected_row = -1;
		static Zapisnik selected_zapisnik;


		if (ImGui::BeginTabItem("Pacienti"))
		{				
			selected_row = -1;
			static string search_term = "";
			ImGui::Spacing();
			ImGui::Text("Search:");
			ImGui::InputText("##SearchBar", &search_term, ImVec2(m_screen_size.x * 0.2f, 20), 32);
			
			m_filtered_pacienti.clear();
			if (!search_term.empty() || search_term != "")
			{
				for (auto& p : m_pacienti)
				{
					const auto it = search(p.m_priimek.begin(), p.m_priimek.end(), search_term.begin(), search_term.end(), 
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it != p.m_priimek.end())
						m_filtered_pacienti.push_back(p);
				}
			}
			else
				m_filtered_pacienti.insert(m_filtered_pacienti.end(), m_pacienti.begin(), m_pacienti.end());

			if (ImGui::BeginTable("##pacienti", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("ime");
				ImGui::TableSetupColumn("priimek");
				ImGui::TableSetupColumn("naslov");
				ImGui::TableSetupColumn("tel_st");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_filtered_pacienti.size(); ++row) // row
				{
					auto pacient = m_filtered_pacienti[row];
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 5; column++) // column
					{
						ImGui::TableSetColumnIndex(column);
						if (ImGui::Selectable(pacient.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
						{
							selected_row = row; 
							selected_pacient = pacient;
							selected_zapisnik = Zapisnik();
						}
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
										if (lhs.m_id == rhs.m_id)
											return false;
										bool sort = lhs.m_id > rhs.m_id ? true : false;
										return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
									}; break;
									case 1: {
										if (lhs.m_ime == rhs.m_ime)
											return false;
										bool sort = lhs.m_ime.compare(rhs.m_ime) <= 0 ? false : true;
										return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
									}; break;
									case 2: {
										if (lhs.m_priimek == rhs.m_priimek)
											return false;
										bool sort = lhs.m_priimek.compare(rhs.m_priimek) <= 0 ? false : true;
										return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
									}; break;
									case 3: {
										if (lhs.m_naslov == rhs.m_naslov)
											return false;
										bool sort = lhs.m_naslov.compare(rhs.m_naslov) <= 0 ? false : true;
										return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
									}; break;
									case 4: {
										if (lhs.m_tel_st == rhs.m_tel_st)
											return false;
										bool sort = lhs.m_tel_st.compare(rhs.m_tel_st) <= 0 ? false : true;
										return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
									}; break;

									default: {
										return false;
											}; break;
									}
								}
								return false;
							});
					}

					sortSpecs->SpecsDirty = false;
				}

				ImGui::EndTable();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Pacient", NULL, selected_row != -1 ? ImGuiTabItemFlags_SetSelected : 0))
		{
			static tm date = { };
			if (!date.tm_year)
				ImGui::SetDateToday(&date);
			static Doktor selected_doktor_termin;

			auto cursor_pos_start = ImGui::GetCursorPos();
			ImGui::Text("Name:");
			ImGui::InputText("##name_pacient", &selected_pacient.m_ime, ImVec2(m_screen_size.x * 0.25f, 0), 32);
			ImGui::Text("Last name:");
			ImGui::InputText("##lastname_pacient", &selected_pacient.m_priimek, ImVec2(m_screen_size.x * 0.25f, 0), 32);
			ImGui::Text("Address:");
			ImGui::InputText("##address_pacient", &selected_pacient.m_naslov, ImVec2(m_screen_size.x * 0.25f, 0), 96);
			ImGui::Text("Phone number:");
			ImGui::InputText("##number_pacient", &selected_pacient.m_tel_st, ImVec2(m_screen_size.x * 0.25f, 0), 32);
			if (ImGui::Button("Update##pacient"))
			{
				if (selected_row == -1)
					ExecuteUpdate("INSERT INTO pacient (ime, priimek, naslov, tel_st) VALUES ('{}', '{}', '{}', '{}');", selected_pacient.m_ime, selected_pacient.m_priimek, selected_pacient.m_naslov, selected_pacient.m_tel_st);
				else
					ExecuteUpdate("UPDATE pacient SET ime = '{}', priimek = '{}', naslov = '{}', tel_st = '{}' WHERE id = {};", selected_pacient.m_ime, selected_pacient.m_priimek, selected_pacient.m_naslov, selected_pacient.m_tel_st, selected_pacient.m_id);
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear##pacient"))
			{
				selected_row = -1;
				selected_zapisnik = Zapisnik();
				selected_pacient = Pacient();
			}
			if (selected_row != -1)
			{
				ImGui::Spacing(3);

				auto cursor_pos_end = ImGui::GetCursorPos();

				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, cursor_pos_start.y));
				ImGui::Text("Date:");
				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
				ImGui::DateChooser("##datum_termin", date);

				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				ImGui::Text("Ure:");
				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
				ImGui::SliderInt("##ure_termin", &date.tm_hour, 1, 24);

				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				ImGui::Text("Minute:");
				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
				ImGui::SliderInt("##minute_termin", &date.tm_min, 1, 60);

				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				ImGui::Text("Doktor: ");
				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
				if (ImGui::BeginCombo("##doktor_termin", selected_doktor_termin.get_ime_priimek().c_str()))
				{
					for (auto& doktor : m_doktroji)
					{
						if (ImGui::Selectable(doktor.get_ime_priimek().c_str()))
							selected_doktor_termin = doktor;
					}

					ImGui::EndCombo();
				}
				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				if (ImGui::Button("Add##termin"))
				{
					auto date_formated = date_to_sql(date);
					ExecuteUpdate("INSERT INTO termin (cas_datum, doktor_id, pacient_id) VALUES ('{}', {}, {});", date_formated, selected_doktor_termin.m_id, selected_pacient.m_id);
				}


				ImGui::SeparatorText("Termini");
				if (ImGui::BeginTable("##termini", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("ID");
					ImGui::TableSetupColumn("cas");
					ImGui::TableSetupColumn("doktor ime");
					ImGui::TableSetupColumn("doktor priimek");
					ImGui::TableHeadersRow();

					for (int row = 0; row < m_termini.size(); ++row) // row
					{
						auto termin = m_termini[row];
						if (termin.m_pacient_id != selected_pacient.m_id)
							continue;

						Doktor doktor;
						for (auto& d : m_doktroji)
							if (d.m_id == termin.m_doktor_id)
								doktor = d;

						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						for (int column = 0; column < 4; column++) // column
						{
							ImGui::TableSetColumnIndex(column);
							if (column == 0 || column == 1)
								ImGui::Text(termin.get(column).c_str());
							else if (column == 2 || column == 3)
								ImGui::Text(doktor.get(column - 1).c_str());
						}
					}
					if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
					{
						if (sortSpecs->SpecsDirty)
						{
							std::sort(
								m_termini.begin(), m_termini.end(),
								[&sortSpecs](const Termin& lhs, const Termin& rhs) -> bool {
									for (size_t i = 0; i < sortSpecs->SpecsCount; ++i)
									{
										const ImGuiTableColumnSortSpecs* currentSpecs = &sortSpecs->Specs[i];
										switch (currentSpecs->ColumnIndex)
										{
										case 0: {
											if (lhs.m_id == rhs.m_id)
												return false;
											bool sort = lhs.m_id > rhs.m_id ? true : false;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 1: {
											if (lhs.m_cas_datum == rhs.m_cas_datum)
												return false;
											bool sort = lhs.m_cas_datum.compare(rhs.m_cas_datum) <= 0 ? false : true;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 2: {
											if (lhs.m_doktor_id == rhs.m_doktor_id)
												return false;
											bool sort = lhs.m_doktor_id > rhs.m_doktor_id ? true : false;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 3: {
											if (lhs.m_pacient_id == rhs.m_pacient_id)
												return false;
											bool sort = lhs.m_pacient_id > rhs.m_pacient_id ? true : false;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;

										default: {
											return false;
										}; break;
										}
									}
									return false;
								});
						}

						sortSpecs->SpecsDirty = false;
					}
					ImGui::EndTable();
				}
			}

			static bool novi_zapisnik = false;
			ImGui::SeparatorText("Zapisnik");
			cursor_pos_start = ImGui::GetCursorPos();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			if (ImGui::BeginCombo("##zapisnik", novi_zapisnik ? "Novi" : selected_zapisnik.m_naslov.c_str()))
			{
				for (auto& z : m_zapisniki)
					if (z.m_pacient_id == selected_pacient.m_id)
						if (ImGui::Selectable(z.m_naslov.c_str()))
						{
							selected_zapisnik = z;
							novi_zapisnik = false;
						}

				if (ImGui::Selectable("Novi"))
				{
					selected_zapisnik = Zapisnik();
					novi_zapisnik = true;
				}
				ImGui::EndCombo();
			}
			ImGui::PushTextWrapPos(m_screen_size.x * 0.45f);
			ImGui::TextWrapped(selected_zapisnik.m_opis.c_str());
			ImGui::PopTextWrapPos();

			ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.5f, cursor_pos_start.y));
			ImGui::Text("Naslov:");
			ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
			ImGui::InputText("##zapisnik_naslov", &selected_zapisnik.m_naslov);

			ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
			ImGui::Text("Opis:");
			ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputTextMultiline("##zapisnik_opis", &selected_zapisnik.m_opis);

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

string Driver::date_to_sql(tm& date, bool include_time)
{
	char buffer[25];
	date.tm_sec = 0;
	if (include_time)
		std::strftime(buffer, sizeof(buffer), "%F %T", &date);
	else
		std::strftime(buffer, sizeof(buffer), "%F", &date);
	return string(buffer);
}

bool Driver::GetDatabaseVariables()
{
	m_pacienti.clear();
	m_termini.clear();
	m_doktroji.clear();
	m_zapisniki.clear();

	std::unique_ptr<sql::ResultSet> results_pacient(driver.ExecuteQuery("SELECT * FROM pacient"));
	if (results_pacient == nullptr)
		return false;
	while (results_pacient->next()) // row
		m_pacienti.push_back(Pacient(results_pacient->getInt(1), string(results_pacient->getString(2)), string(results_pacient->getString(3)), string(results_pacient->getString(4)), string(results_pacient->getString(5))));

	std::unique_ptr<sql::ResultSet> results_termin(driver.ExecuteQuery("SELECT * FROM termin"));
	if (results_termin == nullptr)
		return false;
	while (results_termin->next()) // row
		m_termini.push_back(Termin(results_termin->getInt(1), string(results_termin->getString(2)), results_termin->getInt(3), results_termin->getInt(4)));

	std::unique_ptr<sql::ResultSet> results_doktor(driver.ExecuteQuery("SELECT * FROM doktor"));
	if (results_doktor == nullptr)
		return false;
	while (results_doktor->next()) // row
		m_doktroji.push_back(Doktor(results_doktor->getInt(1), string(results_doktor->getString(2)), string(results_doktor->getString(3)), string(results_doktor->getString(4)), results_doktor->getInt(5)));

	std::unique_ptr<sql::ResultSet> results_zapisnik(driver.ExecuteQuery("SELECT * FROM zapisnik"));
	if (results_zapisnik == nullptr)
		return false;
	while (results_zapisnik->next()) // row
		m_zapisniki.push_back(Zapisnik(results_zapisnik->getInt(1), string(results_zapisnik->getString(2)), string(results_zapisnik->getString(3)), string(results_zapisnik->getString(4)), results_zapisnik->getInt(5)));

	return true;
}

Driver driver;