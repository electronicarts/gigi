/*$(ShaderResources)*/

/*$(_compute:CalculateExtents)*/(uint3 DTid : SV_DispatchThreadID)
{
	if (Canvas[DTid.xy] == 0.0f)
		return;

	uint dummy = 0;
	InterlockedMin(DrawExtents[0].MinX, DTid.x, dummy);
	InterlockedMax(DrawExtents[0].MaxX, DTid.x, dummy);
	InterlockedMin(DrawExtents[0].MinY, DTid.y, dummy);
	InterlockedMax(DrawExtents[0].MaxY, DTid.y, dummy);

	InterlockedAdd(DrawExtents[0].PixelCount, 1, dummy);
	InterlockedAdd(DrawExtents[0].PixelLocationSum.x, DTid.x, dummy);
	InterlockedAdd(DrawExtents[0].PixelLocationSum.y, DTid.y, dummy);
}