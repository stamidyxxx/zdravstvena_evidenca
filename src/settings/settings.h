#pragma once
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Settings
{
public:
	void Save();
	void Load();


	bool m_is_fullscreen = true;
	bool m_vsync = false;
	int m_colors = 0;
	int m_font_scale = 13;
	bool m_notifications = true;
	int m_notification_time = 3;

	string m_saved_username;
	string m_saved_password;
};

extern Settings settings;