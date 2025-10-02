namespace SubGraphLoops
{
	void MakeUI(halcyon::SubGraphLoopsSettings& settings)
	{
		if (ImGui::CollapsingHeader("SubGraphLoopsPass Settings"))
		{
			ImGui::PushID("gigi_SubGraphLoops");
			ImGui::Checkbox("FilterSub_Iteration_0_sRGB", &settings.FilterSub_Iteration_0_sRGB);
			ImGui::Checkbox("FilterSub_Iteration_1_sRGB", &settings.FilterSub_Iteration_1_sRGB);
			ImGui::Checkbox("FilterSub_Iteration_2_sRGB", &settings.FilterSub_Iteration_2_sRGB);
			ImGui::Checkbox("FilterSub_Iteration_3_sRGB", &settings.FilterSub_Iteration_3_sRGB);
			ImGui::Checkbox("FilterSub_Iteration_4_sRGB", &settings.FilterSub_Iteration_4_sRGB);
			ImGui::PopID();
		}
	}
}
