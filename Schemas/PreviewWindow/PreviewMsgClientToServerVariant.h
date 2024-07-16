///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

VARIANT_BEGIN(PreviewMsgClientToServer, "Viewer client to server messages")
	VARIANT_TYPE(PreviewMsg_Ping, ping, {}, "")
	VARIANT_TYPE(PreviewMsgCS_Version, version, {}, "")
	VARIANT_TYPE(PreviewMsgCS_Log, log, {}, "")
VARIANT_END()