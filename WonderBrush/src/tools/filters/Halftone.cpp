// Halftone.cpp

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

#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
#include "LanguageManager.h"
#include "OptionProperty.h"
#include "PropertyObject.h"
#include "RandomNumberGenerator.h"

#include "Halftone.h"

enum {
	HALFTONE_ROUND_DOT,
	HALFTONE_DIAGONAL_LINE,
	HALFTONE_DITHER,
	HALFTONE_FS_DITHER,
	HALFTONE_NC_DITHER,
};

// constructor
Halftone::Halftone()
	: FilterObject(FILTER_HALFTONE),
	  fMode(HALFTONE_ROUND_DOT)
{
	_InitPatterns();
}

// copy constructor
Halftone::Halftone(const Halftone& other)
	: FilterObject(other),
	  fMode(other.fMode)
{
	_InitPatterns();
}

// BArchivable constructor
Halftone::Halftone(BMessage* archive)
	: FilterObject(archive),
	  fMode(HALFTONE_ROUND_DOT)
{
	if (archive) {
		if (archive->FindInt32("halftone mode", &fMode) < B_OK)
			fMode = HALFTONE_ROUND_DOT;
	} else {
		SetFilterID(FILTER_HALFTONE);
	}
	_InitPatterns();
}

// destructor
Halftone::~Halftone()
{
}

// Clone
Stroke*
Halftone::Clone() const
{
	return new Halftone(*this);
}

// SetTo
bool
Halftone::SetTo(const Stroke* from)
{
	const Halftone* halftone = dynamic_cast<const Halftone*>(from);

	AutoNotificationSuspender _(this);

	if (halftone && FilterObject::SetTo(from)) {
		fMode = halftone->fMode;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
Halftone::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "Halftone"))
		return new Halftone(archive);
	return NULL;
}

// Archive
status_t
Halftone::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddInt32("halftone mode", fMode);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Halftone");

	return status;
}

// ExtendRebuildArea
void
Halftone::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	area = layerBounds;
}

// ProcessBitmap
void
Halftone::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	// TODO: this function is broken if area != dest->Bounds()
	// (but it wouldn't make sense if we didn't blur the entire bitmap)
	if (dest && dest->IsValid()) {

		uint32* bits = (uint32*)dest->Bits();
		uint32 ppl = dest->BytesPerRow() / 4;
		
		int32 left = (int32)dest->Bounds().left;
		int32 top = (int32)dest->Bounds().top;
		int32 right = (int32)dest->Bounds().right + 1;
		int32 bottom = (int32)dest->Bounds().bottom + 1;

		color c1, c2;

		c1.word = 0;
		
		rgb_color c = Color();
		c2.bytes[0] = c.blue;
		c2.bytes[1] = c.green;
		c2.bytes[2] = c.red;
		c2.bytes[3] = c.alpha;

		switch (fMode) {
			case HALFTONE_ROUND_DOT:
				round_dot_halftone(bits, ppl, left, top, right, bottom, c1, c2);
				break;
			case HALFTONE_DIAGONAL_LINE:
				diagonal_line_halftone(bits, ppl, left, top, right, bottom, c1, c2);
				break;
			case HALFTONE_DITHER:
				ordered_dither_halftone(bits, ppl, left, top, right, bottom, c1, c2);
				break;
			case HALFTONE_FS_DITHER:
				fs_dither_halftone(bits, ppl, left, top, right, bottom, c1, c2);
				break;
			case HALFTONE_NC_DITHER:
				ncandidate_dither_halftone(bits, ppl, left, top, right, bottom, c1, c2);
				break;
		}
	}
}

// MakePropertyObject
PropertyObject*
Halftone::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();

	if (object) {
		OptionProperty* property = new OptionProperty("mode",
													  PROPERTY_HALFTONE_MODE);
		LanguageManager* m = LanguageManager::Default();
		property->AddOption(HALFTONE_ROUND_DOT, m->GetString(DOT, "Dot"));
		property->AddOption(HALFTONE_DIAGONAL_LINE, m->GetString(LINE, "Line"));
		property->AddOption(HALFTONE_DITHER, m->GetString(DITHER, "Dither"));
		property->AddOption(HALFTONE_FS_DITHER, m->GetString(DITHER_FS, "Dither (FS)"));
		property->AddOption(HALFTONE_NC_DITHER, m->GetString(DITHER_NC, "Dither (NC)"));

		property->SetCurrentOptionID(fMode);
	
		object->AddProperty(property);
	}

	return object;
}

