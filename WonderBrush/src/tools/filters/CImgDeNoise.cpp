#include "CImgDeNoise.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>

#include "bitmap_compression.h"
#include "bitmap_support.h"
#include "blending.h"
#include "defines.h"
#include "support.h"

#include "BoolProperty.h"
#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
#include "FloatProperty.h"
#include "LanguageManager.h"
#include "OptionProperty.h"
#include "PropertyObject.h"
#include "Strings.h"

// define these before including CImg.h (not necessary though)
#define cimg_display_type 0
#define cimg_OS 0
#include "CImg.h"
using namespace cimg_library;

enum {
	INTERPOLATION_NN = 0,
	INTERPOLATION_LINEAR,
	INTERPOLATION_RUNGE_KUTTA,
};

// constructor
CImgDeNoise::CImgDeNoise()
	: FilterObject(FILTER_DENOISE),
	  fAmplitude(30.0),
	  fSharpness(0.8),
	  fAnisotropy(0.3),
	  fAlpha(1.2),
	  fSigma(0.8),
	  fDL(0.8),
	  fDA(30.0),
	  fGaussPrecision(2.0),
	  fInterpolationMode(0),
	  fFastApproximation(true)
{
}

// copy constructor
CImgDeNoise::CImgDeNoise(const CImgDeNoise& other)
	: FilterObject(other),
	  fAmplitude(other.fAmplitude),
	  fSharpness(other.fSharpness),
	  fAnisotropy(other.fAnisotropy),
	  fAlpha(other.fAlpha),
	  fSigma(other.fSigma),
	  fDL(other.fDL),
	  fDA(other.fDA),
	  fGaussPrecision(other.fGaussPrecision),
	  fInterpolationMode(other.fInterpolationMode),
	  fFastApproximation(other.fFastApproximation)
{
}

// BArchivable constructor
CImgDeNoise::CImgDeNoise(BMessage* archive)
	: FilterObject(archive),
	  fAmplitude(30.0),
	  fSharpness(0.8),
	  fAnisotropy(0.3),
	  fAlpha(1.2),
	  fSigma(0.8),
	  fDL(0.8),
	  fDA(30.0),
	  fGaussPrecision(2.0),
	  fInterpolationMode(0),
	  fFastApproximation(true)
{
	if (!archive) {
		SetFilterID(FILTER_DENOISE);
		return;
	}
	float f;
	if (archive->FindFloat("amplitude", &f) == B_OK)
		fAmplitude = f;
	if (archive->FindFloat("sharpness", &f) == B_OK)
		fSharpness = f;
	if (archive->FindFloat("anisotropy", &f) == B_OK)
		fAnisotropy = f;
	if (archive->FindFloat("alpha", &f) == B_OK)
		fAlpha = f;
	if (archive->FindFloat("sigma", &f) == B_OK)
		fSigma = f;
	if (archive->FindFloat("dl", &f) == B_OK)
		fDL = f;
	if (archive->FindFloat("da", &f) == B_OK)
		fDA = f;
	if (archive->FindFloat("gauss precision", &f) == B_OK)
		fGaussPrecision = f;

	int32 i;
	if (archive->FindInt32("interpolation mode", &i) == B_OK)
		fInterpolationMode = i;
	bool b;
	if (archive->FindBool("fast approximation", &b) == B_OK)
		fFastApproximation = b;
}

// destructor
CImgDeNoise::~CImgDeNoise()
{
}

// Clone
Stroke*
CImgDeNoise::Clone() const
{
	return new CImgDeNoise(*this);
}

