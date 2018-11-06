// CommonPropertyIDs.cpp

#include "CommonPropertyIDs.h"
#include "LanguageManager.h"

// name_for_id
const char*
name_for_id(int32 id)
{
	LanguageManager* m = LanguageManager::Default();
	const char* name = NULL;
	switch (id) {
		case PROPERTY_OPACITY:
			name = m->GetString(ALPHA, "Opacity");
			break;
		case PROPERTY_MIN_OPACITY:
			name = m->GetString(MIN_OPACITY, "min Op.");
			break;
		case PROPERTY_BLENDING_MODE:
			name = m->GetString(MODE, "Mode");
			break;
		case PROPERTY_COLOR:
			name = m->GetString(COLOR, "Color");
			break;
		case PROPERTY_GRADIENT:
			name = m->GetString(GRADIENT, "Gradient");
			break;
		case PROPERTY_RADIUS:
			name = m->GetString(RADIUS, "Radius");
			break;
		case PROPERTY_MIN_RADIUS:
			name = m->GetString(MIN_RADIUS, "min Radius");
			break;
		case PROPERTY_HARDNESS:
			name = m->GetString(HARDNESS, "Hardness");
			break;
		case PROPERTY_MIN_HARDNESS:
			name = m->GetString(MIN_HARDNESS, "min Hardn.");
			break;
		case PROPERTY_NO_ANTIALIASING:
			name = m->GetString(SOLID, "Solid");
			break;
		case PROPERTY_SPACING:
			name = m->GetString(SPACING, "Spacing");
			break;
		case PROPERTY_PRESSURE_CONTROLS_OPACITY:
			name = m->GetString(DYN_OPACITY, "dyn. Op.");
			break;
		case PROPERTY_PRESSURE_CONTROLS_RADIUS:
			name = m->GetString(DYN_RADIUS, "dyn. Radius");
			break;
		case PROPERTY_PRESSURE_CONTROLS_HARDNESS:
			name = m->GetString(DYN_HARDNESS, "dyn. Hardn.");
			break;
		case PROPERTY_PRESSURE_CONTROLS_SPACING:
			name = m->GetString(DYN_SPACING, "dyn. Spac.");
			break;
		case PROPERTY_TILT:
			name = m->GetString(TILT, "Tilt");
			break;
		case PROPERTY_BLUR_RADIUS:
			name = m->GetString(BLUR_RADIUS, "Blur Radius");
			break;
		case PROPERTY_TEXT:
			name = m->GetString(TEXT_INPUT, "Text");
			break;
		case PROPERTY_FONT:
			name = m->GetString(FONT, "Font");
			break;
		case PROPERTY_FONT_SIZE:
			name = m->GetString(SIZE, "Size");
			break;
		case PROPERTY_FONT_X_SCALE:
			name = m->GetString(SPACING, "Spacing");
			break;
		case PROPERTY_FONT_LINE_SCALE:
			name = m->GetString(TEXT_LINE_SPACING, "Line Spacing");
			break;
		case PROPERTY_FONT_HINTING:
			name = m->GetString(HINTING, "Hinting");
			break;
		case PROPERTY_FONT_KERNING:
			name = m->GetString(KERNING, "Kerning");
			break;
		case PROPERTY_ALIGNMENT:
			name = m->GetString(ALIGNMENT, "Alignment");
			break;
		case PROPERTY_WIDTH:
			name = m->GetString(WIDTH, "Width");
			break;
		case PROPERTY_PARAGRAPH_INSET:
			name = m->GetString(PARAGRAPH_INSET, "Inset");
			break;
		case PROPERTY_PARAGRAPH_SPACING:
			name = m->GetString(PARAGRAPH_SPACING, "P. Spacing");
			break;
		case PROPERTY_PATH:
			name = m->GetString(PATH, "Path");
			break;
		case PROPERTY_OUTLINE:
			name = m->GetString(OUTLINE, "Outline");
			break;
		case PROPERTY_OUTLINE_WIDTH:
			name = m->GetString(WIDTH, "Width");
			break;
		case PROPERTY_CLOSED:
			name = m->GetString(CLOSED, "Closed");
			break;
		case PROPERTY_CAP_MODE:
			name = m->GetString(CAP_MODE, "Caps");
			break;
		case PROPERTY_JOIN_MODE:
			name = m->GetString(JOIN_MODE, "Joints");
			break;
		case PROPERTY_FILLING_RULE:
			name = m->GetString(FILLING_RULE, "Filling Rule");
			break;
		case PROPERTY_X_OFFSET:
			name = m->GetString(X_OFFSET, "X Offset");
			break;
		case PROPERTY_Y_OFFSET:
			name = m->GetString(Y_OFFSET, "Y Offset");
			break;
		case PROPERTY_BRIGHTNESS:
			name = m->GetString(BRIGHTNESS, "Brightness");
			break;
		case PROPERTY_CONTRAST:
			name = m->GetString(CONTRAST, "Contrast");
			break;
		case PROPERTY_SATURATION:
			name = m->GetString(SATURATION, "Saturation");
			break;
		case PROPERTY_ANGLE:
			name = m->GetString(ANGLE, "Angle");
			break;
		case PROPERTY_BLUR_ALPHA:
			name = m->GetString(ALPHA_CHANNEL, "Alpha channel");
			break;
		case PROPERTY_HALFTONE_MODE:
			name = m->GetString(MODE, "Modus");
			break;
		case PROPERTY_STRENGTH:
			name = m->GetString(STRENGTH, "Strength");
			break;
		case PROPERTY_LUMINANCE_ONLY:
			name = m->GetString(LUMINANCE_ONLY, "Luminance only");
			break;

		case PROPERTY_WARPSHARP_LAMBDA:
			name = m->GetString(LAMBDA, "Lambda");
			break;
		case PROPERTY_WARPSHARP_MU:
			name = m->GetString(MU, "Mu");
			break;
		case PROPERTY_WARPSHARP_NON_MAX_SUPR:
			name = m->GetString(NON_MAXIMAL_SUPPRESSION, "NMS");
			break;

		case PROPERTY_BITMAP_INTERPOLATION:
			name = m->GetString(INTERPOLATION, "Interpolation");
			break;

		case PROPERTY_CIMG_AMPLITUDE:
			name = m->GetString(AMPLITUDE, "Amplitude");
			break;
		case PROPERTY_CIMG_SHARPNESS:
			name = m->GetString(SHARPNESS, "Sharpness");
			break;
		case PROPERTY_CIMG_ANISOTROPY:
			name = m->GetString(ANISOTROPY, "Anisotropy");
			break;
		case PROPERTY_CIMG_ALPHA:
			name = m->GetString(ALPHA, "Alpha");
			break;
		case PROPERTY_CIMG_SIGMA:
			name = m->GetString(SIGMA, "Sigma");
			break;
		case PROPERTY_CIMG_DL:
			name = m->GetString(CIMG_DL, "dl");
			break;
		case PROPERTY_CIMG_DA:
			name = m->GetString(CIMG_DA, "da");
			break;
		case PROPERTY_CIMG_GAUSS_PRECISION:
			name = m->GetString(GAUSS_PRECISION, "Gauss Prec.");
			break;
		case PROPERTY_CIMG_INTERPOLATION:
			name = m->GetString(INTERPOLATION, "Interpolation");
			break;
		case PROPERTY_CIMG_FAST_APPROXIMATION:
			name = m->GetString(FAST_APPROXIMATION, "Fast Approx.");
			break;

		default:
			name = m->GetString(UNKOWN_PROPERTY, "<unkown>");
			break;
	}
	return name;
}

