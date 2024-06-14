#include "menu.h"
#include "../sqldriver/driver.h"
#include "../handler/handler.h"

void Menu::Draw()
{
	driver.m_screen_size = ImGui::GetIO().DisplaySize;

	driver.m_logged_in = true;
	if (!driver.m_logged_in)
	{
		DrawLoginRegister();
		return;
	}

	m_can_switch = false;
	m_selected_tab_pacient = false; 
	m_selected_tab_doktor = false; 
	m_selected_tab_zdravilo = false; 
	m_selected_tab_oddelek = false;

	driver.GetDatabaseVariables();

	if (ImGui::BeginTabBar("##topnavbar"))
	{
		// ------ PACIENTI ------ \\
		
		DrawPacienti();

		// ------ PACIENT ------ \\

		DrawPacient();

		// ------ ZDRAVNIKI ------ \\

		DrawZdravniki();

		// ------ ZDRAVNIK ------ \\

		DrawZdravnik();

		// ------ ODDELKI ------ \\

		DrawOddelki();

		// ------ ZDRAVILA ------ \\

		DrawZdravila();

		// ------ BOLNICE ------ \\
		
		DrawBolnice();


		// ------ NASTAVITVE ------ \\

		DrawNastavitve();

		/*
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Nastavitve").x);

		if (ImGui::BeginTabItem("Nastavitve", NULL, ImGuiTabItemFlags_Trailing))
		{

			ImGui::EndTabItem();
		}
		*/

		ImGui::EndTabBar();
	}
}

void Menu::DrawLoginRegister()
{
	ImVec2 container_size = driver.m_screen_size / ImVec2(3, 3);
	ImVec2 center(driver.m_screen_size.x * 0.5f, driver.m_screen_size.y * 0.5f);
	ImVec2 startPos(center.x - container_size.x * 0.5f, center.y - container_size.y * 0.5f);

	if (driver.m_register_prompt)
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

			if (driver.m_login_error != "")
			{
				ImGui::InsertNotification({ ImGuiToastType_Error, 2000, driver.m_login_error.c_str() });
				driver.m_login_error = "";
			}

			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
			if (ImGui::Button("Register", ImVec2(container_size.x / 2, 20)) || skip_register_button)
			{
				if (strlen(username) < 4)
				{
					driver.m_login_error = "Uporabniško ime je prekratko";
					goto register_end;
				}
				if (strlen(password) < 8)
				{
					driver.m_login_error = "Geslo je prekratko";
					goto register_end;
				}
				if (strcmp(confirmpassword, password) != 0)
				{
					driver.m_login_error = "Gesli se ne ujemata";
					goto register_end;
				}
				string encrypted_password = encryption.Encrypt(password, username);
				if (driver.ExecuteUpdate("INSERT INTO uporabnik (ime, geslo, role) VALUES ('{}', '{}', 'U')", username, encrypted_password) > 0) // returns rows affected
				{
					driver.m_register_prompt = false;
					driver.m_login_error = "";
				}
				else
					driver.m_login_error = "Napaka z bazo podatkov";
			}

		register_end:
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
			if (ImGui::TextSelectable("Nazaj na prijavo"))
				driver.m_register_prompt = false;

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


			if (driver.m_login_error != "")
			{
				ImGui::InsertNotification({ ImGuiToastType_Error, 2000, driver.m_login_error.c_str() });
				driver.m_login_error = "";
			}

			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
			if (ImGui::Button("Log in", ImVec2(container_size.x / 2, 20)) || skip_login_button)
			{
				std::unique_ptr<sql::ResultSet> results(driver.ExecuteQuery("SELECT * FROM uporabnik WHERE ime = '{}'", username));
				if (results)
				{
					if (results->rowsCount() > 0)
					{
						while (results->next())
						{
							string encrypted_password = encryption.Encrypt(password, username);
							if (results->getString("geslo") == encrypted_password)
							{
								driver.m_logged_in = true;
								driver.m_login_error = "";
							}
							else
								driver.m_login_error = "Napačno geslo za uporabnika " + string(username);
						}

					}
					else
						driver.m_login_error = "Napačno uporabniško ime";
				}
				else
					driver.m_login_error = "Napaka v bazi podatkov, poskusite znova";
			}
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + container_size.x * 0.25f, ImGui::GetCursorPosY() + 5));
			if (ImGui::TextSelectable("Nov uporabnik"))
				driver.m_register_prompt = true;

			ImGui::EndChild();
		}
	}
}

