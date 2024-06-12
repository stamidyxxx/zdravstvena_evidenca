#include "driver.h"
#include "../imgui/imgui_stdlib.h"

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

static int input_filter_numbers_only(ImGuiInputTextCallbackData* data)
{
	auto c = data->EventChar;
	if (c >= '0' && c <= '9')
		return 0;
	return 1;
}

bool emso_verify(string emso)
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
				ImGui::Text("Uporabniško ime:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##Username", NULL, username, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_None);

				static char password[32] = "";
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				ImGui::Text("Geslo:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##Password", NULL, password, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password);

				static char confirmpassword[32] = "";
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				ImGui::Text("Geslo ponovno:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				bool skip_register_button = ImGui::InputTextEx("##ConfirmPassword", NULL, confirmpassword, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);

				if (m_login_error != "")
				{
					ImGui::InsertNotification({ ImGuiToastType_Error, 2000, m_login_error.c_str() });
					m_login_error = "";
				}

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("Register", ImVec2(container_size.x / 2, 20)) || skip_register_button)
				{
					if (strlen(username) < 4)
					{
						m_login_error = "Uporabniško ime je prekratko";
						goto register_end;
					}
					if (strlen(password) < 8)
					{
						m_login_error = "Geslo je prekratko";
						goto register_end;
					}
					if (strcmp(confirmpassword, password) != 0)
					{
						m_login_error = "Gesli se ne ujemata";
						goto register_end;
					}
					string encrypted_password = encryption.Encrypt(password, username);
					if (ExecuteUpdate("INSERT INTO uporabnik (ime, geslo, role) VALUES ('{}', '{}', 'U')", username, encrypted_password) > 0) // returns rows affected
					{
						m_register_prompt = false;
						m_login_error = "";
					}
					else
						m_login_error = "Napaka z bazo podatkov";
				}

