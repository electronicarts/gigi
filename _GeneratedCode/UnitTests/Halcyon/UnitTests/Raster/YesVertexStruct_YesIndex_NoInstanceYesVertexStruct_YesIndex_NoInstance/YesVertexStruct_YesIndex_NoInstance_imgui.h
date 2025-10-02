namespace YesVertexStruct_YesIndex_NoInstance
{
	void MakeUI(halcyon::YesVertexStruct_YesIndex_NoInstanceSettings& settings)
	{
		if (ImGui::CollapsingHeader("YesVertexStruct_YesIndex_NoInstancePass Settings"))
		{
			ImGui::PushID("gigi_YesVertexStruct_YesIndex_NoInstance");
			{
				static const char* labels[] = {
					"UV",
					"Solid",
				};
				ImGui::Combo("viewMode", (int*)&settings.viewMode, labels, 2);
			}
			ImGui::PopID();
		}
	}
}
