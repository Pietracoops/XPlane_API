#pragma once

#include <functional>

namespace ImGuiUtility
{
	// ImGui Utility function
	void createImGuiDockspace(std::function<void(void)> function);
	void drawImGuiLabelWithColumn(const char* label, std::function<void(void)> function, float columnWidth = 100.0f);
};