void Menu::DrawPacienti()
{
	static bool open_pacienti = false, confirm_pacienti = false;
	if (m_open_popup_pacienti)
	{
		m_open_popup_pacienti = false;
		open_pacienti = true;
		ImGui::OpenPopup("Pacient:##pacienti");
	}

	ImGui::SetNextWindowSize(driver.m_screen_size / 8);
	if (ImGui::BeginPopupModal("Pacient:##pacienti", &open_pacienti, ImGuiWindowFlags_NoResize))
	{
		if (confirm_pacienti)
		{
			auto str = "Izbrisali boste " + m_temp_selected_pacient.get_ime_priimek();
			ImGui::TextWrappedCentered(str.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##confirmpacienti", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Da##confirmpacienti", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					if (driver.ExecuteUpdate("DELETE FROM pacient WHERE id = {};", m_temp_selected_pacient.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Pacient zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Pacient ni zbrisan" });

					m_selected_pacient = m_temp_selected_pacient = Pacient();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Ne##confirmpacienti", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_pacienti = false;
				}
				ImGui::EndHorizontal();
			}
		}
		else
		{
			ImGui::TextWrappedCentered(m_temp_selected_pacient.get_ime_priimek().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##pacienti", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##pacienti", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					m_selected_tab_pacient = true;
					m_selected_tab_doktor = false;
					m_selected_tab_oddelek = false;
					m_selected_tab_zdravilo = false;

					m_can_switch = true;
					m_selected_pacient = m_temp_selected_pacient;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##pacienti", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_pacienti = true;
				}
			}
			ImGui::EndHorizontal();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginTabItem("Pacienti"))
	{
		static string search_term = "";
		ImGui::Spacing();
		ImGui::Text("Iskanje:");
		ImGui::InputText("##SearchBar", &search_term, ImVec2(driver.m_screen_size.x * 0.2f, 20), 32);

		driver.m_filtered_pacienti.clear();
		if (!search_term.empty() || search_term != "")
		{
			for (auto& p : driver.m_pacienti)
			{
				const auto it_priimek = search(p.m_priimek.begin(), p.m_priimek.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_priimek != p.m_priimek.end())
				{
					driver.m_filtered_pacienti.push_back(p);
					continue;
				}

				const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_ime != p.m_ime.end())
				{
					driver.m_filtered_pacienti.push_back(p);
					continue;
				}
			}
		}
		else
			driver.m_filtered_pacienti.insert(driver.m_filtered_pacienti.end(), driver.m_pacienti.begin(), driver.m_pacienti.end());

		if (ImGui::BeginTable("##pacienti", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Ime");
			ImGui::TableSetupColumn("Priimek");
			ImGui::TableSetupColumn("Naslov");
			ImGui::TableSetupColumn("Tel. Številka");
			ImGui::TableSetupColumn("EMŠO");
			ImGui::TableHeadersRow();

			for (int row = 0; row < driver.m_filtered_pacienti.size(); ++row) // row
			{
				auto pacient = driver.m_filtered_pacienti[row];
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
				for (int column = 0; column < 6; column++) // column
				{
					ImGui::TableSetColumnIndex(column);
					if (ImGui::Selectable(pacient.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
					{
						m_temp_selected_pacient = pacient;
						m_open_popup_pacienti = true;
					}
				}
			}

			if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
			{
				if (sortSpecs->SpecsDirty)
				{
					std::sort(
						driver.m_filtered_pacienti.begin(), driver.m_filtered_pacienti.end(),
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
}

void Menu::DrawPacient()
{
	static bool open_termin = false, confirm_termin = false;;
	if (m_open_popup_termin)
	{
		m_open_popup_termin = false;
		open_termin = true;
		ImGui::OpenPopup("Termin:##termin");
	}

	ImGui::SetNextWindowSize(driver.m_screen_size / 8);
	if (ImGui::BeginPopupModal("Termin:##termin", &open_termin, ImGuiWindowFlags_NoResize))
	{
		if (confirm_termin)
		{
			auto str = "Izbrisali boste " + m_temp_selected_termin.get_text();
			ImGui::TextWrappedCentered(str.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##confirmtermin", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Da##confirmtermin", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					if (driver.ExecuteUpdate("DELETE FROM termin WHERE id = {};", m_temp_selected_termin.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Termin zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Termin ni zbrisan" });

					m_selected_termin = m_temp_selected_termin = Termin();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Ne##confirmtermin", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_termin = false;
				}
				ImGui::EndHorizontal();
			}
		}
		else
		{
			ImGui::TextWrappedCentered(m_temp_selected_termin.get_text().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##termin", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##termin", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					m_selected_tab_pacient = false;
					m_selected_tab_doktor = false;
					m_selected_tab_oddelek = false;
					m_selected_tab_zdravilo = false;

					m_can_switch = true;
					m_update_date = true;
					m_selected_termin = m_temp_selected_termin;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##termin", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_termin = true;
				}
			}
			ImGui::EndHorizontal();
		}
		ImGui::EndPopup();
	}

	static bool open_recept = false, confirm_recept;
	if (m_open_popup_recept)
	{
		m_open_popup_recept = false;
		open_recept = true;
		ImGui::OpenPopup("Recept:##recept");
	}

	ImGui::SetNextWindowSize(driver.m_screen_size / 8);
	if (ImGui::BeginPopupModal("Recept:##recept", &open_recept, ImGuiWindowFlags_NoResize))
	{
		if (confirm_recept)
		{
			auto str = "Izbrisali boste " + m_temp_selected_recept.get_text();
			ImGui::TextWrappedCentered(str.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##confirmrecept", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Da##confirmrecept", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					if (driver.ExecuteUpdate("DELETE FROM recept WHERE id = {};", m_temp_selected_termin.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Recept zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Recept ni zbrisan" });

					m_selected_recept = m_temp_selected_recept = Recept();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Ne##confirmrecept", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_recept = false;
				}
				ImGui::EndHorizontal();
			}
		}
		else
		{
			ImGui::TextWrappedCentered(m_temp_selected_recept.get_text().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##recept", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##recept", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					m_selected_tab_pacient = false;
					m_selected_tab_doktor = false;
					m_selected_tab_oddelek = false;
					m_selected_tab_zdravilo = false;

					m_can_switch = true;
					m_selected_recept = m_temp_selected_recept;
					for (auto& rhz : driver.m_recepti_has_zdravila)
						if (rhz.m_recept->m_id == m_selected_recept.m_id)
							m_selected_recept.m_selected_zdravila.emplace(make_pair(rhz.m_zdravilo->m_id, rhz.m_zdravilo->m_ime));

					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##recept", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_recept = true;
				}
			}
			ImGui::EndHorizontal();
		}

		ImGui::EndPopup();
	}
	if (ImGui::BeginTabItem("Pacient", NULL, m_selected_tab_pacient && m_can_switch ? ImGuiTabItemFlags_SetSelected : 0))
	{
		static tm date = { };
		static Doktor selected_doktor_termin;

		auto cursor_pos_start = ImGui::GetCursorPos();
		ImGui::Text("Ime:");
		ImGui::InputText("##name_pacient", &m_selected_pacient.m_ime, ImVec2(driver.m_screen_size.x * 0.25f, 0), 32);
		ImGui::Text("Priimek:");
		ImGui::InputText("##lastname_pacient", &m_selected_pacient.m_priimek, ImVec2(driver.m_screen_size.x * 0.25f, 0), 32);
		ImGui::Text("Naslov:");
		ImGui::InputText("##address_pacient", &m_selected_pacient.m_naslov, ImVec2(driver.m_screen_size.x * 0.25f, 0), 96);
		ImGui::Text("Tel. Številka:");
		ImGui::InputText("##number_pacient", (char*)m_selected_pacient.m_tel_st.c_str(), ImVec2(driver.m_screen_size.x * 0.25f, 0), 9, ImGuiInputTextFlags_CallbackCharFilter, driver.input_filter_numbers_only);
		ImGui::Text("EMŠO:");
		ImGui::InputText("##emso_pacient", (char*)m_selected_pacient.m_emso.c_str(), ImVec2(driver.m_screen_size.x * 0.25f, 0), 13, ImGuiInputTextFlags_CallbackCharFilter, driver.input_filter_numbers_only);
		if (m_selected_pacient.m_id == -1)
		{
			if (ImGui::Button("Dodaj##pacient"))
				if (driver.emso_verify(m_selected_pacient.m_emso))
					if (driver.ExecuteUpdate("INSERT INTO pacient (ime, priimek, naslov, tel_st, emso) VALUES ('{}', '{}', '{}', '{}', '{}');", m_selected_pacient.m_ime, m_selected_pacient.m_priimek, m_selected_pacient.m_naslov, m_selected_pacient.m_tel_st, m_selected_pacient.m_emso) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Pacient dodan!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Pacient ni dodan - napaka v bazi" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "EMŠO ni veljaven!" });
		}
		else
		{
			if (ImGui::Button("Posodobi##pacient"))
				if (driver.emso_verify(m_selected_pacient.m_emso))
					if (driver.ExecuteUpdate("UPDATE pacient SET ime = '{}', priimek = '{}', naslov = '{}', tel_st = '{}', emso = '{}' WHERE id = {};", m_selected_pacient.m_ime, m_selected_pacient.m_priimek, m_selected_pacient.m_naslov, m_selected_pacient.m_tel_st, m_selected_pacient.m_emso, m_selected_pacient.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Pacient posodobljen!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Pacient ni posodobljen - napaka v bazi" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "EMŠO ni veljaven!" });
		}

		ImGui::SameLine();
		if (ImGui::Button("Počisti##pacient"))
		{
			m_selected_zapisnik = Zapisnik();
			m_selected_pacient = Pacient();
		}
		if (m_selected_pacient.m_id != -1)
		{
			ImGui::SeparatorText("Termini");
			if (ImGui::BeginTable("##termini", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Čas");
				ImGui::TableSetupColumn("Ime zdravnika");
				ImGui::TableSetupColumn("Priimek zdravnika");
				ImGui::TableHeadersRow();

				for (int row = 0; row < driver.m_termini.size(); ++row) // row
				{
					auto termin = driver.m_termini[row];
					if (termin.m_pacient->m_id != m_selected_pacient.m_id)
						continue;

					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					for (int column = 0; column < 4; column++) // column
					{
						ImGui::TableSetColumnIndex(column);
						if (column == 0 || column == 1)
						{
							if (ImGui::Selectable(termin.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
							{
								m_temp_selected_termin = termin;
								m_open_popup_termin = true;
							}
						}
						else if (column == 2 || column == 3)
						{
							if (ImGui::Selectable(termin.m_doktor->get(column - 1).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
							{
								m_temp_selected_termin = termin;
								m_open_popup_termin = true;
							}
						}
					}
				}
				if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
				{
					if (sortSpecs->SpecsDirty)
					{
						std::sort(
							driver.m_termini.begin(), driver.m_termini.end(),
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

				if (m_update_date)
				{
					date = driver.sql_to_date(m_selected_termin.m_cas_datum);
					m_update_date = false;
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

				if (driver.m_doktorji_imena.empty())
				{
					driver.m_doktorji_imena.reserve(driver.m_doktroji.size());
					std::transform(driver.m_doktroji.begin(), driver.m_doktroji.end(), std::back_inserter(driver.m_doktorji_imena),
						[](const Doktor& o) { return o.m_ime + " " + o.m_priimek; });
				}
				static int selected_doktor_termin_int;
				ImGui::Text("Zdravnik: ");
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
				if (ImGui::ComboWithFilter("##doktor_termin", &selected_doktor_termin_int, driver.m_doktorji_imena))
				{
					m_selected_termin.m_doktor = &driver.m_doktroji[selected_doktor_termin_int];
				}
				if (m_selected_termin.m_id != -1)
				{
					if (ImGui::Button("Posodobi##termin"))
					{
						auto date_formated = driver.date_to_sql(date);
						if (driver.ExecuteUpdate("UPDATE termin SET cas_datum = '{}', doktor_id = {}, pacient_id = {} WHERE id = {};", date_formated, m_selected_termin.m_doktor->m_id, m_selected_pacient.m_id, m_selected_termin.m_id) > 0)
							ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Termin posodobljen!" });
						else
							ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Termin ni posodobljen - napaka v bazi" });
					}
				}
				else
				{
					if (ImGui::Button("Dodaj##termin"))
					{
						auto date_formated = driver.date_to_sql(date);
						if (driver.ExecuteUpdate("INSERT INTO termin (cas_datum, doktor_id, pacient_id) VALUES ('{}', {}, {});", date_formated, m_selected_termin.m_doktor->m_id, m_selected_pacient.m_id) > 0)
							ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Termin dodan!" });
						else
							ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Termin ni dodan - napaka v bazi" });
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Počisti##termin"))
				{
					m_selected_termin = Termin();
					date = ImGui::GetCurrentDate();
				}

				ImGui::SeparatorText("Recepti");
				if (ImGui::BeginTable("##recepti", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("ID");
					ImGui::TableSetupColumn("Čas");
					ImGui::TableSetupColumn("Zdravila");
					ImGui::TableHeadersRow();

					for (int row = 0; row < driver.m_recepti.size(); ++row) // row
					{
						auto recept = driver.m_recepti[row];
						if (recept.m_pacient->m_id != m_selected_pacient.m_id)
							continue;

						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						for (int column = 0; column < 3; column++) // column
						{
							ImGui::TableSetColumnIndex(column);
							if (column == 0 || column == 1)
							{
								if (ImGui::Selectable(recept.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
								{
									m_temp_selected_recept = recept;
									m_open_popup_recept = true;
								}
							}
							else
							{
								if (recept.m_zdravila == "")
								{
									for (auto& rhz : driver.m_recepti_has_zdravila)
										if (rhz.m_recept->m_id == recept.m_id)
											for (auto& z : driver.m_zdravila)
												if (rhz.m_zdravilo->m_id == z.m_id)
													recept.m_zdravila.append(z.m_ime + ", ");
									if (!recept.m_zdravila.empty())
										recept.m_zdravila.resize(recept.m_zdravila.size() - 2);
								}
								if (recept.m_zdravila != "")
								{
									if (ImGui::Selectable(recept.m_zdravila.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0), true))
									{
										m_temp_selected_recept = recept;
										m_open_popup_recept = true;
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
								driver.m_recepti.begin(), driver.m_recepti.end(),
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

				if (m_selected_recept.m_zdravila.empty() && m_selected_recept.m_id != -1)
				{
					for (auto& z : driver.m_zdravila)
					{
						if (m_selected_recept.m_selected_zdravila.find(z.m_id) != m_selected_recept.m_selected_zdravila.end())
							m_selected_recept.m_zdravila.append(z.m_ime + ", ");
					}
					if (m_selected_recept.m_zdravila.size() > 3 && !m_selected_recept.m_zdravila.empty())
						m_selected_recept.m_zdravila.resize(m_selected_recept.m_zdravila.size() - 2);
				}

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
				if (ImGui::BeginCombo("##Zdravila", m_selected_recept.m_zdravila.c_str()))
				{
					m_selected_recept.m_zdravila.clear();
					for (auto& z : driver.m_zdravila)
					{
						bool is_selected = m_selected_recept.m_selected_zdravila.find(z.m_id) != m_selected_recept.m_selected_zdravila.end();
						if (ImGui::Selectable(z.m_ime.c_str(), is_selected, ImGuiSelectableFlags_DontClosePopups))
						{
							if (is_selected)
								m_selected_recept.m_selected_zdravila.erase(m_selected_recept.m_selected_zdravila.find(z.m_id));
							else
								m_selected_recept.m_selected_zdravila.emplace(make_pair(z.m_id, z.m_ime));
						}

						if (is_selected)
							m_selected_recept.m_zdravila.append(z.m_ime + ", ");
					}
					if (m_selected_recept.m_zdravila.size() > 3 && !m_selected_recept.m_zdravila.empty())
						m_selected_recept.m_zdravila.resize(m_selected_recept.m_zdravila.size() - 2);

					ImGui::EndCombo();
				}

				if (m_selected_recept.m_id != -1)
				{
					if (ImGui::Button("Posodobi##recept"))
					{
						if (driver.ExecuteUpdate("DELETE FROM recepti_has_zdravila WHERE recept_id = {}", m_selected_recept.m_id))
						{
							for (auto& z : m_selected_recept.m_selected_zdravila)
							{
								if (driver.ExecuteUpdate("INSERT INTO recepti_has_zdravila (zdravilo_id, recept_id) VALUES ({}, {})", z.first, m_selected_recept.m_id))
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
						if (m_selected_recept.m_selected_zdravila.size() > 0)
						{
							bool success = false;
							auto datum = driver.date_to_sql(ImGui::GetCurrentDate());
							if (driver.ExecuteUpdate("INSERT INTO recept (cas_datum, pacient_id) VALUES ('{}', {});", datum, m_selected_pacient.m_id) > 0)
							{
								auto results = driver.ExecuteQuery("SELECT LAST_INSERT_ID();");
								while (results->next())
								{
									auto id = results->getInt(1);
									for (auto& zdravilo : m_selected_recept.m_selected_zdravila)
										if (driver.ExecuteUpdate("INSERT INTO recepti_has_zdravila (zdravilo_id, recept_id) VALUES ({}, {});", zdravilo.first, id) <= 0)
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
					m_selected_recept = Recept();
			}
			static bool novi_zapisnik = false;
			ImGui::SeparatorText("Zapisnik");
			cursor_pos_start = ImGui::GetCursorPos();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
			if (ImGui::BeginCombo("##zapisnik", novi_zapisnik ? "Novi" : m_selected_zapisnik.m_naslov.c_str()))
			{
				for (auto& z : driver.m_zapisniki)
					if (z.m_pacient->m_id == m_selected_pacient.m_id)
						if (ImGui::Selectable(z.m_naslov.c_str()))
						{
							m_selected_zapisnik = z;
							novi_zapisnik = false;
						}

				if (ImGui::Selectable("Novi"))
				{
					m_selected_zapisnik = Zapisnik();
					novi_zapisnik = true;
				}
				ImGui::EndCombo();
			}
		//	ImGui::PushTextWrapPos(driver.m_screen_size.x * 0.45f);
		//	ImGui::TextWrapped(m_selected_zapisnik.m_opis.c_str());
		//	ImGui::PopTextWrapPos();

		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, cursor_pos_start.y));
			ImGui::Text("Naslov:");
		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
			ImGui::InputText("##zapisnik_naslov", &m_selected_zapisnik.m_naslov);

		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::Text("Simptomi:");
		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputTextMultiline("##zapisnik_simptomi", &m_selected_zapisnik.m_simptomi, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 45.f));

		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::Text("Znaki:");
		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputTextMultiline("##zapisnik_znaki", &m_selected_zapisnik.m_znaki, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 45.f));

		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::Text("Diagnoza:");
		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputTextMultiline("##zapisnik_diagnoza", &m_selected_zapisnik.m_diagnoza, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 45.f));

		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::Text("Zdravljenje:");
		//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.5f, ImGui::GetCursorPosY()));
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputTextMultiline("##zapisnik_zdravljenje", &m_selected_zapisnik.m_zdravljenje, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 45.f));

			if (novi_zapisnik)
			{
			//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				if (ImGui::Button("Dodaj##zapisnik"))
				{
					auto datum = driver.date_to_sql(ImGui::GetCurrentDate());
					if (driver.ExecuteUpdate("INSERT INTO zapisnik (naslov, simptomi, znaki, diagnoza, zdravljenje, datum, pacient_id) VALUES ('{}', '{}', '{}', '{}', '{}', '{}', {});", m_selected_zapisnik.m_naslov, m_selected_zapisnik.m_simptomi, m_selected_zapisnik.m_znaki, m_selected_zapisnik.m_diagnoza, m_selected_zapisnik.m_zdravljenje, datum, m_selected_pacient.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zapisnik dodan!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zapisnik ni dodan - napaka v bazi" });
				}
			}
			else
			{
			//	ImGui::SetCursorPos(ImVec2(driver.m_screen_size.x * 0.50f, ImGui::GetCursorPosY()));
				if (ImGui::Button("Posodobi##zapisnik"))
					if (driver.ExecuteUpdate("UPDATE zapisnik SET naslov = '{}', simptomi = '{}', znaki = '{}', diagnoza = '{}', zdravljenje = '{}' WHERE id = {};", m_selected_zapisnik.m_naslov, m_selected_zapisnik.m_simptomi, m_selected_zapisnik.m_znaki, m_selected_zapisnik.m_diagnoza, m_selected_zapisnik.m_zdravljenje, m_selected_zapisnik.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zapisnik posodobljen!" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zapisnik ni posodobljen - napaka v bazi" });
			}
			ImGui::SameLine();
			if (ImGui::Button("Počisti##zapisnik"))
			{
				m_selected_zapisnik = Zapisnik();
			}
		}
		ImGui::EndTabItem();
	}
}

void Menu::DrawZdravniki()
{
	static bool open_doktor = false, confirm_zdravniki = false;
	if (m_open_popup_doktor)
	{
		m_open_popup_doktor = false;
		open_doktor = true;
		ImGui::OpenPopup("Zdravnik:##zdravnik");
	}

	ImGui::SetNextWindowSize(driver.m_screen_size / 8);
	if (ImGui::BeginPopupModal("Zdravnik:##zdravnik", &open_doktor, ImGuiWindowFlags_NoResize))
	{
		if (confirm_zdravniki)
		{
			auto str = "Izbrisali boste " + m_temp_selected_doktor.get_ime_priimek();
			ImGui::TextWrappedCentered(str.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##confirmzdravniki", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Da##confirmzdravniki", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					if (driver.ExecuteUpdate("DELETE FROM doktor WHERE id = {};", m_temp_selected_doktor.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Zdravnik zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Zdravnik ni zbrisan" });

					m_selected_doktor = m_temp_selected_doktor = Doktor();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Ne##confirmzdravniki", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_zdravniki = false;
				}
				ImGui::EndHorizontal();
			}
		}
		else
		{
			ImGui::TextWrappedCentered(m_temp_selected_doktor.get_ime_priimek().c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##zdravnik", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##zdravnik", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					m_selected_tab_pacient = false;
					m_selected_tab_doktor = true;
					m_selected_tab_oddelek = false;
					m_selected_tab_zdravilo = false;

					m_can_switch = true;
					m_selected_doktor = m_temp_selected_doktor;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##zdravnik", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_zdravniki = true;
				}
			}
			ImGui::EndHorizontal();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginTabItem("Zdravniki"))
	{
		static string search_term = "";
		ImGui::Spacing();
		ImGui::Text("Iskanje:");
		ImGui::InputText("##SearchBardoktorji", &search_term, ImVec2(driver.m_screen_size.x * 0.2f, 20), 32);

		driver.m_filtered_doktroji.clear();
		if (!search_term.empty() && search_term != "")
		{
			for (auto& p : driver.m_doktroji)
			{
				const auto it_priimek = search(p.m_priimek.begin(), p.m_priimek.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_priimek != p.m_priimek.end())
				{
					driver.m_filtered_doktroji.push_back(p);
					continue;
				}

				const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_ime != p.m_ime.end())
				{
					driver.m_filtered_doktroji.push_back(p);
					continue;
				}
			}
		}
		else
			driver.m_filtered_doktroji.insert(driver.m_filtered_doktroji.end(), driver.m_doktroji.begin(), driver.m_doktroji.end());

		if (ImGui::BeginTable("##doktorji", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Ime");
			ImGui::TableSetupColumn("Priimek");
			ImGui::TableSetupColumn("Oddelek");
			ImGui::TableHeadersRow();

			for (int row = 0; row < driver.m_filtered_doktroji.size(); ++row) // row
			{
				string oddelek_name = "";
				auto doktor = driver.m_filtered_doktroji[row];
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
				for (int column = 0; column < 4; column++) // column
				{
					ImGui::TableSetColumnIndex(column);
					auto text = column == 3 ? doktor.m_oddelek->m_ime : doktor.get(column);
					if (ImGui::Selectable(text.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
					{
						m_temp_selected_doktor = doktor;
						m_open_popup_doktor = true;
					}
				}
			}

			if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
			{
				if (sortSpecs->SpecsDirty)
				{
					std::sort(
						driver.m_doktroji.begin(), driver.m_doktroji.end(),
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
}

void Menu::DrawZdravnik()
{
	if (ImGui::BeginTabItem("Zdravnik", NULL, m_selected_tab_doktor && m_can_switch ? ImGuiTabItemFlags_SetSelected : 0))
	{
		auto cursor_pos_start = ImGui::GetCursorPos();
		ImGui::Text("Ime:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		ImGui::InputText("##name_doktor", &m_selected_doktor.m_ime, ImVec2(driver.m_screen_size.x * 0.25f, 0), 32);

		ImGui::Text("Priimek:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		ImGui::InputText("##lastname_doktor", &m_selected_doktor.m_priimek, ImVec2(driver.m_screen_size.x * 0.25f, 0), 32);

		ImGui::Text("Tel. Številka:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		ImGui::InputText("##telst_doktor", &m_selected_doktor.m_tel_st, ImVec2(driver.m_screen_size.x * 0.25f, 0), 9, ImGuiInputTextFlags_CallbackCharFilter, driver.input_filter_numbers_only);


		if (driver.m_oddelki_imena.empty())
		{
			driver.m_oddelki_imena.reserve(driver.m_oddelki.size());
			std::transform(driver.m_oddelki.begin(), driver.m_oddelki.end(), std::back_inserter(driver.m_oddelki_imena),
				[](const Oddelek& o) { return o.m_ime; });
		}
		static int selected_oddelek_int;
		ImGui::Text("Oddelek:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		if (ImGui::ComboWithFilter("##doktor", &selected_oddelek_int, driver.m_oddelki_imena))
		{
			m_selected_doktor.m_oddelek = &driver.m_oddelki[selected_oddelek_int];
		}

		if (m_selected_doktor.m_id == -1)
		{
			if (ImGui::Button("Dodaj##doktor"))
				if (driver.ExecuteUpdate("INSERT INTO doktor (ime, priimek, tel_st, oddelek_id) VALUES ('{}', '{}', '{}', {});", m_selected_doktor.m_ime, m_selected_doktor.m_priimek, m_selected_doktor.m_tel_st, m_selected_doktor.m_oddelek->m_id) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zdravnik dodan!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zdravnik ni dodan - napaka v bazi" });
		}
		else
		{
			if (ImGui::Button("Posodobi##doktor"))
				if (driver.ExecuteUpdate("UPDATE doktor SET ime = '{}', priimek = '{}', tel_st = '{}', oddelek_id = {} WHERE id = {};", m_selected_doktor.m_ime, m_selected_doktor.m_priimek, m_selected_doktor.m_tel_st, m_selected_doktor.m_oddelek->m_id, m_selected_doktor.m_id) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zdravnik posodobljen!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zdravnik ni posodobljen - napaka v bazi" });
		}

		ImGui::SameLine();
		if (ImGui::Button("Počisti##doktor"))
		{
			m_selected_doktor = Doktor();
		}
		if (m_selected_doktor.m_id != -1)
		{
			ImGui::SeparatorText("Termini");
			if (ImGui::BeginTable("##termini", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Čas");
				ImGui::TableSetupColumn("Ime pacienta");
				ImGui::TableSetupColumn("Priimek pacienta");
				ImGui::TableHeadersRow();

				for (int row = 0; row < driver.m_termini.size(); ++row) // row
				{
					auto termin = driver.m_termini[row];
					if (termin.m_doktor->m_id != m_selected_doktor.m_id)
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
							driver.m_termini.begin(), driver.m_termini.end(),
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
}

void Menu::DrawOddelki()
{
	static int selected_bolnica_oddelek_int;
	static bool open_oddelki = false, confirm_oddelki = false;
	if (m_open_popup_oddelki)
	{
		m_open_popup_oddelki = false;
		open_oddelki = true;
		ImGui::OpenPopup("Oddelek:##oddelki");
	}
	ImGui::SetNextWindowSize(driver.m_screen_size / 8);
	if (ImGui::BeginPopupModal("Oddelek:##oddelki", &open_oddelki, ImGuiWindowFlags_NoResize))
	{
		if (confirm_oddelki)
		{
			auto str = "Izbrisali boste " + m_temp_selected_oddelek.m_ime;
			ImGui::TextWrappedCentered(str.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##confirmoddelki", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Da##confirmoddelki", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					if (driver.ExecuteUpdate("DELETE FROM oddelek WHERE id = {};", m_temp_selected_oddelek.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Oddelek zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Oddelek ni zbrisan" });

					m_selected_oddelek = m_temp_selected_oddelek = Oddelek();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Ne##confirmoddelki", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_oddelki = false;
				}
				ImGui::EndHorizontal();
			}
		}
		else
		{
			ImGui::TextWrappedCentered(m_temp_selected_oddelek.m_ime.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##oddelki", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##oddelki", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					m_selected_tab_pacient = false;
					m_selected_tab_doktor = false;
					m_selected_tab_oddelek = true;
					m_selected_tab_zdravilo = false;

					m_can_switch = true;

					auto id = m_temp_selected_oddelek.m_bolnica->m_id;
					auto it = std::find_if(driver.m_bolnice.begin(), driver.m_bolnice.end(), [id](const Bolnica& z) {
						return z.m_id == id;
						});

					if (it != driver.m_bolnice.end())
						selected_bolnica_oddelek_int = distance(driver.m_bolnice.begin(), it);

					m_selected_oddelek = m_temp_selected_oddelek;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##oddelki", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_oddelki = true;
				}
			}
			ImGui::EndHorizontal();
		}

		ImGui::EndPopup();
	}
	if (ImGui::BeginTabItem("Oddelki"))
	{
		static string search_term = "";
		ImGui::Spacing();
		ImGui::Text("Iskanje:");
		ImGui::InputText("##SearchBaroddelek", &search_term, ImVec2(driver.m_screen_size.x * 0.2f, 20), 32);

		driver.m_filtered_oddeleki.clear();
		if (!search_term.empty() && search_term != "")
		{
			for (auto& p : driver.m_oddelki)
			{
				const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_ime != p.m_ime.end())
				{
					driver.m_filtered_oddeleki.push_back(p);
					continue;
				}

				const auto it_opis = search(p.m_bolnica->m_ime.begin(), p.m_bolnica->m_ime.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_opis != p.m_bolnica->m_ime.end())
				{
					driver.m_filtered_oddeleki.push_back(p);
					continue;
				}
			}
		}
		else
			driver.m_filtered_oddeleki.insert(driver.m_filtered_oddeleki.end(), driver.m_oddelki.begin(), driver.m_oddelki.end());

		if (ImGui::BeginTable("##oddelki", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Ime");
			ImGui::TableSetupColumn("Bolnica");
			ImGui::TableHeadersRow();

			for (int row = 0; row < driver.m_filtered_oddeleki.size(); ++row) // row
			{
				auto oddelek = driver.m_filtered_oddeleki[row];
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
				for (int column = 0; column < 3; column++) // column
				{
					ImGui::TableSetColumnIndex(column);
					if (column == 0 || column == 1)
					{
						if (ImGui::Selectable(oddelek.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
						{
							m_temp_selected_oddelek = oddelek;
							m_open_popup_oddelki = true;
						}
					}
					else if (column == 2)
					{
						if (oddelek.m_bolnica)
						{
							if (ImGui::Selectable(oddelek.m_bolnica->m_ime.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
							{
								m_temp_selected_oddelek = oddelek;
								m_open_popup_oddelki = true;
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
						driver.m_filtered_oddeleki.begin(), driver.m_filtered_oddeleki.end(),
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
		ImGui::InputText("##name_oddelek", &m_selected_oddelek.m_ime, ImVec2(driver.m_screen_size.x * 0.25f, 0), 32);

		if (driver.m_bolnice_imena.empty())
		{
			driver.m_bolnice_imena.reserve(driver.m_bolnice.size());
			std::transform(driver.m_bolnice.begin(), driver.m_bolnice.end(), std::back_inserter(driver.m_bolnice_imena),
				[](const Bolnica& o) { return o.m_ime; });
		}

		ImGui::Text("Bolnica: ");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		if (ImGui::ComboWithFilter("##oddelek_bolnica", &selected_bolnica_oddelek_int, driver.m_bolnice_imena))
		{
			m_selected_oddelek.m_bolnica = &driver.m_bolnice[selected_bolnica_oddelek_int];
		}
		if (m_selected_oddelek.m_id == -1)
		{
			if (ImGui::Button("Dodaj##oddelek"))
			{
				if (driver.ExecuteUpdate("INSERT INTO oddelek (ime, bolnica_id) VALUES ('{}', {});", m_selected_oddelek.m_ime, m_selected_oddelek.m_bolnica->m_id) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Oddelek dodan!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Oddelek ni dodan - napaka v bazi" });
			}
		}
		else
		{
			if (ImGui::Button("Posodobi##oddelek"))
			{
				if (driver.ExecuteUpdate("UPDATE oddelek SET ime = '{}', bolnica_id = {} WHERE id = {};", m_selected_oddelek.m_ime, m_selected_oddelek.m_bolnica->m_id, m_selected_oddelek.m_id) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Oddelek posodobljen!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Oddelek ni posodobljen - napaka v bazi" });
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Počisti##oddelek"))
			m_selected_oddelek = Oddelek();

		ImGui::EndTabItem();
	}
}

void Menu::DrawZdravila()
{
	static bool open_zdravila = false, confirm_zdravila = false;
	bool scroll_to_bottom = false;
	if (m_open_popup_zdravila)
	{
		m_open_popup_zdravila = false;
		open_zdravila = true;
		ImGui::OpenPopup("Zdravila:##zdravila");
	}

	ImGui::SetNextWindowSize(driver.m_screen_size / 8);
	if (ImGui::BeginPopupModal("Zdravila:##zdravila", &open_zdravila, ImGuiWindowFlags_NoResize))
	{
		if (confirm_zdravila)
		{
			auto str = "Izbrisali boste " + m_temp_selected_zdravilo.m_ime;
			ImGui::TextWrappedCentered(str.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##confirmzdravila", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Da##confirmzdravila", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					if (driver.ExecuteUpdate("DELETE FROM zdravilo WHERE id = {};", m_temp_selected_zdravilo.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Zdravilo zbrisan" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Zdravilo ni zbrisan" });

					m_selected_zdravilo = m_temp_selected_zdravilo = Zdravilo();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Ne##confirmzdravila", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_zdravila = false;
				}
				ImGui::EndHorizontal();
			}
		}
		else
		{
			ImGui::TextWrappedCentered(m_temp_selected_zdravilo.m_ime.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##zdravila", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##zdravila", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					m_selected_tab_pacient = false;
					m_selected_tab_doktor = false;
					m_selected_tab_oddelek = false;
					m_selected_tab_zdravilo = true;

					m_can_switch = true;
					m_selected_zdravilo = m_temp_selected_zdravilo;
					scroll_to_bottom = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##zdravila", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_zdravila = true;
				}
			}
			ImGui::EndHorizontal();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginTabItem("Zdravila"))
	{
		static string search_term = "";
		ImGui::Spacing();
		ImGui::Text("Iskanje:");
		ImGui::InputText("##SearchBarzdravila", &search_term, ImVec2(driver.m_screen_size.x * 0.2f, 20), 32);

		driver.m_filtered_zdravila.clear();
		if (!search_term.empty() && search_term != "")
		{
			for (auto& p : driver.m_zdravila)
			{
				const auto it_ime = search(p.m_ime.begin(), p.m_ime.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_ime != p.m_ime.end())
				{
					driver.m_filtered_zdravila.push_back(p);
					continue;
				}

				const auto it_opis = search(p.m_opis.begin(), p.m_opis.end(), search_term.begin(), search_term.end(),
					[](char a, char b) {
						return tolower(a) == tolower(b);
					});
				if (it_opis != p.m_opis.end())
				{
					driver.m_filtered_zdravila.push_back(p);
					continue;
				}
			}
		}
		else
			driver.m_filtered_zdravila.insert(driver.m_filtered_zdravila.end(), driver.m_zdravila.begin(), driver.m_zdravila.end());

		if (ImGui::BeginTable("##zdravila", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Ime");
			ImGui::TableSetupColumn("Količina");
			ImGui::TableSetupColumn("Opis");
			ImGui::TableHeadersRow();

			for (int row = 0; row < driver.m_filtered_zdravila.size(); ++row) // row
			{
				auto zdravilo = driver.m_filtered_zdravila[row];
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
				for (int column = 0; column < 4; column++) // column
				{
					ImGui::TableSetColumnIndex(column);

					if (ImGui::Selectable(zdravilo.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0), true))
					{
						m_temp_selected_zdravilo = zdravilo;
						m_open_popup_zdravila = true;
					}
				}
			}
			if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
			{
				if (sortSpecs->SpecsDirty)
				{
					std::sort(
						driver.m_filtered_zdravila.begin(), driver.m_filtered_zdravila.end(),
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
		ImGui::InputText("##ime_zdravilo", &m_selected_zdravilo.m_ime, ImVec2(driver.m_screen_size.x * 0.25f, 0), 32);

		ImGui::Text("Količina:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		ImGui::SliderInt("##kolicina_zdravilo", &m_selected_zdravilo.m_kolicina, 1, 1000, "%dg");

		ImGui::Text("Opis:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		ImGui::InputTextMultiline("##opis_zdravilo", &m_selected_zdravilo.m_opis);

		if (m_selected_zdravilo.m_id == -1)
		{
			if (ImGui::Button("Dodaj##Zdravilo"))
			{
				if (driver.ExecuteUpdate("INSERT INTO zdravilo (ime, kolicina, opis) VALUES ('{}', {}, '{}');", m_selected_zdravilo.m_ime, m_selected_zdravilo.m_kolicina, m_selected_zdravilo.m_opis) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zdravilo dodano!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zdravilo ni dodano - napaka v bazi" });
			}
		}
		else
		{
			if (ImGui::Button("Posodobi##Zdravilo"))
			{
				if (driver.ExecuteUpdate("UPDATE zdravilo SET ime = '{}', kolicina = {}, opis = '{}' WHERE id = {};", m_selected_zdravilo.m_ime, m_selected_zdravilo.m_kolicina, m_selected_zdravilo.m_opis, m_selected_zdravilo.m_id) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Zdravilo posodobljeno!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Zdravilo ni posodobljeno - napaka v bazi" });
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Počisti##zdravilo"))
			m_selected_zdravilo = Zdravilo();

		if (scroll_to_bottom)
		{
			scroll_to_bottom = false;
			ImGui::SetScrollHereY();
		}


		ImGui::EndTabItem();
	}
}

void Menu::DrawBolnice()
{
	static bool open_bolnice = false, confirm_bolnice;
	if (m_open_popup_bolnice)
	{
		m_open_popup_bolnice = false;
		open_bolnice = true;
		ImGui::OpenPopup("Bolnišnica:##bolnice");
	}

	ImGui::SetNextWindowSize(driver.m_screen_size / 8);
	if (ImGui::BeginPopupModal("Bolnišnica:##bolnice", &open_bolnice, ImGuiWindowFlags_NoResize))
	{
		if (confirm_bolnice)
		{
			auto str = "Izbrisali boste " + m_temp_selected_bolnica.m_ime;
			ImGui::TextWrappedCentered(str.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##confirmbolnice", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Da##confirmbolnice", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					if (driver.ExecuteUpdate("DELETE FROM bolnica WHERE id = {};", m_temp_selected_bolnica.m_id) > 0)
						ImGui::InsertNotification({ ImGuiToastType_Success, 2000, "Bolnica zbrisana" });
					else
						ImGui::InsertNotification({ ImGuiToastType_Warning, 2000, "Bolinca ni zbrisana" });

					m_selected_bolnica = m_temp_selected_bolnica = Bolnica();
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Ne##confirmbolnice", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_bolnice = false;
				}
				ImGui::EndHorizontal();
			}
		}
		else
		{
			ImGui::TextWrappedCentered(m_temp_selected_bolnica.m_ime.c_str());
			ImGui::Spacing();

			ImGui::BeginHorizontal("##bolnice", ImGui::GetContentRegionAvail(), 1.f);
			{
				if (ImGui::Button("Izberi##bolnice", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					m_selected_tab_pacient = false;
					m_selected_tab_doktor = false;
					m_selected_tab_oddelek = false;
					m_selected_tab_zdravilo = false;

					m_can_switch = true;
					m_selected_bolnica = m_temp_selected_bolnica;
					ImGui::CloseCurrentPopup();
				}

				ImGui::Spring(0.5f);
				if (ImGui::Button("Izbriši##bolnice", ImVec2(driver.m_screen_size.x * 0.05f, 20.f)))
				{
					confirm_bolnice = true;
				}
			}
			ImGui::EndHorizontal();
		}

		ImGui::EndPopup();
	}
	if (ImGui::BeginTabItem("Bolnišnica"))
	{
		if (ImGui::BeginTable("##bolnice", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable))
		{
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("Ime");
			ImGui::TableSetupColumn("Naslov");
			ImGui::TableSetupColumn("Tel. Številka");
			ImGui::TableSetupColumn("Poštna Številka");
			ImGui::TableHeadersRow();

			for (int row = 0; row < driver.m_bolnice.size(); ++row) // row
			{
				auto bolnica = driver.m_bolnice[row];
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
				for (int column = 0; column < 5; column++) // column
				{
					ImGui::TableSetColumnIndex(column);

					if (ImGui::Selectable(bolnica.get(column).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 0), true))
					{
						m_temp_selected_bolnica = bolnica;
						m_open_popup_bolnice = true;
					}
				}
			}
			if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
			{
				if (sortSpecs->SpecsDirty)
				{
					std::sort(
						driver.m_bolnice.begin(), driver.m_bolnice.end(),
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
									bool sort = lhs.m_postna_st > rhs.m_postna_st ? false : true;
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
		ImGui::InputText("##bolnica_ime", &m_selected_bolnica.m_ime, ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), 32);

		ImGui::Text("Naslov:");
		ImGui::InputText("##bolnica_naslov", &m_selected_bolnica.m_naslov, ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), 32);

		ImGui::Text("Tel. Številka:");
		ImGui::InputText("##bolnica_tel_st", &m_selected_bolnica.m_tel_st, ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 0), 9, ImGuiInputTextFlags_CallbackCharFilter, driver.input_filter_numbers_only);

		ImGui::Text("Poštna Številka:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		ImGui::InputInt("##bolnica_posna_st", (int*)(&m_selected_bolnica.m_postna_st), 0, 0);

		if (m_selected_bolnica.m_id == -1)
		{
			if (ImGui::Button("Dodaj##bolince"))
			{
				if (driver.ExecuteUpdate("INSERT INTO bolnica (ime, naslov, tel_st, postna_st) VALUES ('{}', '{}', '{}', {});", m_selected_bolnica.m_ime, m_selected_bolnica.m_naslov, m_selected_bolnica.m_tel_st, m_selected_bolnica.m_postna_st) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Bolnica dodana!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Bolnica ni dodana - napaka v bazi" });
			}
		}
		else
		{
			if (ImGui::Button("Posodobi##bolince"))
			{
				if (driver.ExecuteUpdate("UPDATE bolnica SET ime = '{}', naslov = '{}', tel_st = '{}', postna_st = {} WHERE id = {};", m_selected_bolnica.m_ime, m_selected_bolnica.m_naslov, m_selected_bolnica.m_tel_st, m_selected_bolnica.m_postna_st, m_selected_bolnica.m_id) > 0)
					ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Bolnica posodobljena!" });
				else
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Bolnica ni posodobljena - napaka v bazi" });
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Počisti##bolince"))
			m_selected_bolnica = Bolnica();


		ImGui::EndTabItem();
	}
}

void Menu::DrawNastavitve()
{
	static bool fullscreen = true, vsync = false, anti_aliasing = true, notifications = true;
	static float global_scale = 1.f;
	static int notification_time = 3;
	const char* barve[] = {"Privzeto", "Temno", "Svetlo"};
	ImGuiStyle& style = ImGui::GetStyle();
	if (ImGui::BeginTabItem("Nastavitve"))
	{
		if (ImGui::Checkbox("Fullscreen", &fullscreen))
			handler.ToggleFullscreen();

		if (ImGui::Checkbox("VSync", &vsync))
			handler.m_vsync = vsync;

		if (ImGui::Checkbox("Anti-Aliasing", &anti_aliasing))
		{
			style.AntiAliasedFill = anti_aliasing;
			style.AntiAliasedLines = anti_aliasing;
			style.AntiAliasedLinesUseTex = anti_aliasing;
		}

		ImGui::Text("Barve:");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		if (ImGui::BeginCombo("##barve",barve[handler.m_colors]))
		{
			for (int i = 0; i < IM_ARRAYSIZE(barve); ++i)
			{
				if (ImGui::Selectable(barve[i]))
					handler.m_colors = i;
			}
			ImGui::EndCombo();
		}
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
		ImGui::SliderInt("Font Scale", &handler.m_font_scale, 10.f, 20.f, NULL, ImGuiSliderFlags_NoInput);

		ImGui::Checkbox("Notifikacije", &notifications);
		if (notifications)
		{
			ImGui::SliderInt("Dolžina", &notification_time, 1, 5);
		}

		ImGui::EndTabItem();
	}
}

Menu menu;