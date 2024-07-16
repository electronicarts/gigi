///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

enum class GigiBuildFlavor
{
	#define GIGI_BUILD_FLAVOR(BACKEND, FLAVOR, INTERNAL) BACKEND ## _ ## FLAVOR,
	#include "GigiBuildFlavorList.h"
	#undef GIGI_BUILD_FLAVOR
};

inline bool StringToEnum(const char* value, GigiBuildFlavor& out)
{
	#define GIGI_BUILD_FLAVOR(BACKEND, FLAVOR, INTERNAL) \
		if(!_stricmp(value, #BACKEND "_" #FLAVOR)) \
		{ \
			out = GigiBuildFlavor::BACKEND ## _ ## FLAVOR; \
			return true; \
		}
	#include "GigiBuildFlavorList.h"
	#undef GIGI_BUILD_FLAVOR
	return false;
}

inline const char* EnumToString(GigiBuildFlavor value)
{
	switch (value)
	{
		#define GIGI_BUILD_FLAVOR(BACKEND, FLAVOR, INTERNAL) \
			case GigiBuildFlavor::BACKEND ## _ ## FLAVOR: \
			{\
				return #BACKEND "_" #FLAVOR; \
			}
		#include "GigiBuildFlavorList.h"
		#undef GIGI_BUILD_FLAVOR
	}
	return nullptr;
}

inline bool GigiBuildFlavorBackend(GigiBuildFlavor flavor, Backend& out)
{
	switch (flavor)
	{
		#define GIGI_BUILD_FLAVOR(BACKEND, FLAVOR, INTERNAL) \
			case GigiBuildFlavor::BACKEND ## _ ## FLAVOR: \
			{\
				out = Backend::BACKEND; \
				return true; \
			}
		#include "GigiBuildFlavorList.h"
		#undef GIGI_BUILD_FLAVOR
	}

	return false;
}
