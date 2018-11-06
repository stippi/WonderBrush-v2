// FilterFactory.cpp

#include "CanvasView.h"

#include "CImgDeNoise.h"
#include "ColorReduction.h"
#include "Contrast.h"
#include "Dispersion.h"
#include "DropShadow.h"
#include "GaussianBlur.h"
#include "Halftone.h"
#include "Invert.h"
#include "MotionBlur.h"
#include "Noise.h"
#include "Saturation.h"
#include "SimpleBrightness.h"
#include "SolidColor.h"
#include "Warpsharp.h"

#include "LanguageManager.h"
#include "SettingsFile.h"

#include "FilterFactory.h"

// default instance
FilterFactory
FilterFactory::fDefaultFactory;

// constructor
FilterFactory::FilterFactory()
	: fFilterSettings(new SettingsFile("WonderBrush/filter_settings", 'ftst'))
{
	int32 index = 0;
	fIndexIDMap[index++] = FILTER_SIMPLE_BRIGHTNESS;
	fIndexIDMap[index++] = FILTER_CONTRAST;
	fIndexIDMap[index++] = FILTER_SATURATION;

	fIndexIDMap[index++] = FILTER_INVERT;
	fIndexIDMap[index++] = FILTER_COLOR_REDUCTION;

	fIndexIDMap[index++] = FILTER_GAUSSIAN_BLUR;
	fIndexIDMap[index++] = FILTER_MOTION_BLUR;
	fIndexIDMap[index++] = FILTER_DISPERSION;

	fIndexIDMap[index++] = FILTER_DROP_SHADOW;
	fIndexIDMap[index++] = FILTER_HALFTONE;
	fIndexIDMap[index++] = FILTER_NOISE;

	fIndexIDMap[index++] = FILTER_WARPSHARP;
	fIndexIDMap[index++] = FILTER_DENOISE;

	fIndexIDMap[index++] = FILTER_SOLID_COLOR;
}

// destructor
FilterFactory::~FilterFactory()
{
	delete fFilterSettings;
}

// Default
FilterFactory*
FilterFactory::Default()
{
	return &fDefaultFactory;
}

// MakeFilter
FilterObject*
FilterFactory::MakeFilter(CanvasView* view, int32 id) const
{
	FilterObject* filter = NULL;
	BMessage* settings = _SettingsFor(id);
	switch (id) {
		case FILTER_DROP_SHADOW:
			filter = new DropShadow(settings);
			filter->SetColor(view->Color());
			break;
		case FILTER_INVERT:
			filter = new Invert();
			break;
		case FILTER_GAUSSIAN_BLUR:
			filter = new GaussianBlur(settings);
			break;
		case FILTER_SIMPLE_BRIGHTNESS:
			filter = new SimpleBrightness(settings);
			break;
		case FILTER_CONTRAST:
			filter = new Contrast(settings);
			break;
		case FILTER_SATURATION:
			filter = new Saturation(settings);
			break;
		case FILTER_MOTION_BLUR:
			filter = new MotionBlur(settings);
			break;
		case FILTER_HALFTONE:
			filter = new Halftone(settings);
			filter->SetColor(view->Color());
			break;
		case FILTER_DISPERSION: {
			Dispersion* d = new Dispersion(settings);
			d->Randomize();
			filter = d;
			break;
		}
		case FILTER_NOISE: {
			Noise* n = new Noise(settings);
			n->Randomize();
			filter = n;
			break;
		}
		case FILTER_WARPSHARP:
			filter = new Warpsharp(settings);
			break;
		case FILTER_SOLID_COLOR:
			filter = new SolidColor(settings);
			filter->SetColor(view->Color());
			break;
		case FILTER_COLOR_REDUCTION:
			filter = new ColorReduction(settings);
			break;
		case FILTER_DENOISE:
			filter = new CImgDeNoise(settings);
			break;
	}
	return filter;
}

// GetFilterInfoAt
bool
FilterFactory::GetFilterInfoAt(int32 index, const char** name, int32* id) const
{
	bool ret = false;
	*id = -1;
	*name = NULL;

	if (index >= 0 && index < FILTER_COUNT) {
		ret = true;
		*id = fIndexIDMap[index];
		*name = NameFor(*id);

		if (!*name) {
			ret = false;
			*id = -1;
		}
	}

	return ret;
}

// NameFor
const char*
FilterFactory::NameFor(int32 id) const
{
	return _NameFor(id);
}

// SaveSettings
void
FilterFactory::SaveSettings(const FilterObject* object)
{
	BMessage archive;
	if (object->Archive(&archive) >= B_OK) {
		fFilterSettings->SetValue(_NameFor(object->FilterID(), true), archive);
	}
}

// _NameFor
const char*
FilterFactory::_NameFor(int32 id, bool internal) const
{
	LanguageManager* m = LanguageManager::Default();
	const char* name = NULL;
	switch (id) {
		case FILTER_DROP_SHADOW:
			name = internal ? "drop shadow"
							: m->GetString(DROP_SHADOW, "Drop Shadow");
			break;
		case FILTER_INVERT:
			name = internal ? "invert"
							: m->GetString(INVERT, "Invert");
			break;
		case FILTER_GAUSSIAN_BLUR:
			name = internal ? "gaussian blur"
							: m->GetString(GAUSSIAN_BLUR, "Gaussian Blur");
			break;
		case FILTER_SIMPLE_BRIGHTNESS:
			name = internal ? "simple brightness"
							: m->GetString(BRIGHTNESS, "Brightness");
			break;
		case FILTER_CONTRAST:
			name = internal ? "contrast"
							: m->GetString(CONTRAST, "Contrast");
			break;
		case FILTER_SATURATION:
			name = internal ? "saturation"
							: m->GetString(SATURATION, "Saturation");
			break;
		case FILTER_MOTION_BLUR:
			name = internal ? "motion blur"
							: m->GetString(MOTION_BLUR, "Motion Blur");
			break;
		case FILTER_HALFTONE:
			name = internal ? "halftone"
							: m->GetString(HALFTONE, "Halftone");
			break;
		case FILTER_DISPERSION:
			name = internal ? "dispersion"
							: m->GetString(DISPERSION, "Dispersion");
			break;
		case FILTER_NOISE:
			name = internal ? "noise"
							: m->GetString(NOISE, "Noise");
			break;
		case FILTER_WARPSHARP:
			name = internal ? "warpsharp"
							: m->GetString(WARPSHARP, "Warpsharp");
			break;
		case FILTER_SOLID_COLOR:
			name = internal ? "solid area"
							: m->GetString(SOLID_AREA, "Area");
			break;
		case FILTER_COLOR_REDUCTION:
			name = internal ? "color reduction"
							: m->GetString(COLOR_REDUCTION, "Color Reduction");
			break;
		case FILTER_DENOISE:
			name = internal ? "cimg denoise"
							: m->GetString(DE_NOISE, "De-Noise");
			break;
	}
	return name;
}

// _SettingsFor
BMessage*
FilterFactory::_SettingsFor(int32 id) const
{
	BMessage* message = new BMessage();
	if (fFilterSettings->GetValue(_NameFor(id, true), message) < B_OK) {
		delete message;
		message = NULL;
	}
	return message;
}
