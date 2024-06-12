#pragma once
#include <vector>

// Built using imgui v1.78 WIP
#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

// https://github.com/forrestthewoods/lib_fts/blob/632ca1ea82bdf65688241bb8788c77cb242fba4f/code/fts_fuzzy_match.h
#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fts_fuzzy_match.h"
#include <string>
#include <algorithm>

namespace ImGui
{
	bool ComboWithFilter(const char* label, int* current_item, std::vector<std::string>& items, int popup_max_height_in_items = -1);
	static float CalcMaxPopupHeightFromItemCount(int items_count);
	static int index_of_key(std::vector<std::pair<int, int> > pair_list, int key);
	static bool sortbysec_desc(const std::pair<int, int>& a, const std::pair<int, int>& b);
}