register_end:
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::TextSelectable("Nazaj na prijavo"))
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
				ImGui::Text("Uporabnik:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				ImGui::InputTextEx("##Username", NULL, username, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_None);

				static char password[32] = "";
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				ImGui::Text("Geslo:");
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY()));
				bool skip_login_button = ImGui::InputTextEx("##Password", NULL, password, sizeof(char) * 32, ImVec2(container_size.x / 2, 20), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);


				if (m_login_error != "")
				{
					ImGui::InsertNotification({ ImGuiToastType_Error, 2000, m_login_error.c_str() });
					m_login_error = "";
				}

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::Button("Log in", ImVec2(container_size.x / 2, 20)) || skip_login_button)
				{
					std::unique_ptr<sql::ResultSet> results(ExecuteQuery("SELECT * FROM uporabnik WHERE ime = '{}'", username));
					if (results)
					{
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
									m_login_error = "Napačno geslo za uporabnika " + string(username);
							}

						}
						else
							m_login_error = "Napačno uporabniško ime";
					}
					else
						m_login_error = "Napaka v bazi podatkov, poskusite znova";
				}
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
				if (ImGui::TextSelectable("Nov uporabnik"))
					m_register_prompt = true;

				ImGui::EndChild();
			}
		}
		return;
	}

	GetDatabaseVariables();

	if (ImGui::BeginTabBar("##topnavbar"))
	{
		static Pacient selected_pacient, temp_selected_pacient;
		static Doktor selected_doktor, temp_selected_doktor;
		static Zapisnik selected_zapisnik;
		static Oddelek selected_oddelek, temp_selected_oddelek;
		static Zdravilo selected_zdravilo, temp_selected_zdravilo;
		static Recept selected_recept, temp_selected_recept;
		static Termin selected_termin, temp_selected_termin;
		static Bolnica selected_bolnica, temp_selected_bolnica;
		bool can_switch = false;
		static bool open_popup_oddelki = false, open_popup_zdravila = false, open_popup_pacienti = false, open_popup_doktor = false, open_popup_termin = false, open_popup_recept, open_popup_bolnice = false;
		static bool selected_tab_pacient = false, selected_tab_doktor = false, selected_tab_zdravilo = false, selected_tab_oddelek = false;
		static bool update_date = false;

		// ------ PACIENTI ------ \\

		static bool open_pacienti = false;
		if (open_popup_pacienti)
		{
			open_popup_pacienti = false;
			open_pacienti = true;
			ImGui::OpenPopup("Pacient:##pacient");
		}

		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Pacient:##pacient", &open_pacienti, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrappedCentered(temp_selected_pacient.get_ime_priimek().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##pacient", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##pacient", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					selected_tab_pacient = true;
					selected_tab_doktor = false;
					selected_tab_oddelek = false;
					selected_tab_zdravilo = false;

					can_switch = true;
					selected_pacient = temp_selected_pacient;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##pacient", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					if (ExecuteUpdate("DELETE FROM pacient WHERE id = {};", temp_selected_pacient.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Pacient zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Pacient ni zbrisan" });

					selected_pacient = Pacient();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}
		if (ImGui::BeginTabItem("Pacienti"))
		{				
			static string search_term = "";
			ImGui::Spacing();
			ImGui::Text("Iskanje:");
			ImGui::InputText("##SearchBar", &search_term, ImVec2(m_screen_size.x * 0.2f, 20), 32);
			
			m_filtered_pacienti.clear();
			if (!search_term.empty() || search_term != "")
			{
				for (auto& p : m_pacienti)
				{
					const auto it_priimek = search(p.m_priimek.begin(), p.m_priimek.end(), search_term.begin(), search_term.end(), 
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_priimek != p.m_priimek.end())
					{
						m_filtered_pacienti.push_back(p);
						continue;
					}

					const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_ime != p.m_ime.end())
					{
						m_filtered_pacienti.push_back(p);
						continue;
					}
				}
			}
			else
				m_filtered_pacienti.insert(m_filtered_pacienti.end(), m_pacienti.begin(), m_pacienti.end());

			if (ImGui::BeginTable("##pacienti", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Ime");
				ImGui::TableSetupColumn("Priimek");
				ImGui::TableSetupColumn("Naslov");
				ImGui::TableSetupColumn("Tel. Številka");
				ImGui::TableSetupColumn("EMŠO");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_filtered_pacienti.size(); ++row) // row
				{
					auto pacient = m_filtered_pacienti[row];
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 6; column++) // column
					{
						ImGui::TableSetColumnIndex(column);
						if (ImGui::Selectable(pacient.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
						{
							temp_selected_pacient = pacient;
							open_popup_pacienti = true;
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
									case 5: {
										if (lhs.m_emso == rhs.m_emso)
											return false;
										bool sort = lhs.m_emso.compare(rhs.m_emso) <= 0 ? false : true;
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

		// ------ PACIENT ------ \\

		static bool open_termin = false;
		if (open_popup_termin)
		{
			open_popup_termin = false;
			open_termin = true;
			ImGui::OpenPopup("Termin:##termin");
		}

		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Termin:##termin", &open_termin, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrappedCentered(temp_selected_termin.get_text().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##termin", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##termin", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					selected_tab_pacient = false;
					selected_tab_doktor = false;
					selected_tab_oddelek = false;
					selected_tab_zdravilo = false;

					can_switch = true;
					update_date = true;
					selected_termin = temp_selected_termin;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##termin", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					if (ExecuteUpdate("DELETE FROM termin WHERE id = {};", temp_selected_termin.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Termin zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Termin ni zbrisan" });

					selected_termin = temp_selected_termin = Termin();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}

		static bool open_recept = false;
		if (open_popup_recept)
		{
			open_popup_recept = false;
			open_recept = true;
			ImGui::OpenPopup("Recept:##recept");
		}

		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Recept:##recept", &open_recept, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrappedCentered(temp_selected_recept.get_text().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##recept", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##recept", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					selected_tab_pacient = false;
					selected_tab_doktor = false;
					selected_tab_oddelek = false;
					selected_tab_zdravilo = false;

					can_switch = true;
					selected_recept = temp_selected_recept;
					for (auto& rhz : m_recepti_has_zdravila)
						if (rhz.m_recept->m_id == selected_recept.m_id)
							selected_recept.m_selected_zdravila.emplace(make_pair(rhz.m_zdravilo->m_id, rhz.m_zdravilo->m_ime));

					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##recept", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					if (ExecuteUpdate("DELETE FROM recept WHERE id = {};", temp_selected_termin.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Recept zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Recept ni zbrisan" });

					selected_recept = temp_selected_recept = Recept();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}
		if (ImGui::BeginTabItem("Pacient", NULL, selected_tab_pacient && can_switch ? ImGuiTabItemFlags_SetSelected : 0))
		{
			static tm date = { };
			static Doktor selected_doktor_termin;

			auto cursor_pos_start = ImGui::GetCursorPos();
			ImGui::Text("Ime:");
			ImGui::InputText("##name_pacient", &selected_pacient.m_ime, ImVec2(m_screen_size.x * 0.25f, 0), 32);
			ImGui::Text("Priimek:");
			ImGui::InputText("##lastname_pacient", &selected_pacient.m_priimek, ImVec2(m_screen_size.x * 0.25f, 0), 32);
			ImGui::Text("Naslov:");
			ImGui::InputText("##address_pacient", &selected_pacient.m_naslov, ImVec2(m_screen_size.x * 0.25f, 0), 96);
			ImGui::Text("Tel. Številka:");
			ImGui::InputText("##number_pacient", (char*)selected_pacient.m_tel_st.c_str(), ImVec2(m_screen_size.x * 0.25f, 0), 9, ImGuiInputTextFlags_CallbackCharFilter, input_filter_numbers_only);
			ImGui::Text("EMŠO:");
			ImGui::InputText("##emso_pacient", (char*)selected_pacient.m_emso.c_str(), ImVec2(m_screen_size.x * 0.25f, 0), 13, ImGuiInputTextFlags_CallbackCharFilter, input_filter_numbers_only);
			if (selected_pacient.m_id == -1)
			{
				if (ImGui::Button("Dodaj##pacient"))
					if (emso_verify(selected_pacient.m_emso))
						if (ExecuteUpdate("INSERT INTO pacient (ime, priimek, naslov, tel_st, emso) VALUES ('{}', '{}', '{}', '{}', '{}');", selected_pacient.m_ime, selected_pacient.m_priimek, selected_pacient.m_naslov, selected_pacient.m_tel_st, selected_pacient.m_emso) > 0)
							ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Pacient dodan!" });
						else
							ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Pacient ni dodan - napaka v bazi" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "EMŠO ni veljaven!" });
			}
			else
			{
				if (ImGui::Button("Posodobi##pacient"))
					if (emso_verify(selected_pacient.m_emso))
						if (ExecuteUpdate("UPDATE pacient SET ime = '{}', priimek = '{}', naslov = '{}', tel_st = '{}', emso = '{}' WHERE id = {};", selected_pacient.m_ime, selected_pacient.m_priimek, selected_pacient.m_naslov, selected_pacient.m_tel_st, selected_pacient.m_emso, selected_pacient.m_id) > 0)
							ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Pacient posodobljen!" });
						else
							ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Pacient ni posodobljen - napaka v bazi" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "EMŠO ni veljaven!" });
			}

			ImGui::SameLine();
			if (ImGui::Button("Počisti##pacient"))
			{
				selected_zapisnik = Zapisnik();
				selected_pacient = Pacient();
			}
			if (selected_pacient.m_id != -1)
			{
				ImGui::SeparatorText("Termini");
				if (ImGui::BeginTable("##termini", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("ID");
					ImGui::TableSetupColumn("Čas");
					ImGui::TableSetupColumn("Ime doktorja");
					ImGui::TableSetupColumn("Priimek doktorja");
					ImGui::TableHeadersRow();

					for (int row = 0; row < m_termini.size(); ++row) // row
					{
						auto termin = m_termini[row];
						if (termin.m_pacient->m_id != selected_pacient.m_id)
							continue;

						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						for (int column = 0; column < 4; column++) // column
						{
							ImGui::TableSetColumnIndex(column);
							if (column == 0 || column == 1)
							{
								if (ImGui::Selectable(termin.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
								{
									temp_selected_termin = termin;
									open_popup_termin = true;
								}
							}
							else if (column == 2 || column == 3)
							{
								if (ImGui::Selectable(termin.m_doktor->get(column - 1).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
								{
									temp_selected_termin = termin;
									open_popup_termin = true;
								}
							}
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
											if (lhs.m_doktor->m_ime == rhs.m_doktor->m_ime)
												return false;
											bool sort = lhs.m_doktor->m_ime.compare(rhs.m_doktor->m_ime) <= 0 ? false : true;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 3: {
											if (lhs.m_doktor->m_priimek == rhs.m_doktor->m_priimek)
												return false;
											bool sort = lhs.m_doktor->m_priimek.compare(rhs.m_doktor->m_priimek) <= 0 ? false : true;
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

					if (update_date)
					{
						date = sql_to_date(selected_termin.m_cas_datum);
						update_date = false;
					}

					ImGui::Text("Datum:");
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
					ImGui::DateChooser("##datum_termin", date);

					ImGui::Text("Ure:");
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
					ImGui::SliderInt("##ure_termin", &date.tm_hour, 1, 24);

					ImGui::Text("Minute:");
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
					ImGui::SliderInt("##minute_termin", &date.tm_min, 0, 60);

					if (m_doktorji_imena.empty())
					{
						m_doktorji_imena.reserve(m_doktroji.size());
						std::transform(m_doktroji.begin(), m_doktroji.end(), std::back_inserter(m_doktorji_imena),
							[](const Doktor& o) { return o.m_ime + " " + o.m_priimek; });
					}
					static int selected_doktor_termin_int;
					ImGui::Text("Doktor: ");
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
					if (ImGui::ComboWithFilter("##doktor_termin", &selected_doktor_termin_int, m_doktorji_imena))
					{
						selected_termin.m_doktor = &m_doktroji[selected_doktor_termin_int];
					}
					if (selected_termin.m_id != -1)
					{
						if (ImGui::Button("Posodobi##termin"))
						{
							auto date_formated = date_to_sql(date);
							if (ExecuteUpdate("UPDATE termin SET cas_datum = '{}', doktor_id = {}, pacient_id = {} WHERE id = {};", date_formated, selected_termin.m_doktor->m_id, selected_pacient.m_id, selected_termin.m_id) > 0)
								ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Termin posodobljen!" });
							else
								ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Termin ni posodobljen - napaka v bazi" });
						}
					}
					else
					{
						if (ImGui::Button("Dodaj##termin"))
						{
							auto date_formated = date_to_sql(date);
							if (ExecuteUpdate("INSERT INTO termin (cas_datum, doktor_id, pacient_id) VALUES ('{}', {}, {});", date_formated, selected_termin.m_doktor->m_id, selected_pacient.m_id) > 0)
								ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Termin dodan!" });
							else
								ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Termin ni dodan - napaka v bazi" });
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Počisti##termin"))
					{
						selected_termin = Termin();
						date = ImGui::GetCurrentDate();
					}

					ImGui::SeparatorText("Recepti");
					if (ImGui::BeginTable("##recepti", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
					{
						ImGui::TableSetupColumn("ID");
						ImGui::TableSetupColumn("Čas");
						ImGui::TableSetupColumn("Zdravila");
						ImGui::TableHeadersRow();

						for (int row = 0; row < m_recepti.size(); ++row) // row
						{
							auto recept = m_recepti[row];
							if (recept.m_pacient->m_id != selected_pacient.m_id)
								continue;

							ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
							for (int column = 0; column < 3; column++) // column
							{
								ImGui::TableSetColumnIndex(column);
								if (column == 0 || column == 1)
								{
									if (ImGui::Selectable(recept.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
									{
										temp_selected_recept = recept;
										open_popup_recept = true;
									}
								}
								else
								{
									if (recept.m_zdravila == "")
									{
										for (auto& rhz : m_recepti_has_zdravila)
											if (rhz.m_recept->m_id == recept.m_id)
												for (auto& z : m_zdravila)
													if (rhz.m_zdravilo->m_id == z.m_id)
														recept.m_zdravila.append(z.m_ime + ", ");
										if (!recept.m_zdravila.empty())
											recept.m_zdravila.resize(recept.m_zdravila.size() - 2);
									}
									if (recept.m_zdravila != "")
									{
										if (ImGui::Selectable(recept.m_zdravila.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0), true))
										{
											temp_selected_recept = recept;
											open_popup_recept = true;
										}
									}
								}
							}
						}
						if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
						{
							if (sortSpecs->SpecsDirty)
							{
								std::sort(
									m_recepti.begin(), m_recepti.end(),
									[&sortSpecs](const Recept& lhs, const Recept& rhs) -> bool {
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
												if (lhs.m_zdravila == rhs.m_zdravila)
													return false;
												bool sort = lhs.m_zdravila.compare(rhs.m_zdravila) <= 0 ? false : true;
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

					if (selected_recept.m_zdravila.empty() && selected_recept.m_id != -1)
					{
						for (auto& z : m_zdravila)
						{
							if (selected_recept.m_selected_zdravila.find(z.m_id) != selected_recept.m_selected_zdravila.end())
								selected_recept.m_zdravila.append(z.m_ime + ", ");
						}
						if (selected_recept.m_zdravila.size() > 3 && !selected_recept.m_zdravila.empty())
							selected_recept.m_zdravila.resize(selected_recept.m_zdravila.size() - 2);
					}

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
					if (ImGui::BeginCombo("##Zdravila",selected_recept.m_zdravila.c_str()))
					{
						selected_recept.m_zdravila.clear();
						for (auto& z : m_zdravila)
						{
							bool is_selected = selected_recept.m_selected_zdravila.find(z.m_id) != selected_recept.m_selected_zdravila.end();
							if (ImGui::Selectable(z.m_ime.c_str(), is_selected, ImGuiSelectableFlags_DontClosePopups))
							{
								if (is_selected)
									selected_recept.m_selected_zdravila.erase(selected_recept.m_selected_zdravila.find(z.m_id));
								else 
									selected_recept.m_selected_zdravila.emplace(make_pair(z.m_id, z.m_ime));
							}

							if (is_selected)
								selected_recept.m_zdravila.append(z.m_ime + ", ");
						}
						if (selected_recept.m_zdravila.size() > 3 && !selected_recept.m_zdravila.empty())
							selected_recept.m_zdravila.resize(selected_recept.m_zdravila.size() - 2);

						ImGui::EndCombo();
					}

					
				//	if (novi_zapisnik)
				//	{
					if (selected_recept.m_id != -1)
					{
						if (ImGui::Button("Posodobi##recept"))
						{
							if (ExecuteUpdate("DELETE FROM recepti_has_zdravila WHERE recept_id = {}", selected_recept.m_id))
							{
								for (auto& z : selected_recept.m_selected_zdravila)
								{
									if (ExecuteUpdate("INSERT INTO recepti_has_zdravila (zdravilo_id, recept_id) VALUES ({}, {})", z.first, selected_recept.m_id))
										ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Recept posodobljen!" });
									else
										ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Recept ni posodobljen - napaka v bazi" });
								}
							}
							else
								ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Recept ni posodobljen - napaka v bazi" });
						}
					}
					else
					{
						if (ImGui::Button("Dodaj##recept"))
						{
							if (selected_recept.m_selected_zdravila.size() > 0)
							{
								bool success = false;
								auto datum = date_to_sql(ImGui::GetCurrentDate());
								if (ExecuteUpdate("INSERT INTO recept (cas_datum, pacient_id) VALUES ('{}', {});", datum, selected_pacient.m_id) > 0)
								{
									auto results = ExecuteQuery("SELECT LAST_INSERT_ID();");
									while (results->next())
									{
										auto id = results->getInt(1);
										for (auto& zdravilo : selected_recept.m_selected_zdravila)
											if (ExecuteUpdate("INSERT INTO recepti_has_zdravila (zdravilo_id, recept_id) VALUES ({}, {});", zdravilo.first, id) <= 0)
											{
												success = false;
												ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Recept ni dodan - napaka v bazi" });
											}
											else
												success = true;
									}
									if (success)
										ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Recept dodan!" });
								}
								else
									ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Recept ni dodan - napaka v bazi" });
							}
							else
								ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Izberite vsaj 1 zdravilo za recept." });
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Počisti##recept"))
						selected_recept = Recept();
				/*	}
					else
					{
						ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
						if (ImGui::Button("Posodobi##zapisnik"))
							if (ExecuteUpdate("UPDATE zapisnik SET naslov = '{}', opis = '{}' WHERE id = {};", selected_zapisnik.m_naslov, selected_zapisnik.m_opis, selected_zapisnik.m_id) > 0)
								ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zapisnik posodobljen!" });
							else
								ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zapisnik ni posodobljen - napaka v bazi" });
					}
				*/
					
				}
				static bool novi_zapisnik = false;
				ImGui::SeparatorText("Zapisnik");
				cursor_pos_start = ImGui::GetCursorPos();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
				if (ImGui::BeginCombo("##zapisnik", novi_zapisnik ? "Novi" : selected_zapisnik.m_naslov.c_str()))
				{
					for (auto& z : m_zapisniki)
						if (z.m_pacient->m_id == selected_pacient.m_id)
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

				if (novi_zapisnik)
				{
					ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
					if (ImGui::Button("Dodaj##zapisnik"))
					{
						auto datum = date_to_sql(ImGui::GetCurrentDate());
						if (ExecuteUpdate("INSERT INTO zapisnik (naslov, opis, datum, pacient_id) VALUES ('{}', '{}', '{}', {});", selected_zapisnik.m_naslov, selected_zapisnik.m_opis, datum, selected_pacient.m_id) > 0)
							ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zapisnik dodan!" });
						else
							ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zapisnik ni dodan - napaka v bazi" });
					}
				}
				else
				{
					ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
					if (ImGui::Button("Posodobi##zapisnik"))
						if (ExecuteUpdate("UPDATE zapisnik SET naslov = '{}', opis = '{}' WHERE id = {};", selected_zapisnik.m_naslov, selected_zapisnik.m_opis, selected_zapisnik.m_id) > 0)
							ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zapisnik posodobljen!" });
						else
							ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zapisnik ni posodobljen - napaka v bazi" });
				}
			}
			ImGui::EndTabItem();
		}

		// ------ ZDRAVNIKI ------ \\

		static bool open_doktor = false;
		if (open_popup_doktor)
		{
			open_popup_doktor = false;
			open_doktor = true;
			ImGui::OpenPopup("Zdravnik:##zdravnik");
		}

		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Zdravnik:##zdravnik", &open_doktor, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrappedCentered(temp_selected_doktor.get_ime_priimek().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##zdravnik", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##zdravnik", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					selected_tab_pacient = false;
					selected_tab_doktor = true;
					selected_tab_oddelek = false;
					selected_tab_zdravilo = false;

					can_switch = true;
					selected_doktor = temp_selected_doktor;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##zdravnik", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					if (ExecuteUpdate("DELETE FROM doktor WHERE id = {};", temp_selected_doktor.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Zdravnik zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Zdravnik ni zbrisan" });

					selected_doktor = temp_selected_doktor = Doktor();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}

		if (ImGui::BeginTabItem("Zdravniki"))
		{
			static string search_term = "";
			ImGui::Spacing();
			ImGui::Text("Iskanje:");
			ImGui::InputText("##SearchBardoktorji", &search_term, ImVec2(m_screen_size.x * 0.2f, 20), 32);

			m_filtered_doktroji.clear();
			if (!search_term.empty() && search_term != "")
			{
				for (auto& p : m_doktroji)
				{
					const auto it_priimek = search(p.m_priimek.begin(), p.m_priimek.end(), search_term.begin(), search_term.end(),
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_priimek != p.m_priimek.end())
					{
						m_filtered_doktroji.push_back(p);
						continue;
					}

					const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_ime != p.m_ime.end())
					{
						m_filtered_doktroji.push_back(p);
						continue;
					}
				}
			}
			else
				m_filtered_doktroji.insert(m_filtered_doktroji.end(), m_doktroji.begin(), m_doktroji.end());

			if (ImGui::BeginTable("##doktorji", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Ime");
				ImGui::TableSetupColumn("Priimek");
				ImGui::TableSetupColumn("Oddelek");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_filtered_doktroji.size(); ++row) // row
				{
					string oddelek_name = "";
					auto doktor = m_filtered_doktroji[row];
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 4; column++) // column
					{
						ImGui::TableSetColumnIndex(column);
						auto text = column == 3 ? doktor.m_oddelek->m_ime : doktor.get(column);
						if (ImGui::Selectable(text.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
						{
							temp_selected_doktor = doktor;
							open_popup_doktor = true;
						}
					}
				}

				if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
				{
					if (sortSpecs->SpecsDirty)
					{
						std::sort(
							m_doktroji.begin(), m_doktroji.end(),
							[&sortSpecs](const Doktor& lhs, const Doktor& rhs) -> bool {
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
										if (lhs.m_oddelek->m_ime == rhs.m_oddelek->m_ime)
											return false;
										bool sort = lhs.m_oddelek->m_ime.compare(rhs.m_oddelek->m_ime) <= 0 ? false : true;
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

		// ------ ZDRAVNIK ------ \\

		if (ImGui::BeginTabItem("Zdravnik", NULL, selected_tab_doktor && can_switch ? ImGuiTabItemFlags_SetSelected : 0))
		{
			auto cursor_pos_start = ImGui::GetCursorPos();
			ImGui::Text("Ime:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::InputText("##name_doktor", &selected_doktor.m_ime, ImVec2(m_screen_size.x * 0.25f, 0), 32);
			
			ImGui::Text("Priimek:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::InputText("##lastname_doktor", &selected_doktor.m_priimek, ImVec2(m_screen_size.x * 0.25f, 0), 32);
			
			ImGui::Text("Tel. Številka:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::InputText("##telst_doktor", &selected_doktor.m_tel_st, ImVec2(m_screen_size.x * 0.25f, 0), 9, ImGuiInputTextFlags_CallbackCharFilter, input_filter_numbers_only);


			if (m_oddelki_imena.empty()) 
			{
				m_oddelki_imena.reserve(m_oddelki.size());
				std::transform(m_oddelki.begin(), m_oddelki.end(), std::back_inserter(m_oddelki_imena),
					[](const Oddelek& o) { return o.m_ime; });
			}
			static int selected_oddelek_int;
			ImGui::Text("Oddelek:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			if (ImGui::ComboWithFilter("##doktor", &selected_oddelek_int, m_oddelki_imena))
			{
				selected_doktor.m_oddelek = &m_oddelki[selected_oddelek_int];
			}

			if (selected_doktor.m_id == -1)
			{
				if (ImGui::Button("Dodaj##doktor"))
					if (ExecuteUpdate("INSERT INTO doktor (ime, priimek, tel_st, oddelek_id) VALUES ('{}', '{}', '{}', {});", selected_doktor.m_ime, selected_doktor.m_priimek, selected_doktor.m_tel_st, selected_doktor.m_oddelek->m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Doktor dodan!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Doktor ni dodan - napaka v bazi" });
			}
			else
			{
				if (ImGui::Button("Posodobi##doktor"))
					if (ExecuteUpdate("UPDATE doktor SET ime = '{}', priimek = '{}', tel_st = '{}', oddelek_id = {} WHERE id = {};", selected_doktor.m_ime, selected_doktor.m_priimek, selected_doktor.m_tel_st, selected_doktor.m_oddelek->m_id, selected_doktor.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Doktor posodobljen!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Doktor ni posodobljen - napaka v bazi" });
			}

			ImGui::SameLine();
			if (ImGui::Button("Počisti##doktor"))
			{
				selected_doktor = Doktor();
			}
			if (selected_doktor.m_id != -1)
			{
				ImGui::SeparatorText("Termini");
				if (ImGui::BeginTable("##termini", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("ID");
					ImGui::TableSetupColumn("Čas");
					ImGui::TableSetupColumn("Ime pacienta");
					ImGui::TableSetupColumn("Priimek pacienta");
					ImGui::TableHeadersRow();

					for (int row = 0; row < m_termini.size(); ++row) // row
					{
						auto termin = m_termini[row];
						if (termin.m_doktor->m_id != selected_doktor.m_id)
							continue;

						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						for (int column = 0; column < 4; column++) // column
						{
							ImGui::TableSetColumnIndex(column);
							if (column == 0 || column == 1)
								ImGui::Text(termin.get(column).c_str());
							else if (column == 2 || column == 3)
								ImGui::Text(termin.m_pacient->get(column - 1).c_str());
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
											if (lhs.m_pacient->m_ime == rhs.m_pacient->m_ime)
												return false;
											bool sort = lhs.m_pacient->m_ime.compare(rhs.m_pacient->m_ime) <= 0 ? false : true;
											return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
										}; break;
										case 3: {
											if (lhs.m_pacient->m_priimek == rhs.m_pacient->m_priimek)
												return false;
											bool sort = lhs.m_pacient->m_priimek.compare(rhs.m_pacient->m_priimek) <= 0 ? false : true;
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

			ImGui::EndTabItem();
		}

		// ------ ODDELKI ------ \\

		static bool open_oddelki = false;
		if (open_popup_oddelki)
		{
			open_popup_oddelki = false;
			open_oddelki = true;
			ImGui::OpenPopup("Oddelek:##oddelki");
		}
		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Oddelek:##oddelki", &open_oddelki, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrappedCentered(temp_selected_oddelek.m_ime.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##oddelki", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##oddelki", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					selected_tab_pacient = false;
					selected_tab_doktor = false;
					selected_tab_oddelek = true;
					selected_tab_zdravilo = false;

					can_switch = true;
					selected_oddelek = temp_selected_oddelek;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##oddelki", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					if (ExecuteUpdate("DELETE FROM oddelek WHERE id = {};", temp_selected_oddelek.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Oddelek zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Oddelek ni zbrisan" });

					selected_oddelek = temp_selected_oddelek = Oddelek();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}
		if (ImGui::BeginTabItem("Oddelki"))
		{
			static string search_term = "";
			ImGui::Spacing();
			ImGui::Text("Iskanje:");
			ImGui::InputText("##SearchBaroddelek", &search_term, ImVec2(m_screen_size.x * 0.2f, 20), 32);

			m_filtered_oddeleki.clear();
			if (!search_term.empty() && search_term != "")
			{
				for (auto& p : m_oddelki)
				{
					const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_ime != p.m_ime.end())
					{
						m_filtered_oddeleki.push_back(p);
						continue;
					}

					const auto it_opis = search(p.m_bolnica->m_ime.begin(), p.m_bolnica->m_ime.end(), search_term.begin(), search_term.end(),
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_opis != p.m_bolnica->m_ime.end())
					{
						m_filtered_oddeleki.push_back(p);
						continue;
					}
				}
			}
			else
				m_filtered_oddeleki.insert(m_filtered_oddeleki.end(), m_oddelki.begin(), m_oddelki.end());

			if (ImGui::BeginTable("##oddelki", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Ime");
				ImGui::TableSetupColumn("Bolnica");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_filtered_oddeleki.size(); ++row) // row
				{
					auto oddelek = m_filtered_oddeleki[row];
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 3; column++) // column
					{
						ImGui::TableSetColumnIndex(column);
						if (column == 0 || column == 1)
						{
							if (ImGui::Selectable(oddelek.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
							{
								temp_selected_oddelek = oddelek;
								open_popup_oddelki = true;
							}
						}
						else if (column == 2)
						{
							if (oddelek.m_bolnica)
							{
								if (ImGui::Selectable(oddelek.m_bolnica->m_ime.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
								{
									temp_selected_oddelek = oddelek;
									open_popup_oddelki = true;
								}
							}
							else
								ImGui::Text("/");
						}
					}
				}
				if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
				{
					if (sortSpecs->SpecsDirty)
					{
						std::sort(
							m_filtered_oddeleki.begin(), m_filtered_oddeleki.end(),
							[&sortSpecs](const Oddelek& lhs, const Oddelek& rhs) -> bool {
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
										if (lhs.m_bolnica->m_ime == rhs.m_bolnica->m_ime)
											return false;
										bool sort = lhs.m_bolnica->m_ime.compare(rhs.m_bolnica->m_ime) <= 0 ? false : true;
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
			ImGui::SeparatorText("Odelek:");

			ImGui::Text("Ime:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::InputText("##name_oddelek", &selected_oddelek.m_ime, ImVec2(m_screen_size.x * 0.25f, 0), 32);

			if (m_bolnice_imena.empty())
			{
				m_bolnice_imena.reserve(m_bolnice.size());
				std::transform(m_bolnice.begin(), m_bolnice.end(), std::back_inserter(m_bolnice_imena),
					[](const Bolnica& o) { return o.m_ime; });
			}
			static int selected_bolnica_oddelek_int;
			ImGui::Text("Bolnica: ");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			if (ImGui::ComboWithFilter("##oddelek_bolnica", &selected_bolnica_oddelek_int, m_bolnice_imena))
			{
				selected_oddelek.m_bolnica = &m_bolnice[selected_bolnica_oddelek_int];
			}
			if (selected_oddelek.m_id == -1)
			{
				if (ImGui::Button("Dodaj##oddelek"))
				{
					if (ExecuteUpdate("INSERT INTO oddelek (ime, bolnica_id) VALUES ('{}', {});", selected_oddelek.m_ime, selected_oddelek.m_bolnica->m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Oddelek dodan!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Oddelek ni dodan - napaka v bazi" });
				}
			}
			else
			{
				if (ImGui::Button("Posodobi##oddelek"))
				{
					if (ExecuteUpdate("UPDATE oddelek SET ime = '{}', bolnica_id = {} WHERE id = {};", selected_oddelek.m_ime, selected_oddelek.m_bolnica->m_id, selected_oddelek.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Oddelek posodobljen!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Oddelek ni posodobljen - napaka v bazi" });
				}
			}			
			ImGui::SameLine();
			if (ImGui::Button("Počisti##oddelek"))
				selected_oddelek = Oddelek();

			ImGui::EndTabItem();
		}

		// ------ ZDRAVILA ------ \\

		static bool open_zdravila = false;
		bool scroll_to_bottom = false;
		if (open_popup_zdravila)
		{
			open_popup_zdravila = false;
			open_zdravila = true;
			ImGui::OpenPopup("Zdravila:##zdravila");
		}

		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Zdravila:##zdravila", &open_zdravila, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrappedCentered(temp_selected_zdravilo.m_ime.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##zdravila", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##zdravila", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					selected_tab_pacient = false;
					selected_tab_doktor = false;
					selected_tab_oddelek = false;
					selected_tab_zdravilo = true;

					can_switch = true;
					selected_zdravilo = temp_selected_zdravilo;
					scroll_to_bottom = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##zdravila", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					if (ExecuteUpdate("DELETE FROM zdravilo WHERE id = {};", temp_selected_zdravilo.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Zdravilo zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Zdravilo ni zbrisan" });

					selected_zdravilo = temp_selected_zdravilo = Zdravilo();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}
		if (ImGui::BeginTabItem("Zdravila"))
		{
			static string search_term = "";
			ImGui::Spacing();
			ImGui::Text("Iskanje:");
			ImGui::InputText("##SearchBarzdravila", &search_term, ImVec2(m_screen_size.x * 0.2f, 20), 32);

			m_filtered_zdravila.clear();
			if (!search_term.empty() && search_term != "")
			{
				for (auto& p : m_zdravila)
				{
					const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_ime != p.m_ime.end())
					{
						m_filtered_zdravila.push_back(p);
						continue;
					}

					const auto it_opis = search(p.m_opis.begin(), p.m_opis.end(), search_term.begin(), search_term.end(),
						[](char a, char b) {
							return tolower(a) == tolower(b);
						});
					if (it_opis != p.m_opis.end())
					{
						m_filtered_zdravila.push_back(p);
						continue;
					}
				}
			}
			else
				m_filtered_zdravila.insert(m_filtered_zdravila.end(), m_zdravila.begin(), m_zdravila.end());

			if (ImGui::BeginTable("##zdravila", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Ime");
				ImGui::TableSetupColumn("Količina");
				ImGui::TableSetupColumn("Opis");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_filtered_zdravila.size(); ++row) // row
				{
					auto zdravilo = m_filtered_zdravila[row];
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 4; column++) // column
					{
						ImGui::TableSetColumnIndex(column);

						if (ImGui::Selectable(zdravilo.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0), true))
						{
							temp_selected_zdravilo = zdravilo;
							open_popup_zdravila = true;
						}
					}
				}
				if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
				{
					if (sortSpecs->SpecsDirty)
					{
						std::sort(
							m_filtered_zdravila.begin(), m_filtered_zdravila.end(),
							[&sortSpecs](const Zdravilo& lhs, const Zdravilo& rhs) -> bool {
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
										if (lhs.m_kolicina == rhs.m_kolicina)
											return false;
										bool sort = lhs.m_kolicina > rhs.m_kolicina ? true : false;
										return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
									}; break;
									case 3: {
										if (lhs.m_opis == rhs.m_opis)
											return false;
										bool sort = lhs.m_opis.compare(rhs.m_opis) <= 0 ? false : true;
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
			
			ImGui::SeparatorText("Zdravilo:");

			ImGui::Text("Ime:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::InputText("##ime_zdravilo", &selected_zdravilo.m_ime, ImVec2(m_screen_size.x * 0.25f, 0), 32);

			ImGui::Text("Količina:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::SliderInt("##kolicina_zdravilo", &selected_zdravilo.m_kolicina, 1, 1000, "%dg");

			ImGui::Text("Opis:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::InputTextMultiline("##opis_zdravilo", &selected_zdravilo.m_opis);

			if (selected_zdravilo.m_id == -1)
			{
				if (ImGui::Button("Dodaj##Zdravilo"))
				{
					if (ExecuteUpdate("INSERT INTO zdravilo (ime, kolicina, opis) VALUES ('{}', {}, '{}');", selected_zdravilo.m_ime, selected_zdravilo.m_kolicina, selected_zdravilo.m_opis) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zdravilo dodano!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zdravilo ni dodano - napaka v bazi" });
				}
			}
			else
			{
				if (ImGui::Button("Posodobi##Zdravilo"))
				{
					if (ExecuteUpdate("UPDATE zdravilo SET ime = '{}', kolicina = {}, opis = '{}' WHERE id = {};", selected_zdravilo.m_ime, selected_zdravilo.m_kolicina, selected_zdravilo.m_opis, selected_zdravilo.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zdravilo posodobljeno!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zdravilo ni posodobljeno - napaka v bazi" });
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Počisti##zdravilo"))
				selected_zdravilo = Zdravilo();

			if (scroll_to_bottom)
			{
				scroll_to_bottom = false;
				ImGui::SetScrollHereY();
			}


			ImGui::EndTabItem();
		}

		// ------ BOLNICE ------ \\

		static bool open_bolnice = false;
		if (open_popup_bolnice)
		{
			open_popup_bolnice = false;
			open_bolnice = true;
			ImGui::OpenPopup("Bolnica:##bolnice");
		}

		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Bolnica:##bolnice", &open_bolnice, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrappedCentered(temp_selected_bolnica.m_ime.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##bolnice", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##bolnice", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					selected_tab_pacient = false;
					selected_tab_doktor = false;
					selected_tab_oddelek = false;
					selected_tab_zdravilo = false;

					can_switch = true;
					selected_bolnica = temp_selected_bolnica;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##bolnice", ImVec2(m_screen_size.x * 0.05f, 20.f)))
				{
					if (ExecuteUpdate("DELETE FROM bolnica WHERE id = {};", temp_selected_bolnica.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Bolnica zbrisana" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Bolinca ni zbrisana" });

					selected_bolnica = temp_selected_bolnica = Bolnica();
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}
		if (ImGui::BeginTabItem("Bolnice"))
		{
			if (ImGui::BeginTable("##bolnice", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Ime");
				ImGui::TableSetupColumn("Naslov");
				ImGui::TableSetupColumn("Tel. Številka");
				ImGui::TableSetupColumn("Poštna Številka");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_bolnice.size(); ++row) // row
				{
					auto bolnica = m_bolnice[row];
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 5; column++) // column
					{
						ImGui::TableSetColumnIndex(column);

						if (ImGui::Selectable(bolnica.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0), true))
						{
							temp_selected_bolnica = bolnica;
							open_popup_bolnice = true;
						}
					}
				}
				if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
				{
					if (sortSpecs->SpecsDirty)
					{
						std::sort(
							m_bolnice.begin(), m_bolnice.end(),
							[&sortSpecs](const Bolnica& lhs, const Bolnica& rhs) -> bool {
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
										if (lhs.m_naslov == rhs.m_naslov)
											return false;
										bool sort = lhs.m_naslov.compare(rhs.m_naslov) <= 0 ? false : true;
										return sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? sort : !sort;
									}; break;
									case 3: {
										if (lhs.m_postna_st == rhs.m_postna_st)
											return false;
										bool sort = lhs.m_postna_st > rhs.m_postna_st <= 0 ? false : true;
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

			ImGui::SeparatorText("Bolnica:");

			ImGui::Text("Ime:");
			ImGui::InputText("##bolnica_ime", &selected_bolnica.m_ime, ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), 32);

			ImGui::Text("Naslov:");
			ImGui::InputText("##bolnica_naslov", &selected_bolnica.m_naslov, ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), 32);

			ImGui::Text("Tel. Številka:");
			ImGui::InputText("##bolnica_tel_st", &selected_bolnica.m_tel_st, ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), 9, ImGuiInputTextFlags_CallbackCharFilter, input_filter_numbers_only);

			ImGui::Text("Poštna Številka:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::InputInt("##bolnica_posna_st", (int*)(&selected_bolnica.m_postna_st), 0, 0);

			if (selected_bolnica.m_id == -1)
			{
				if (ImGui::Button("Dodaj##bolince"))
				{
					if (ExecuteUpdate("INSERT INTO bolnica (ime, naslov, tel_st, postna_st) VALUES ('{}', '{}', '{}', {});", selected_bolnica.m_ime, selected_bolnica.m_naslov, selected_bolnica.m_tel_st, selected_bolnica.m_postna_st) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Bolnica dodana!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Bolnica ni dodana - napaka v bazi" });
				}
			}
			else
			{
				if (ImGui::Button("Posodobi##bolince"))
				{
					if (ExecuteUpdate("UPDATE bolnica SET ime = '{}', naslov = '{}', tel_st = '{}', postna_st = {} WHERE id = {};", selected_bolnica.m_ime, selected_bolnica.m_naslov, selected_bolnica.m_tel_st, selected_bolnica.m_postna_st, selected_bolnica.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Bolnica posodobljena!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Bolnica ni posodobljena - napaka v bazi" });
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Počisti##bolince"))
				selected_bolnica = Bolnica();


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

template<typename... Args>
int Driver::ExecuteUpdate(const string& query, Args&&... args)
{
	return ExecuteUpdate(std::vformat(query, std::make_format_args(std::forward<Args>(args)...)));
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