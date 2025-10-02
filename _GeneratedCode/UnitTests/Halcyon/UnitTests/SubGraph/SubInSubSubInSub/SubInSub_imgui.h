namespace SubInSub
{
	void MakeUI(halcyon::SubInSubSettings& settings)
	{
		if (ImGui::CollapsingHeader("SubInSubPass Settings"))
		{
			ImGui::PushID("gigi_SubInSub");
			ImGui::InputFloat("Inner1_Inner1Mult.x", &settings.Inner1_Inner1Mult[0]);
			ImGui::InputFloat("Inner1_Inner1Mult.y", &settings.Inner1_Inner1Mult[1]);
			ImGui::InputFloat("Inner1_Inner1Mult.z", &settings.Inner1_Inner1Mult[2]);
			ImGui::InputFloat("Inner1_Inner1Mult.w", &settings.Inner1_Inner1Mult[3]);
			ImGui::PopID();
		}
	}
}