// SetToPropertyObject
bool
Halftone::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = Stroke::SetToPropertyObject(object);
	if (object) {
		OptionProperty* property = dynamic_cast<OptionProperty*>(object->FindProperty(PROPERTY_HALFTONE_MODE));
		if (property) {
			int32 mode = property->CurrentOptionID();
			if (mode != fMode)
				ret = true;
			fMode = mode;
		}

		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

// _InitPatterns
void
Halftone::_InitPatterns()
{
	round_dot_pattern[0][0] = 1;	
	round_dot_pattern[0][1] = 8;	
	round_dot_pattern[0][2] = 16;	
	round_dot_pattern[0][3] = 29;	
	round_dot_pattern[0][4] = 25;	
	round_dot_pattern[0][5] = 22;	
	round_dot_pattern[0][6] = 6;	
	round_dot_pattern[0][7] = 2;	

	round_dot_pattern[1][0] = 5;	
	round_dot_pattern[1][1] = 12;	
	round_dot_pattern[1][2] = 33;	
	round_dot_pattern[1][3] = 42;	
	round_dot_pattern[1][4] = 46;	
	round_dot_pattern[1][5] = 38;	
	round_dot_pattern[1][6] = 13;	
	round_dot_pattern[1][7] = 9;	

	round_dot_pattern[2][0] = 21;	
	round_dot_pattern[2][1] = 37;	
	round_dot_pattern[2][2] = 49;	
	round_dot_pattern[2][3] = 58;	
	round_dot_pattern[2][4] = 54;	
	round_dot_pattern[2][5] = 50;	
	round_dot_pattern[2][6] = 34;	
	round_dot_pattern[2][7] = 17;	

	round_dot_pattern[3][0] = 24;	
	round_dot_pattern[3][1] = 45;	
	round_dot_pattern[3][2] = 53;	
	round_dot_pattern[3][3] = 62;	
	round_dot_pattern[3][4] = 63;	
	round_dot_pattern[3][5] = 59;	
	round_dot_pattern[3][6] = 43;	
	round_dot_pattern[3][7] = 30;	

	round_dot_pattern[4][0] = 28;	
	round_dot_pattern[4][1] = 41;	
	round_dot_pattern[4][2] = 57;	
	round_dot_pattern[4][3] = 61;	
	round_dot_pattern[4][4] = 60;	
	round_dot_pattern[4][5] = 55;	
	round_dot_pattern[4][6] = 47;	
	round_dot_pattern[4][7] = 26;	

	round_dot_pattern[5][0] = 19;	
	round_dot_pattern[5][1] = 32;	
	round_dot_pattern[5][2] = 48;	
	round_dot_pattern[5][3] = 52;	
	round_dot_pattern[5][4] = 56;	
	round_dot_pattern[5][5] = 51;	
	round_dot_pattern[5][6] = 39;	
	round_dot_pattern[5][7] = 23;	

	round_dot_pattern[6][0] = 11;	
	round_dot_pattern[6][1] = 15;	
	round_dot_pattern[6][2] = 36;	
	round_dot_pattern[6][3] = 44;	
	round_dot_pattern[6][4] = 40;	
	round_dot_pattern[6][5] = 35;	
	round_dot_pattern[6][6] = 14;	
	round_dot_pattern[6][7] = 7;	

	round_dot_pattern[7][0] = 0;	
	round_dot_pattern[7][1] = 4;	
	round_dot_pattern[7][2] = 20;	
	round_dot_pattern[7][3] = 27;	
	round_dot_pattern[7][4] = 31;	
	round_dot_pattern[7][5] = 18;	
	round_dot_pattern[7][6] = 10;	
	round_dot_pattern[7][7] = 3;	


	diagonal_line_pattern[2][2] = 24;
	diagonal_line_pattern[1][3] = 23;
	diagonal_line_pattern[3][1] = 22;
	diagonal_line_pattern[0][4] = 21;
	diagonal_line_pattern[4][0] = 20;
	
	diagonal_line_pattern[1][0] = 19;
	diagonal_line_pattern[0][1] = 18;

	diagonal_line_pattern[4][3] = 17;
	diagonal_line_pattern[3][4] = 16;

	diagonal_line_pattern[1][1] = 15;
	diagonal_line_pattern[0][2] = 14;
	diagonal_line_pattern[2][0] = 13;

	diagonal_line_pattern[3][3] = 12;
	diagonal_line_pattern[2][4] = 11;
	diagonal_line_pattern[4][2] = 10;

	diagonal_line_pattern[2][1] = 9;
	diagonal_line_pattern[1][2] = 8;
	diagonal_line_pattern[3][0] = 7;
	diagonal_line_pattern[0][3] = 6;

	diagonal_line_pattern[3][2] = 5;
	diagonal_line_pattern[2][3] = 4;
	diagonal_line_pattern[4][1] = 3;
	diagonal_line_pattern[1][4] = 2;

	diagonal_line_pattern[0][0] = 1;
	diagonal_line_pattern[4][4] = 0;

	ordered_matrix[0][0] = 0;
	ordered_matrix[0][1] = 8;
	ordered_matrix[0][2] = 2;
	ordered_matrix[0][3] = 10;

	ordered_matrix[1][0] = 12;
	ordered_matrix[1][1] = 4;
	ordered_matrix[1][2] = 14;
	ordered_matrix[1][3] = 16;

	ordered_matrix[2][0] = 3;
	ordered_matrix[2][1] = 11;
	ordered_matrix[2][2] = 1;
	ordered_matrix[2][3] = 9;

	ordered_matrix[3][0] = 15;
	ordered_matrix[3][1] = 7;
	ordered_matrix[3][2] = 13;
	ordered_matrix[3][3] = 5;
}

static	const	int32	round_dot_size		= ROUND_DOT_SIZE;
static	const	int32	diagonal_line_size	= DIAGONAL_LINE_SIZE;
static	const	int32	ordered_matrix_size	= ORDERED_MATRIX_SIZE;

// round_dot_halftone
void
Halftone::round_dot_halftone(uint32* bits, uint32 ppl,
							 int32 left, int32 top, int32 right, int32 bottom,
							 color c1, color c2) const
{
	color c;
	float normalizer = 1.0/255.0*round_dot_size*round_dot_size;
	for (int32 y=top;y<bottom;y+=round_dot_size) {
		for (int32 x=left;x<right;x+=round_dot_size) {
			int32 r =min_c(x+round_dot_size,right);
			int32 b = min_c(y+round_dot_size,bottom);
			uint32 *s_delta_bits;
			
			for (int32 dy=y;dy<b;dy++) {
				s_delta_bits = bits + dy*ppl + x;
				for (int32 dx=x;dx<r;dx++) {
					c.word = *s_delta_bits;
c2.bytes[3] = c.bytes[3];
					float luminance = c.bytes[0] * .114 + c.bytes[1]*.587 + c.bytes[2]*.299;						
					int threshold = (int)(luminance * normalizer);
					*s_delta_bits++ = ((int)(round_dot_pattern[dy-y][dx-x]) > threshold ? c1.word : c2.word);
				}
			}	
		}
	}
}

// diagonal_line_halftone
void
Halftone::diagonal_line_halftone(uint32* bits, uint32 ppl,
								 int32 left, int32 top, int32 right, int32 bottom,
								 color c1, color c2) const
{
	color c;
	float normalizer = 1.0/255.0*diagonal_line_size*diagonal_line_size;
	for (int32 y=top;y<bottom;y+=diagonal_line_size) {
		for (int32 x=left;x<right;x+=diagonal_line_size) {
			int32 r =min_c(x+diagonal_line_size,right);
			int32 b = min_c(y+diagonal_line_size,bottom);
			uint32 *s_delta_bits;
			
			for (int32 dy=y;dy<b;dy++) {
				s_delta_bits = bits + dy*ppl + x;
				for (int32 dx=x;dx<r;dx++) {
					c.word = *s_delta_bits;
c2.bytes[3] = c.bytes[3];
					float luminance = c.bytes[0] * .114 + c.bytes[1]*.587 + c.bytes[2]*.299;						
					int threshold = (int)(luminance * normalizer);
					*s_delta_bits++ = ((int)(diagonal_line_pattern[dy-y][dx-x]) > threshold ? c1.word : c2.word);
				}
			}	
		}
	}
}

// ordered_dither_halftone
void
Halftone::ordered_dither_halftone(uint32* bits, uint32 ppl,
								  int32 left, int32 top, int32 right, int32 bottom,
								  color c1, color c2) const
{
	color c;
	float normalizer = 1.0/255.0*ordered_matrix_size*ordered_matrix_size;
	for (int32 y=top;y<bottom;y+=ordered_matrix_size) {
		for (int32 x=left;x<right;x+=ordered_matrix_size) {
			int32 r =min_c(x+ordered_matrix_size,right);
			int32 b = min_c(y+ordered_matrix_size,bottom);
			uint32 *s_delta_bits;
			
			for (int32 dy=y;dy<b;dy++) {
				s_delta_bits = bits + dy*ppl + x;
				for (int32 dx=x;dx<r;dx++) {
					c.word = *s_delta_bits;
c2.bytes[3] = c.bytes[3];
					float luminance = c.bytes[0] * .114 + c.bytes[1]*.587 + c.bytes[2]*.299;						
					int threshold = (int)(luminance * normalizer);
					*s_delta_bits++ = ((int)(ordered_matrix[dy-y][dx-x]) > threshold ? c1.word : c2.word);
				}
			}	
		}
	}
}

// fs_dither_halftone
void
Halftone::fs_dither_halftone(uint32* bits, uint32 ppl,
							 int32 left, int32 top, int32 right, int32 bottom,
							 color c1, color c2) const
{
	color c;
	float *errors = new float[right-left+3];
	for (int32 i=0;i<right-left+3;i++)
		errors[i] = 0;
		
	float right_error = 0;

	for (int32 y=top;y<bottom;y++) {
		for (int32 x=left;x<right;x++) {
			c.word = *bits;
c2.bytes[3] = c.bytes[3];
			float threshold = c.bytes[0] * .114 + c.bytes[1]*.587 + c.bytes[2]*.299;
			float value = min_c(255,max_c(threshold+right_error+errors[x+1],0));
			errors[x+1] = 0;
			right_error = 0;
			float error;
			if (value > 127) {
				error = -(255 - value);
				*bits++ = c2.word;
			}
			else {
				error = -(0 - value);
				*bits++ = c1.word;
			}
			right_error = .4375 * error;
			errors[x] += .1875 * error;
			errors[x+1] += .3125 * error;
			errors[x+2] += .0625 * error;					 				
		}
	}
}

// ncandidate_dither_halftone
void
Halftone::ncandidate_dither_halftone(uint32* bits, uint32 ppl,
									 int32 left, int32 top, int32 right, int32 bottom,
									 color c1, color c2) const
{
	color c;
	float *errors = new float[right-left+3];
	for (int32 i=0;i<right-left+3;i++)
		errors[i] = 0;
		
	float probs[256];
	for (int32 i=0;i<256;i++) {
		probs[i] = (float)i/256.0;	// probability to get white
	}

	RandomNumberGenerator generator(1027,1000000);	

	for (int32 y=top;y<bottom;y++) {
		for (int32 x=left;x<right;x++) {
			c.word = *bits;
c2.bytes[3] = c.bytes[3];
			int32 threshold = (int32)(c.bytes[0] * .114 + c.bytes[1]*.587 + c.bytes[2]*.299);
			float r = generator.UniformDistribution(0.0,1.0);
			if (probs[threshold] >= r)
				*bits++ = c2.word;
			else
				*bits++ = c1.word;
		}
	}
}

