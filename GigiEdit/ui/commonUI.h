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

