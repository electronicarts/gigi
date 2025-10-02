namespace boxblur
{
	void MakeUI(halcyon::boxblurSettings& settings)
	{
		if (ImGui::CollapsingHeader("boxblurPass Settings"))
		{
			ImGui::PushID("gigi_boxblur");
			ImGui::Checkbox("enabled", &settings.enabled);
			ImGui::InputInt("radius", &settings.radius);
			ImGui::Checkbox("sRGB", &settings.sRGB);
			ImGui::PopID();
		}
	}
}
