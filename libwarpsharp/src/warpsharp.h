// warpsharp.h

#ifndef WARP_SHARP_H
#define WARP_SHARP_H

#if __POWERPC__
#pragma simple_class_byval off
#endif

#include <BeBuild.h>

#if !defined(IMPEXPLIBWARPSHARP) && defined(BUILDING_LIBWARPSHARP)
#define IMPEXPLIBWARPSHARP	_EXPORT
#else
#define IMPEXPLIBWARPSHARP	_IMPORT
#endif

#include <GraphicsDefs.h>
#include <stdlib.h>

// this structure is exported from libwarpsharp,
// and contains the version number
struct WARPSHARP_VERSION_INFO
{
	uchar major;
	uchar minor;
	ushort patchlevel;
};

extern "C" WARPSHARP_VERSION_INFO IMPEXPLIBWARPSHARP LIBWARPSHARP_VERSION;

#endif
