#pragma once
#include "../includes.h"
#include "../sqldriver/driver.h"

class Menu
{
public:
	void Draw();

	void DrawLoginRegister();
	void DrawPacienti();
	void DrawPacient();
	void DrawZdravniki();
	void DrawZdravnik();
	void DrawOddelki();
	void DrawZdravila();
	void DrawBolnice();

private:
	Pacient m_selected_pacient, m_temp_selected_pacient;
	Doktor m_selected_doktor, m_temp_selected_doktor;
	Zapisnik m_selected_zapisnik;
	Oddelek m_selected_oddelek, m_temp_selected_oddelek;
	Zdravilo m_selected_zdravilo, m_temp_selected_zdravilo;
	Recept m_selected_recept, m_temp_selected_recept;
	Termin m_selected_termin, m_temp_selected_termin;
	Bolnica m_selected_bolnica, m_temp_selected_bolnica;
	bool m_can_switch;
	bool m_open_popup_oddelki, m_open_popup_zdravila, m_open_popup_pacienti, m_open_popup_doktor, m_open_popup_termin, m_open_popup_recept, m_open_popup_bolnice;
	bool m_selected_tab_pacient, m_selected_tab_doktor, m_selected_tab_zdravilo, m_selected_tab_oddelek;
	bool m_update_date;
};

extern Menu menu;