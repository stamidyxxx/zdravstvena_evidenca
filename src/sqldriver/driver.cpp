﻿#include "driver.h"
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

	if (ImGui::BeginTabBar("##topnavbar"))
	{
		static Pacient selected_pacient;
		static Doktor selected_doktor;
		static Oddelek selected_doktor_oddelek;
		static Zapisnik selected_zapisnik;
		static Oddelek selected_oddelek, temp_selected_oddelek;
		static Zdravilo selected_zdravilo;
		static Recept selected_recept;
		static int selected_row_pacient = -1;
		static int selected_row_doktor = -1;
		bool can_switch = false;
		static bool open_popup_oddelki = false;


		if (ImGui::BeginTabItem("Pacienti"))
		{				
			selected_row_pacient = -1;
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
							selected_row_pacient = row; 
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
		if (ImGui::BeginTabItem("Pacient", NULL, selected_row_pacient != -1 && can_switch ? ImGuiTabItemFlags_SetSelected : 0))
		{
			static tm date = { };
			if (!date.tm_year)
				ImGui::SetDateToday(&date);
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
				selected_row_pacient = -1;
				selected_zapisnik = Zapisnik();
				selected_pacient = Pacient();
			}
			if (selected_pacient.m_id != -1)
			{
				ImGui::Spacing(3);

				auto cursor_pos_end = ImGui::GetCursorPos();

				ImGui::SetCursorPos(ImVec2(m_screen_size.x * 0.50f, cursor_pos_start.y));
				ImGui::Text("Datum:");
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
				if (ImGui::Button("Dodaj##termin"))
				{
					auto date_formated = date_to_sql(date);
					if (ExecuteUpdate("INSERT INTO termin (cas_datum, doktor_id, pacient_id) VALUES ('{}', {}, {});", date_formated, selected_doktor_termin.m_id, selected_pacient.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Termin dodan!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Termin ni dodan - napaka v bazi" });
				}

				ImGui::Spacing(10);
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
								ImGui::Text(termin.get(column).c_str());
							else if (column == 2 || column == 3)
								ImGui::Text(termin.m_doktor->get(column - 1).c_str());
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
									ImGui::Text(recept.get(column).c_str());
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
										ImGui::TextWrapped(recept.m_zdravila.c_str());
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
		if (ImGui::BeginTabItem("Zdravniki"))
		{
			selected_row_doktor = -1;
			static string search_term = "";
			ImGui::Spacing();
			ImGui::Text("Iskanje:");
			ImGui::InputText("##SearchBardoktorji", &search_term, ImVec2(m_screen_size.x * 0.2f, 20), 32);

			m_filtered_doktroji.clear();
			if (!search_term.empty() || search_term != "")
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
							selected_row_doktor = row;
							selected_doktor = doktor;
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

		if (ImGui::BeginTabItem("Zdravnik", NULL, selected_row_doktor != -1 && can_switch ? ImGuiTabItemFlags_SetSelected : 0))
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
			ImGui::InputText("##telst_doktor", &selected_doktor.m_tel_st, ImVec2(m_screen_size.x * 0.25f, 0), 32, ImGuiInputTextFlags_CallbackCharFilter, input_filter_numbers_only);

			ImGui::Text("Oddelek:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			if (ImGui::BeginCombo("##doktor", selected_doktor.m_oddelek ? selected_doktor.m_oddelek->m_ime.c_str() : ""))
			{
				for (auto& o : m_oddelki)
					if (ImGui::Selectable(o.m_ime.c_str()))
						selected_doktor.m_oddelek = &o;

				ImGui::EndCombo();
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
					if (ExecuteUpdate("UPDATE doktor SET ime = '{}', priimek = '{}', tel_st = '{}', oddelek_id = '{}' WHERE id = {};", selected_doktor.m_ime, selected_doktor.m_priimek, selected_doktor.m_tel_st, selected_doktor.m_oddelek->m_id, selected_doktor.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Doktor posodobljen!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Doktor ni posodobljen - napaka v bazi" });
			}

			ImGui::SameLine();
			if (ImGui::Button("Počisti##doktor"))
			{
				selected_row_doktor = -1;
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

		static bool open = false;
		if (open_popup_oddelki)
		{
			ImGui::OpenPopup("Oddelek:##oddelki");
			open_popup_oddelki = false;
			open = true;
		}

		ImGui::SetNextWindowSize(m_screen_size / 8);
		if (ImGui::BeginPopupModal("Oddelek:##oddelki", &open, ImGuiWindowFlags_NoResize))
		{
			ImGui::TextWrapped(temp_selected_oddelek.m_ime.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("test", ImGui::GetContentRegionAvail(), 0.5f);

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
			if (ImGui::Button("Izberi"))
			{
				can_switch = true;
				selected_oddelek = temp_selected_oddelek;
				ImGui::CloseCurrentPopup();
			}
		//	ImGui::SameLine();
		//	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.25f);
			ImGui::SetNextItemWidth(500);
			if (ImGui::Button("Izbriši"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}

		if (ImGui::BeginTabItem("Oddelki"))
		{
			if (ImGui::BeginTable("##oddelki", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Ime");
				ImGui::TableSetupColumn("Bolnica");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_oddelki.size(); ++row) // row
				{
					auto oddelek = m_oddelki[row];
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
							m_oddelki.begin(), m_oddelki.end(),
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

			ImGui::Text("Bolnica:");
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			if (ImGui::BeginCombo("##oddelek_bolnica", selected_oddelek.m_bolnica ? selected_oddelek.m_bolnica->m_ime.c_str() : ""))
			{
				for (auto& b : m_bolnice)
					if (ImGui::Selectable(b.m_ime.c_str()))
						selected_oddelek.m_bolnica = &b;

				ImGui::EndCombo();
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
		if (ImGui::BeginTabItem("Zdravila"))
		{
			bool scroll_to_bottom = false;
			if (ImGui::BeginTable("##zdravila", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Ime");
				ImGui::TableSetupColumn("Količina");
				ImGui::TableSetupColumn("Opis");
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_zdravila.size(); ++row) // row
				{
					auto zdravilo = m_zdravila[row];
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 4; column++) // column
					{
						ImGui::TableSetColumnIndex(column);

						auto size = ImGui::CalcTextSize(zdravilo.get(column).c_str(), NULL, false, ImGui::GetContentRegionAvail().x);
						if (ImGui::Selectable(zdravilo.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0), true))
						{
							selected_zdravilo = zdravilo;
							scroll_to_bottom = true;
						}
					}
				}
				if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
				{
					if (sortSpecs->SpecsDirty)
					{
						std::sort(
							m_zdravila.begin(), m_zdravila.end(),
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


		ImGui::EndTabBar();

		selected_row_pacient = -1;
		selected_row_doktor = -1;
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
		Zdravilo* zdravilo = nullptr;
		Recept* recept = nullptr;
		auto zdravilo_id = results_recepti_has_zdravila->getInt(1);
		auto recept_id = results_recepti_has_zdravila->getInt(2);
		for (auto& z : m_zdravila)
			if (zdravilo_id == z.m_id)
				zdravilo = &z;
		for (auto& r : m_recepti)
			if (recept_id == r.m_id)
				recept = &r;

		if (!recept || !zdravilo)
			return false;
		m_recepti_has_zdravila.push_back(Recepti_has_Zdravila(zdravilo, recept));
	}


	return true;
}

Driver driver;