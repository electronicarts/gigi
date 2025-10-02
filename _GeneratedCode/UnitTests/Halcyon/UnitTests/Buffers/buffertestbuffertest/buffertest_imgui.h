namespace buffertest
{
	void MakeUI(halcyon::buffertestSettings& settings)
	{
		if (ImGui::CollapsingHeader("buffertestPass Settings"))
		{
			ImGui::PushID("gigi_buffertest");
			ImGui::InputFloat("gain", &settings.gain);
			ImGui::InputFloat("alpha1", &settings.alpha1);
			ImGui::InputFloat("alpha2", &settings.alpha2);
			ImGui::PopID();
		}
	}
}
