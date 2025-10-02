namespace YesVertexStruct_NoIndex_NoInstance
{
	void MakeUI(halcyon::YesVertexStruct_NoIndex_NoInstanceSettings& settings)
	{
		if (ImGui::CollapsingHeader("YesVertexStruct_NoIndex_NoInstancePass Settings"))
		{
			ImGui::PushID("gigi_YesVertexStruct_NoIndex_NoInstance");
			{
				static const char* labels[] = {
					"Normal",
					"UV",
					"Solid",
				};
				ImGui::Combo("viewMode", (int*)&settings.viewMode, labels, 3);
			}
			ImGui::PopID();
		}
	}
}