// SetTo
bool
CImgDeNoise::SetTo(const Stroke* from)
{
	const CImgDeNoise* denoise = dynamic_cast<const CImgDeNoise*>(from);

	AutoNotificationSuspender _(this);

	if (denoise && FilterObject::SetTo(from)) {
		fAmplitude = denoise->fAmplitude;
		fSharpness = denoise->fSharpness;
		fAnisotropy = denoise->fAnisotropy;
		fAlpha = denoise->fAlpha;
		fSigma = denoise->fSigma;
		fDL = denoise->fDL;
		fDA = denoise->fDA;
		fGaussPrecision = denoise->fGaussPrecision;
		fInterpolationMode = denoise->fInterpolationMode;
		fFastApproximation = denoise->fFastApproximation;

		Notify();
		return true;
	}
	return false;
}

// ExtendRebuildArea
void
CImgDeNoise::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	area = layerBounds;
}

// Instantiate
BArchivable*
CImgDeNoise::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "CImgDeNoise"))
		return new CImgDeNoise(archive);
	return NULL;
}

// Archive
status_t
CImgDeNoise::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status == B_OK)
		status = into->AddFloat("amplitude", fAmplitude);
	if (status == B_OK)
		status = into->AddFloat("sharpness", fSharpness);
	if (status == B_OK)
		status = into->AddFloat("anisotropy", fAnisotropy);
	if (status == B_OK)
		status = into->AddFloat("alpha", fAlpha);
	if (status == B_OK)
		status = into->AddFloat("sigma", fSigma);
	if (status == B_OK)
		status = into->AddFloat("dl", fDL);
	if (status == B_OK)
		status = into->AddFloat("da", fDA);
	if (status == B_OK)
		status = into->AddFloat("gauss precision", fGaussPrecision);

	if (status == B_OK)
		status = into->AddInt32("interpolation mode", fInterpolationMode);
	if (status == B_OK)
		status = into->AddBool("fast approximation", fFastApproximation);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "CImgDeNoise");

	return status;
}

// ProcessBitmap
void
CImgDeNoise::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	try {
		uint32 width = dest->Bounds().IntegerWidth() + 1;
		uint32 height = dest->Bounds().IntegerHeight() + 1;

		CImg<uint8> image(width, height, 1, 3);

		uint8* src = (uint8*)dest->Bits();
		uint8* dst = (uint8*)image.data;
		uint32 srcBPR = dest->BytesPerRow();

		// copy dest contents into image
		for (uint32 y = 0; y < height; y++) {
			uint8* s = src;
			uint8* d1 = dst;
			uint8* d2 = dst + width * height;
			uint8* d3 = dst + 2 * width * height;
			for (uint32 x = 0; x < width; x++) {
				*d1++ = s[0];
				*d2++ = s[1];
				*d3++ = s[2];
				s += 4;
			}
			src += srcBPR;
			dst += width;
		}
	
		image.blur_anisotropic(fAmplitude, fSharpness, fAnisotropy,
							   fAlpha, fSigma, fDL, fDA, fGaussPrecision,
							   fInterpolationMode, fFastApproximation);

		// copy result back into dest
		src = (uint8*)dest->Bits();
		dst = (uint8*)image.data;
		for (uint32 y = 0; y < height; y++) {
			uint8* s = src;
			uint8* d1 = dst;
			uint8* d2 = dst + width * height;
			uint8* d3 = dst + 2 * width * height;
			for (uint32 x = 0; x < width; x++) {
				s[0] = *d1++;
				s[1] = *d2++;
				s[2] = *d3++;
				s += 4;
			}
			src += srcBPR;
			dst += width;
		}

	} catch (...) {
		printf("CImgDeNoise::ProcessBitmap() - caught exception!\n");
	}
}

