namespace TwoRayGensSubgraph
{
	void MakeUI(halcyon::TwoRayGensSubgraphSettings& settings)
	{
		if (ImGui::CollapsingHeader("TwoRayGensSubgraphPass Settings"))
		{
			ImGui::PushID("gigi_TwoRayGensSubgraph");
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
