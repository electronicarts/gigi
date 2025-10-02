namespace SlangAutoDiff
{
	void MakeUI(halcyon::SlangAutoDiffSettings& settings)
	{
		if (ImGui::CollapsingHeader("SlangAutoDiffPass Settings"))
		{
			ImGui::PushID("gigi_SlangAutoDiff");
			ImGui::InputInt("NumGaussians", &settings.NumGaussians);
			settings.Reset = ImGui::Button("Reset");
			ImGui::InputFloat("LearningRate", &settings.LearningRate);
			ImGui::InputFloat("MaximumStepSize", &settings.MaximumStepSize);
			ImGui::Checkbox("UseBackwardAD", &settings.UseBackwardAD);
			ImGui::Checkbox("QuantizeDisplay", &settings.QuantizeDisplay);
			ImGui::PopID();
		}
	}
}
