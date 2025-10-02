namespace simpleRT
{
	void MakeUI(halcyon::simpleRTSettings& settings)
	{
		if (ImGui::CollapsingHeader("simpleRTPass Settings"))
		{
			ImGui::PushID("gigi_simpleRT");
			ImGui::Checkbox("enabled", &settings.enabled);
			ImGui::InputFloat("hitColor.x", &settings.hitColor[0]);
			ImGui::InputFloat("hitColor.y", &settings.hitColor[1]);
			ImGui::InputFloat("hitColor.z", &settings.hitColor[2]);
			ImGui::InputFloat("missColor.x", &settings.missColor[0]);
			ImGui::InputFloat("missColor.y", &settings.missColor[1]);
			ImGui::InputFloat("missColor.z", &settings.missColor[2]);
			ImGui::PopID();
		}
	}
}
