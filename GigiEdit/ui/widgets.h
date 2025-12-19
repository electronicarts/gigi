///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <imgui.h>
#include <string>
#include "drawing.h"

namespace ax {
namespace Widgets {

using Drawing::IconType;

void Icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1), const ImVec4& innerColor = ImVec4(0, 0, 0, 0));

// personal code (Martin Mittring) copied from https://github.com/Kosmokleaner/MartinAssist (MIT license)
// known issue: if you push on the round part or the lens icon the InputText looses focus
// like ImGui::InputText(label, &value) but with rounded corners and lens symbol
// @param value must not be 0
bool ImGuiSearch(const char* label, std::string* value, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

void ImGuiShowIconsWindow(ImFont* font, bool& show);

} // namespace Widgets
} // namespace ax