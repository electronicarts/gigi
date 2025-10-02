namespace YesVertexStruct_NoIndex_YesInstanceStruct
{
	void MakeUI(halcyon::YesVertexStruct_NoIndex_YesInstanceStructSettings& settings)
	{
		if (ImGui::CollapsingHeader("YesVertexStruct_NoIndex_YesInstanceStructPass Settings"))
		{
			ImGui::PushID("gigi_YesVertexStruct_NoIndex_YesInstanceStruct");
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
