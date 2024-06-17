#include "settings.h"

Settings settings;

void Settings::Save()
{
	m_colors = clamp(m_colors, 0, 2);
	m_font_scale = clamp(m_font_scale, 10, 20);
	m_notification_time = clamp(m_notification_time, 1, 5);

	nlohmann::json settings = {
		{"m_is_fullscreen", m_is_fullscreen},
		{"m_vsync", m_vsync},
		{"m_colors", m_colors},
		{"m_font_scale", m_font_scale},
		{"m_notifications", m_notifications},
		{"m_notification_time", m_notification_time},
		{"m_saved_username", m_saved_username},
		{"m_saved_password", m_saved_password}
	};

	ofstream file("nastavitve.json", std::ios::out);
	if (file.is_open())
	{
		file << settings.dump(4);
		file.close();
	}
	else
		cout << "Error! unable to save file \n";
}

void Settings::Load()
{
	ifstream file("nastavitve.json", std::ios::in);
	if (file.is_open())
	{
		nlohmann::json settings;
		file >> settings;
		file.close();

		m_is_fullscreen = settings.value("m_is_fullscreen", true);
		m_vsync = settings.value("m_vsync", false);
		m_colors = settings.value("m_colors", 0);
		m_font_scale = settings.value("m_font_scale", 13);
		m_notifications = settings.value("m_notifications", true);
		m_notification_time = settings.value("m_notification_time", 3);
		m_saved_username = settings.value("m_saved_username", "");
		m_saved_password = settings.value("m_saved_password", "");

		m_colors = clamp(m_colors, 0, 2);
		m_font_scale = clamp(m_font_scale, 10, 20);
		m_notification_time = clamp(m_notification_time, 1, 5);
	}
	else
	{
		Save();
		cout << "Error! unable to load file";
	}
}
