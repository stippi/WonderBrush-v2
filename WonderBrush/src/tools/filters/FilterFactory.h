// FilterFactory.h

#ifndef FILTER_FACTORY_H
#define FILTER_FACTORY_H

#include <SupportDefs.h>

class CanvasView;
class FilterObject;
class SettingsFile;

enum {
	// always add new ids at the end of the list,
	// or old project files will break!
	FILTER_DROP_SHADOW	= 0,
	FILTER_INVERT,
	FILTER_GAUSSIAN_BLUR,
	FILTER_SIMPLE_BRIGHTNESS,
	FILTER_CONTRAST,
	FILTER_SATURATION,
	FILTER_MOTION_BLUR,
	FILTER_HALFTONE,
	FILTER_DISPERSION,
	FILTER_NOISE,
	FILTER_WARPSHARP,
	FILTER_SOLID_COLOR,
	FILTER_COLOR_REDUCTION,
	FILTER_DENOISE,

	// keep this last
	FILTER_COUNT
};

class FilterFactory {
 public:
	static	FilterFactory*		Default();

			FilterObject*		MakeFilter(CanvasView* canvasView,
										   int32 id) const;

			bool				GetFilterInfoAt(int32 index,
												const char** name,
												int32* id) const;

			const char*			NameFor(int32 id) const;

			void				SaveSettings(const FilterObject* object);

 private:	
								FilterFactory();
								~FilterFactory();

			const char*			_NameFor(int32 id, bool internal = false) const;
			BMessage*			_SettingsFor(int32 id) const;

	static	FilterFactory		fDefaultFactory;

			SettingsFile*		fFilterSettings;

								// this can be used to control
								// the sorting of filters in the menu
								// see constructor
			int32				fIndexIDMap[FILTER_COUNT];
};

#endif FILTER_FACTORY_H
