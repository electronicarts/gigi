///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// IMGUI helper

// get a scoped enabled/disabled or not grayed / grayed UI using RAII
// e.g.
// {
//   ImGui_Enabled(shouldBeEnabled);
//   ImGui::Button("Press");
// }
class ImGui_Enabled
{
	bool enabled;
public:
	ImGui_Enabled(bool inEnabled)
		: enabled(inEnabled)
	{
		if (!enabled)
			ImGui::BeginDisabled();
	}
	~ImGui_Enabled()
	{
		if (!enabled)
			ImGui::EndDisabled();
	}
};


// RAII: https://en.cppreference.com/w/cpp/language/raii
// use like this:
// {
//   ImStyleColor_RAII col;
//   ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,0,0,1));
//   ImGui::Text("Test");
// }
// or
// {
//   ImStyleColor_RAII col(2);
//   ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,0,0,1));
//   ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(1,0,0,1));
//   ImGui::Text("Test");
// }
struct ImStyleColor_RAII
{
    uint32_t count = 0;

    ImStyleColor_RAII(const uint32_t inCount = 1)
        : count(inCount)
    {
    }

    ~ImStyleColor_RAII()
    {
        ImGui::PopStyleColor(count);
    }
};
// similar to ImStyleColor_RAII
struct ImStyleVar_RAII
{
    uint32_t count = 0;

    ImStyleVar_RAII(const uint32_t inCount = 1)
        : count(inCount)
    {
    }

    ~ImStyleVar_RAII()
    {
        ImGui::PopStyleVar(count);
    }
};