// MakePropertyObject
PropertyObject*
CImgDeNoise::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();

	object->AddProperty(new FloatProperty("amplitude",
										  PROPERTY_CIMG_AMPLITUDE,
										  fAmplitude,
										  0.01, 1000.0));

	object->AddProperty(new FloatProperty("sharpness",
										  PROPERTY_CIMG_SHARPNESS,
										  fSharpness,
										  0.01, 1000.0));

	object->AddProperty(new FloatProperty("anisotropy",
										  PROPERTY_CIMG_ANISOTROPY,
										  fAnisotropy,
										  0.0, 1.0));

	object->AddProperty(new FloatProperty("alpha",
										  PROPERTY_CIMG_ALPHA,
										  fAlpha,
										  0.01, 1000.0));

	object->AddProperty(new FloatProperty("sigma",
										  PROPERTY_CIMG_SIGMA,
										  fSigma,
										  0.0, 1000.0));

	object->AddProperty(new FloatProperty("dl",
										  PROPERTY_CIMG_DL,
										  fDL,
										  0.01, 1000.0));

	object->AddProperty(new FloatProperty("da",
										  PROPERTY_CIMG_DA,
										  fDA,
										  0.01, 1000.0));

	object->AddProperty(new FloatProperty("gauss_precision",
										  PROPERTY_CIMG_GAUSS_PRECISION,
										  fGaussPrecision,
										  0.01, 1000.0));

	// interpolation
	OptionProperty* property = new OptionProperty("interpolation",
												  PROPERTY_CIMG_INTERPOLATION);
	LanguageManager* m = LanguageManager::Default();
	property->AddOption(INTERPOLATION_NN, m->GetString(NEAREST_NEIGHBOR, "Repeat"));
	property->AddOption(INTERPOLATION_LINEAR, m->GetString(LINEAR, "Linear"));
	property->AddOption(INTERPOLATION_RUNGE_KUTTA, "Runge-Kutta");
	property->SetCurrentOptionID(fInterpolationMode);

	object->AddProperty(property);

	// fast approximation
	object->AddProperty(new BoolProperty("fast approximation",
										  PROPERTY_CIMG_FAST_APPROXIMATION,
										  fFastApproximation));
	return object;
}

// SetToPropertyObject
bool
CImgDeNoise::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		// amplitude
		float f = object->FindFloatProperty(PROPERTY_CIMG_AMPLITUDE, fAmplitude);
		if (f != fAmplitude)
			ret = true;
		fAmplitude = f;

		// sharpness
		f = object->FindFloatProperty(PROPERTY_CIMG_SHARPNESS, fSharpness);
		if (f != fSharpness)
			ret = true;
		fSharpness = f;

		// anisotropy
		f = object->FindFloatProperty(PROPERTY_CIMG_ANISOTROPY, fAnisotropy);
		if (f != fAnisotropy)
			ret = true;
		fAnisotropy = f;

		// alpha
		f = object->FindFloatProperty(PROPERTY_CIMG_ALPHA, fAlpha);
		if (f != fAlpha)
			ret = true;
		fAlpha = f;

		// sigma
		f = object->FindFloatProperty(PROPERTY_CIMG_SIGMA, fSigma);
		if (f != fSigma)
			ret = true;
		fSigma = f;

		// dl
		f = object->FindFloatProperty(PROPERTY_CIMG_DL, fDL);
		if (f != fDL)
			ret = true;
		fDL = f;

		// da
		f = object->FindFloatProperty(PROPERTY_CIMG_DA, fDA);
		if (f != fDA)
			ret = true;
		fDA = f;

		// gauss precision
		f = object->FindFloatProperty(PROPERTY_CIMG_GAUSS_PRECISION, fGaussPrecision);
		if (f != fGaussPrecision)
			ret = true;
		fGaussPrecision = f;

		// interpolation
		OptionProperty* property = dynamic_cast<OptionProperty*>(
			object->FindProperty(PROPERTY_CIMG_INTERPOLATION));
		if (property) {
			int32 interpolation = property->CurrentOptionID();
			if (fInterpolationMode != (uint32)interpolation) {
				fInterpolationMode = (uint32)interpolation;
				ret = true;
			}
		}
		
		// fast approximation
		bool b = object->FindBoolProperty(PROPERTY_CIMG_FAST_APPROXIMATION,
			fFastApproximation);
		if (b != fFastApproximation)
			ret = true;
		fFastApproximation = b;


		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

