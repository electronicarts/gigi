///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

VARIANT_BEGIN(PreviewMsgServerToClient, "Viewer server to client messages")
	VARIANT_TYPE(PreviewMsg_Ping, ping, {}, "")
	VARIANT_TYPE(PreviewMsgSC_VersionResponse, versionResponse, {}, "")
	VARIANT_TYPE(PreviewMsgSC_LoadGGFile, loadGGFile, {}, "")
VARIANT_END()
