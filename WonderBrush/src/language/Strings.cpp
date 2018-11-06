// Strings.cpp

#include "Strings.h"

void
print_header (FILE* file)
{
	fprintf(file, "#include <stdio.h>\n\n");
	fprintf(file, "enum {\n");
	
	fprintf(file, "FILE_MENU	= 0,\n");
	fprintf(file, "NEW,\n");
	fprintf(file, "NEW_CANVAS,\n");
	fprintf(file, "NEW_LAYER,\n");
	fprintf(file, "NEW_BRUSH,\n");
	fprintf(file, "NEW_PALETTE,\n");
	fprintf(file, "LOAD,\n");
	fprintf(file, "OPEN,\n");
	fprintf(file, "OPEN_CANVAS,\n");

	fprintf(file, "EXPORT_CANVAS,\n");
	fprintf(file, "EXPORT,\n");
	fprintf(file, "EXPORT_AS,\n");
	fprintf(file, "SAVE_CANVAS,\n");
	fprintf(file, "SAVE,\n");
	fprintf(file, "SAVE_AS,\n");
	fprintf(file, "SETUP_PAGE,\n");
	fprintf(file, "PRINT,\n");
	fprintf(file, "CLOSE,\n");
	fprintf(file, "CLOSE_CANVAS,\n");
	fprintf(file, "ABOUT,\n");
	fprintf(file, "QUIT,\n");

	fprintf(file, "LOADING_FILE,\n");

	fprintf(file, "EDIT,\n");
	fprintf(file, "UNDO,\n");
	fprintf(file, "REDO,\n");

	fprintf(file, "SETTINGS,\n");
	fprintf(file, "FULLSCREEN,\n");
	fprintf(file, "SHOW_GRID,\n");
	fprintf(file, "LANGUAGE,\n");

	fprintf(file, "CANVAS,\n");
	fprintf(file, "IMAGE,\n");
	fprintf(file, "CLEAR,\n");
	fprintf(file, "RESIZE,\n");
	fprintf(file, "ROTATE_90,\n");
	fprintf(file, "ROTATE_180,\n");
	fprintf(file, "ROTATE_270,\n");

	fprintf(file, "FORMAT,\n");

	fprintf(file, "EXPORT_FORMAT,\n");
	fprintf(file, "CURSOR_SOURCE,\n");
	fprintf(file, "BITMAP_SOURCE,\n");
	fprintf(file, "SVG_FORMAT,\n");
	fprintf(file, "AI_FORMAT,\n");
	fprintf(file, "RDEF_FORMAT,\n");
	fprintf(file, "RDEF_ASK_IF_ICON,\n");
	fprintf(file, "RDEF_NORMAL,\n");
	fprintf(file, "RDEF_ICON,\n");


	fprintf(file, "LAYER,\n");
	fprintf(file, "DUPLICATE,\n");
	fprintf(file, "MERGE_DOWN,\n");
	fprintf(file, "MERGE_VISIBLE,\n");
	fprintf(file, "RENAME,\n");
	fprintf(file, "DELETE,\n");
	fprintf(file, "DELETE_ALL,\n");

	fprintf(file, "MODE,\n");
	fprintf(file, "NORMAL,\n");
	fprintf(file, "ERASE,\n");
	fprintf(file, "MULTIPLY,\n");
	fprintf(file, "INVERSE_MULTIPLY,\n");
	fprintf(file, "LUMINANCE,\n");
	fprintf(file, "MULTIPLY_ALPHA,\n");
	fprintf(file, "MULTIPLY_INVERSE_ALPHA,\n");
	fprintf(file, "REPLACE_RED,\n");
	fprintf(file, "REPLACE_GREEN,\n");
	fprintf(file, "REPLACE_BLUE,\n");
	fprintf(file, "DARKEN,\n");
	fprintf(file, "LIGHTEN,\n");

	fprintf(file, "HISTORY,\n");
	fprintf(file, "OBJECT,\n");
	fprintf(file, "FREEZE,\n");
	fprintf(file, "CUT,\n");
	fprintf(file, "COPY,\n");
	fprintf(file, "PASTE,\n");
	fprintf(file, "MULTI_PASTE,\n");
	fprintf(file, "REMOVE_ALL,\n");
	fprintf(file, "OBJECT_SELECTION,\n");
	fprintf(file, "SELECT_ALL_OBJECTS,\n");
	fprintf(file, "SELECT_NO_OBJECTS,\n");
	fprintf(file, "INVERT_SELECTION,\n");
	fprintf(file, "ALL_OBJECTS,\n");
	fprintf(file, "PROPERTY_SELECTION,\n");
	fprintf(file, "SELECT_ALL_PROPERTIES,\n");
	fprintf(file, "SELECT_NO_PROPERTIES,\n");
	fprintf(file, "SPECIAL,\n");

	fprintf(file, "ADD_FILTER,\n");
	fprintf(file, "DROP_SHADOW,\n");
	fprintf(file, "GAUSSIAN_BLUR,\n");
	fprintf(file, "INVERT,\n");
	fprintf(file, "BRIGHTNESS,\n");
	fprintf(file, "CONTRAST,\n");
	fprintf(file, "SATURATION,\n");
	fprintf(file, "MOTION_BLUR,\n");
	fprintf(file, "HALFTONE,\n");
	fprintf(file, "DOT,\n");
	fprintf(file, "LINE,\n");
	fprintf(file, "DITHER,\n");
	fprintf(file, "DITHER_FS,\n");
	fprintf(file, "DITHER_NC,\n");
	fprintf(file, "NOISE,\n");
	fprintf(file, "HUE_SHIFT,\n");
	fprintf(file, "DISPERSION,\n");
	fprintf(file, "STRENGTH,\n");
	fprintf(file, "LUMINANCE_ONLY,\n");
	fprintf(file, "WARPSHARP,\n");
	fprintf(file, "LAMBDA,\n");
	fprintf(file, "MU,\n");
	fprintf(file, "NON_MAXIMAL_SUPPRESSION,\n");
	fprintf(file, "SOLID_AREA,\n");
	fprintf(file, "COLOR_REDUCTION,\n");

	fprintf(file, "DE_NOISE,\n");
	fprintf(file, "AMPLITUDE,\n");
	fprintf(file, "SHARPNESS,\n");
	fprintf(file, "ANISOTROPY,\n");
	fprintf(file, "SIGMA,\n");
	fprintf(file, "CIMG_DL,\n");
	fprintf(file, "CIMG_DA,\n");
	fprintf(file, "GAUSS_PRECISION,\n");
	fprintf(file, "FAST_APPROXIMATION,\n");

	fprintf(file, "PROPERTY,\n");
	fprintf(file, "UNKOWN_PROPERTY,\n");

	fprintf(file, "SWATCHES,\n");
	fprintf(file, "ZOOM,\n");
	fprintf(file, "ALPHA,\n");
	fprintf(file, "ALPHA_CHANNEL,\n");
	fprintf(file, "COLOR,\n");
	fprintf(file, "GRADIENT,\n");

	fprintf(file, "SELECTABLE,\n");

	fprintf(file, "GRADIENT_TYPE,\n");
	fprintf(file, "GRADIENT_TYPE_LINEAR,\n");
	fprintf(file, "GRADIENT_TYPE_RADIAL,\n");
	fprintf(file, "GRADIENT_TYPE_DIAMONT,\n");
	fprintf(file, "GRADIENT_TYPE_CONIC,\n");
	fprintf(file, "GRADIENT_TYPE_XY,\n");
	fprintf(file, "GRADIENT_TYPE_XY_SQRT,\n");
	fprintf(file, "GRADIENT_INTERPOLATION,\n");
	fprintf(file, "GRADIENT_INTERPOLATION_LINEAR,\n");
	fprintf(file, "GRADIENT_INTERPOLATION_SMOOTH,\n");
	fprintf(file, "GRADIENT_INHERITS_TRANSFORM,\n");

	fprintf(file, "OPACITY,\n");
	fprintf(file, "MIN_OPACITY,\n");
	fprintf(file, "DYN_OPACITY,\n");
	fprintf(file, "RADIUS,\n");
	fprintf(file, "MIN_RADIUS,\n");
	fprintf(file, "DYN_RADIUS,\n");
	fprintf(file, "HARDNESS,\n");
	fprintf(file, "MIN_HARDNESS,\n");
	fprintf(file, "DYN_HARDNESS,\n");
	fprintf(file, "SPACING,\n");
	fprintf(file, "MIN_SPACING,\n");
	fprintf(file, "DYN_SPACING,\n");
	fprintf(file, "SUBPIXELS,\n");
	fprintf(file, "SOLID,\n");
	fprintf(file, "TILT,\n");
	fprintf(file, "BLUR_RADIUS,\n");

	fprintf(file, "X_LABEL,\n");
	fprintf(file, "Y_LABEL,\n");
	fprintf(file, "X_SCALE,\n");
	fprintf(file, "Y_SCALE,\n");
	fprintf(file, "TRANSLATION_X,\n");
	fprintf(file, "TRANSLATION_Y,\n");
	fprintf(file, "SCALE_X,\n");
	fprintf(file, "SCALE_Y,\n");
	fprintf(file, "ANGLE,\n");

	fprintf(file, "X_OFFSET,\n");
	fprintf(file, "Y_OFFSET,\n");

	fprintf(file, "LEFT,\n");
	fprintf(file, "TOP,\n");
	fprintf(file, "ENTIRE_CANVAS,\n");

	fprintf(file, "TOLERANCE,\n");
	fprintf(file, "SOFTNESS,\n");
	fprintf(file, "CONTIGUOUS_AREA,\n");

	fprintf(file, "PATH,\n");
	fprintf(file, "OUTLINE,\n");
	fprintf(file, "TRANSFORM,\n");
	fprintf(file, "CLOSED,\n");
	fprintf(file, "CAP_MODE,\n");
	fprintf(file, "BUTT_CAP,\n");
	fprintf(file, "SQUARE_CAP,\n");
	fprintf(file, "ROUND_CAP,\n");
	fprintf(file, "JOIN_MODE,\n");
	fprintf(file, "MITER_JOIN,\n");
	fprintf(file, "ROUND_JOIN,\n");
	fprintf(file, "BEVEL_JOIN,\n");
	fprintf(file, "CIRCLE,\n");
	fprintf(file, "RECT,\n");
	fprintf(file, "ROUND_CORNERS,\n");
	fprintf(file, "TOGGLE_ROUND_CORNERS,\n");
	fprintf(file, "ROUND_CORNER_RADIUS,\n");
	fprintf(file, "CHANGE_ROUND_CORNER_RADIUS,\n");
	fprintf(file, "REVERSE_PATH,\n");

	fprintf(file, "FONT,\n");
	fprintf(file, "SIZE,\n");
	fprintf(file, "ROTATION,\n");
	fprintf(file, "TEXT_INPUT,\n");
	fprintf(file, "ADVANCE_SCALE,\n");
	fprintf(file, "HINTING,\n");
	fprintf(file, "KERNING,\n");
	fprintf(file, "TEXT_LINE_SPACING,\n");

	fprintf(file, "TIP_SIZE,\n");
	fprintf(file, "PIXEL_1x1,\n");
	fprintf(file, "PIXEL_3x3,\n");
	fprintf(file, "PIXEL_5x5,\n");
	fprintf(file, "INCLUDE_ALL_LAYERS,\n");

	fprintf(file, "BRUSH,\n");
	fprintf(file, "CLONE,\n");
	fprintf(file, "ERASER,\n");
	fprintf(file, "PEN,\n");
	fprintf(file, "PEN_ERASER,\n");
	fprintf(file, "RESTORE,\n");
	fprintf(file, "DROPPER,\n");
	fprintf(file, "TEXT,\n");
	fprintf(file, "BLUR,\n");
	fprintf(file, "BITMAP,\n");
	fprintf(file, "SELECT,\n");
	fprintf(file, "TRANSLATE,\n");
	fprintf(file, "CROP,\n");
	fprintf(file, "BUCKET_FILL,\n");
	fprintf(file, "SHAPE,\n");
	fprintf(file, "ELLIPSE,\n");
	fprintf(file, "ROUND_RECT,\n");
	fprintf(file, "EDIT_GRADIENT,\n");
	fprintf(file, "GUIDES,\n");
	fprintf(file, "PICK_OBJECTS,\n");
	fprintf(file, "CLIPBOARD,\n");

	fprintf(file, "ZOOM_IN,\n");
	fprintf(file, "ZOOM_OUT,\n");

	fprintf(file, "DELETE_KEY,\n");
	fprintf(file, "DELETE_LAYER,\n");
	fprintf(file, "DELETE_OBJECTS,\n");

	fprintf(file, "ADD_LAYER,\n");
	fprintf(file, "REMOVE_LAYER,\n");
	fprintf(file, "REMOVE_LAYERS,\n");
	fprintf(file, "MOVE_LAYER,\n");
	fprintf(file, "MOVE_LAYERS,\n");
	fprintf(file, "MERGE_LAYERS,\n");
	fprintf(file, "UNKOWN_MODIFIER,\n");
	fprintf(file, "ADD_MODIFIER,\n");
	fprintf(file, "ADD_MODIFIERS,\n");
	fprintf(file, "CHANGE_COLOR,\n");
	fprintf(file, "CHANGE_COLORS,\n");
	fprintf(file, "REMOVE_MODIFIER,\n");
	fprintf(file, "REMOVE_MODIFIERS,\n");
	fprintf(file, "MOVE_MODIFIER,\n");
	fprintf(file, "MOVE_MODIFIERS,\n");
	fprintf(file, "TRANSLATE_MODIFIER,\n");
	fprintf(file, "TRANSLATE_MODIFIERS,\n");
	fprintf(file, "CHANGE_LAYER_SETTINGS,\n");
	fprintf(file, "CROP_CANVAS,\n");
	fprintf(file, "RESIZE_CANVAS,\n");
	fprintf(file, "ROTATE_CANVAS,\n");
	fprintf(file, "INSERT_CANVAS,\n");
	fprintf(file, "CHANGE_OPACITY,\n");
	fprintf(file, "EDIT_MODIFIER,\n");

	fprintf(file, "PASTE_PROPERTY,\n");
	fprintf(file, "PASTE_PROPERTIES,\n");
	fprintf(file, "MULTI_PASTE_PROPERTY,\n");
	fprintf(file, "MUTLI_PASTE_PROPERTIES,\n");

	fprintf(file, "ADD_CONTROL_POINT,\n");
	fprintf(file, "TOGGLE_OUTLINE,\n");
	fprintf(file, "CHANGE_OUTLINE_WIDTH,\n");
	fprintf(file, "CHANGE_CAP_MODE,\n");
	fprintf(file, "CHANGE_JOIN_MODE,\n");
	fprintf(file, "MODIFY_CONTROL_POINT,\n");
	fprintf(file, "CLOSE_PATH_ACTION,\n");
	fprintf(file, "INSERT_CONTROL_POINT,\n");
	fprintf(file, "REMOVE_CONTROL_POINT,\n");
	fprintf(file, "REMOVE_CONTROL_POINTS,\n");
	fprintf(file, "CHANGE_POINT_SELECTION,\n");
	fprintf(file, "NUDGE_CONTROL_POINT,\n");
	fprintf(file, "NUDGE_CONTROL_POINTS,\n");
	fprintf(file, "SELECT_PATH,\n");
	fprintf(file, "CREATE_NEW_PATH,\n");
	fprintf(file, "DELETE_PATH,\n");
	fprintf(file, "TEXT_TO_SHAPE,\n");
	fprintf(file, "TEXTS_TO_SHAPES,\n");
	fprintf(file, "TRANSFORM_CONTROL_POINTS,\n");
	fprintf(file, "MERGE_SHAPE_OBJECTS,\n");

	fprintf(file, "FILLING_RULE,\n");
	fprintf(file, "NON_ZERO,\n");
	fprintf(file, "EVEN_ODD,\n");

	fprintf(file, "CHANGE_FONT_SIZE,\n");
	fprintf(file, "CHANGE_TEXT_ROTATION,\n");
	fprintf(file, "MOVE_TEXT,\n");
	fprintf(file, "CHANGE_FONT,\n");
	fprintf(file, "CHANGE_TEXT,\n");
	fprintf(file, "CHANGE_FONT_ADVANCE_SCALE,\n");
	fprintf(file, "ALIGNMENT,\n");
	fprintf(file, "CHANGE_ALIGNMENT,\n");
	fprintf(file, "ALIGNMENT_LEFT,\n");
	fprintf(file, "ALIGNMENT_CENTER,\n");
	fprintf(file, "ALIGNMENT_RIGHT,\n");
	fprintf(file, "ALIGNMENT_JUSTIFY,\n");
	fprintf(file, "CHANGE_TEXT_WIDTH,\n");
	fprintf(file, "PARAGRAPH_INSET,\n");
	fprintf(file, "CHANGE_PARAGRAPH_INSET,\n");
	fprintf(file, "PARAGRAPH_SPACING,\n");

	fprintf(file, "MODIFY_CROP_RECT,\n");

	fprintf(file, "MODIFY_SELECT_RECT,\n");

	fprintf(file, "TRANSFORMATION,\n");
	fprintf(file, "BEGIN_TRANSFORMATION,\n");
	fprintf(file, "FINISH_TRANSFORMATION,\n");
	fprintf(file, "MOVE,\n");
	fprintf(file, "ROTATE,\n");
	fprintf(file, "SCALE,\n");
	fprintf(file, "MOVE_PIVOT,\n");
	fprintf(file, "TRANSFORM_OBJECT,\n");
	fprintf(file, "TRANSFORM_OBJECTS,\n");

	fprintf(file, "SHOW_GUIDES,\n");
	fprintf(file, "HORIZONTAL,\n");
	fprintf(file, "VERTICAL,\n");
	fprintf(file, "UNIT,\n");
	fprintf(file, "UNIT_PERCENT,\n");
	fprintf(file, "UNIT_PIXEL,\n");
	fprintf(file, "UNIT_CM,\n");
	fprintf(file, "UNIT_MM,\n");
	fprintf(file, "UNIT_INCH,\n");
	fprintf(file, "UNIT_DPI,\n");

	fprintf(file, "OK,\n");
	fprintf(file, "CONFIRM,\n");
	fprintf(file, "CANCEL,\n");
	fprintf(file, "REVERT,\n");
	fprintf(file, "REMOVE,\n");
	fprintf(file, "NAME,\n");
	fprintf(file, "UNNAMED,\n");
	fprintf(file, "DEFAULT,\n");
	fprintf(file, "MERGED,\n");
	fprintf(file, "TOOL,\n");
	fprintf(file, "ANY_MODIFIER,\n");
	fprintf(file, "RETURN,\n");
	fprintf(file, "ESCAPE,\n");
	fprintf(file, "SET,\n");
	fprintf(file, "COUNT,\n");
	fprintf(file, "INTERPOLATION,\n");
	fprintf(file, "NEAREST_NEIGHBOR,\n");
	fprintf(file, "BILINEAR,\n");
	fprintf(file, "LINEAR,\n");
	fprintf(file, "RESAMPLE,\n");
	fprintf(file, "RETRY,\n");
	fprintf(file, "OVERWRITE,\n");

	fprintf(file, "PICK_COLOR,\n");
	fprintf(file, "WIDTH,\n");
	fprintf(file, "HEIGHT,\n");
	fprintf(file, "BRUSH_PANEL,\n");
	fprintf(file, "BRUSH_NAME,\n");
	fprintf(file, "PROGRAM_SETTINGS,\n");

	fprintf(file, "ON_STARTUP,\n");
	fprintf(file, "DO_NOTHING,\n");
	fprintf(file, "ASK_OPEN,\n");
	fprintf(file, "ASK_NEW,\n");
	fprintf(file, "AUTO_SIZE,\n");
	fprintf(file, "LIVE_UPDATES,\n");
	fprintf(file, "USE_SYSTEM_LANGUAGE,\n");
	fprintf(file, "STARTUP_ACTION,\n");
	fprintf(file, "INTERFACE,\n");

	fprintf(file, "AKS_SAVE_CHANGES,\n");
	fprintf(file, "DISCARD,\n");
	fprintf(file, "SAVING_FAILED,\n");
	fprintf(file, "ERROR,\n");
	fprintf(file, "BLIP,\n");
	fprintf(file, "DEMO_MODE,\n");
	fprintf(file, "NEVER_MIND,\n");
	fprintf(file, "OPEN_TRANSLATIONKIT_ERROR,\n");
	fprintf(file, "OPEN_NATIVE_ERROR,\n");
	fprintf(file, "UNSUPPORTED_CURSOR_SIZE,\n");
	fprintf(file, "SELECTED_AREA_EMPTY,\n");
	fprintf(file, "ENTRY_DOES_NOT_EXIST,\n");
	fprintf(file, "ENTRY_IS_A_FOLDER,\n");
	fprintf(file, "WARN_SVG_DATA_LOSS,\n");

	fprintf(file, "TOOL_TIPS,\n");
	fprintf(file, "ALPHA_TIP,\n");
	fprintf(file, "VISIBILITY_TIP,\n");
	fprintf(file, "PRESSURE_CONTROL_TIP,\n");
	fprintf(file, "BRUSH_PREVIEW_TIP,\n");
	fprintf(file, "CURRENT_COLOR_TIP,\n");
	fprintf(file, "NAVIGATOR_TIP,\n");
	fprintf(file, "LAYER_LIST_TIP,\n");
	fprintf(file, "HISTORY_LIST_TIP,\n");
	fprintf(file, "PROPERTY_LIST_TIP,\n");
	fprintf(file, "SUB_PIXEL_TIP,\n");
	fprintf(file, "SOLID_TIP,\n");
	fprintf(file, "TILT_TIP,\n");
	fprintf(file, "FULLSCREEN_TIP,\n");
	fprintf(file, "GRADIENT_CONTROL_TIP,\n");
	fprintf(file, "GRADIENT_INHERITS_TRANSFORM_TIP,\n");

	fprintf(file, "READY,\n");
	fprintf(file, "NO_CANVAS_HELP,\n");
	fprintf(file, "NO_HELP,\n");
	fprintf(file, "CROP_HELP,\n");
	fprintf(file, "TRANSLATE_HELP,\n");
	fprintf(file, "EDIT_GRADIENT_HELP,\n");
	fprintf(file, "STROKE_HELP,\n");
	fprintf(file, "CLONE_HELP,\n");
	fprintf(file, "BUCKET_FILL_HELP,\n");
	fprintf(file, "TEXT_HELP,\n");
	fprintf(file, "SHAPE_HELP,\n");
	fprintf(file, "SELECT_HELP,\n");
	fprintf(file, "ELLIPSE_HELP,\n");
	fprintf(file, "ROUND_RECT_HELP,\n");
	fprintf(file, "PICK_OBJECTS_HELP,\n");
	fprintf(file, "GUIDES_HELP,\n");

	fprintf(file, "ENTER_SERIAL_NUMBER,\n");
	fprintf(file, "WRONG_SERIAL_NUMBER,\n");

	fprintf(file, "STRING_COUNT,\n");
	fprintf(file, "};\n");
};

// init_defines
void
init_defines(const char* strings[])
{
	strings[FILE_MENU]			= "FILE_MENU";
	strings[NEW]				= "NEW";
	strings[NEW_CANVAS]			= "NEW_CANVAS";
	strings[NEW_LAYER]			= "NEW_LAYER";
	strings[NEW_BRUSH]			= "NEW_BRUSH";
	strings[NEW_PALETTE]		= "NEW_PALETTE";
	strings[OPEN]				= "OPEN";
	strings[LOAD]				= "LOAD";
	strings[OPEN_CANVAS]		= "OPEN_CANVAS";
	strings[EXPORT_CANVAS]		= "EXPORT_CANVAS";
	strings[EXPORT]				= "EXPORT";
	strings[EXPORT_AS]			= "EXPORT_AS";
	strings[SAVE_CANVAS]		= "SAVE_CANVAS";
	strings[SAVE]				= "SAVE";
	strings[SAVE_AS]			= "SAVE_AS";
	strings[SETUP_PAGE]			= "SETUP_PAGE";
	strings[PRINT]				= "PRINT";
	strings[CLOSE]				= "CLOSE";
	strings[CLOSE_CANVAS]		= "CLOSE_CANVAS";
	strings[ABOUT]				= "ABOUT";
	strings[QUIT]				= "QUIT";

	strings[LOADING_FILE]		= "LOADING_FILE";

	strings[EDIT]				= "EDIT";
	strings[UNDO]				= "UNDO";
	strings[REDO]				= "REDO";

	strings[SETTINGS]			= "SETTINGS";
	strings[FULLSCREEN]			= "FULLSCREEN";
	strings[SHOW_GRID]			= "SHOW_GRID";

	strings[LANGUAGE]			= "LANGUAGE";
	strings[CANVAS]				= "CANVAS";
	strings[IMAGE]				= "IMAGE";
	strings[CLEAR]				= "CLEAR";
	strings[RESIZE]				= "RESIZE";
	strings[ROTATE_90]			= "ROTATE_90";
	strings[ROTATE_180]			= "ROTATE_180";
	strings[ROTATE_270]			= "ROTATE_270";

	strings[FORMAT]				= "FORMAT";

	strings[EXPORT_FORMAT]		= "EXPORT_FORMAT";
	strings[CURSOR_SOURCE]		= "CURSOR_SOURCE";
	strings[BITMAP_SOURCE]		= "BITMAP_SOURCE";
	strings[SVG_FORMAT]			= "SVG_FORMAT";
	strings[AI_FORMAT]			= "AI_FORMAT";
	strings[RDEF_FORMAT]		= "RDEF_FORMAT";

	strings[LAYER]				= "LAYER";
	strings[DUPLICATE]			= "DUPLICATE";
	strings[MERGE_DOWN]			= "MERGE_DOWN";

	strings[MERGE_VISIBLE]		= "MERGE_VISIBLE";
	strings[RENAME]				= "RENAME";
	strings[DELETE]				= "DELETE";
	strings[DELETE_ALL]			= "DELETE_ALL";

	// layer blending modes
	strings[MODE]				= "MODE";
	strings[NORMAL]				= "NORMAL";
	strings[ERASE]				= "ERASE";
	strings[MULTIPLY]			= "MULTIPLY";
	strings[INVERSE_MULTIPLY]	= "INVERSE_MULTIPLY";
	strings[LUMINANCE]			= "LUMINANCE";
	strings[MULTIPLY_ALPHA]		= "MULTIPLY_ALPHA";
	strings[MULTIPLY_INVERSE_ALPHA]	= "MULTIPLY_INVERSE_ALPHA";
	strings[REPLACE_RED]		= "REPLACE_RED";
	strings[REPLACE_GREEN]		= "REPLACE_GREEN";
	strings[REPLACE_BLUE]		= "REPLACE_BLUE";
	strings[DARKEN]				= "DARKEN";
	strings[LIGHTEN]			= "LIGHTEN";

	strings[HISTORY]			= "HISTORY";
	strings[OBJECT]				= "OBJECT";
	strings[FREEZE]				= "FREEZE";

	strings[CUT]				= "CUT";
	strings[COPY]				= "COPY";
	strings[PASTE]				= "PASTE";
	strings[MULTI_PASTE]		= "MULTI_PASTE";

	strings[REMOVE_ALL]			= "REMOVE_ALL";

	strings[OBJECT_SELECTION]	= "OBJECT_SELECTION";
	strings[SELECT_ALL_OBJECTS]	= "SELECT_ALL_OBJECTS";
	strings[SELECT_NO_OBJECTS]	= "SELECT_NO_OBJECTS";
	strings[INVERT_SELECTION]	= "INVERT_SELECTION";
	strings[ALL_OBJECTS]		= "ALL_OBJECTS";
	strings[PROPERTY_SELECTION]	= "PROPERTY_SELECTION";
	strings[SELECT_ALL_PROPERTIES] = "SELECT_ALL_PROPERTIES";
	strings[SELECT_NO_PROPERTIES] = "SELECT_NO_PROPERTIES";
	strings[SPECIAL]			= "SPECIAL";

	// filters
	strings[ADD_FILTER]			= "ADD_FILTER";
	strings[DROP_SHADOW]		= "DROP_SHADOW";
	strings[GAUSSIAN_BLUR]		= "GAUSSIAN_BLUR";
	strings[INVERT]				= "INVERT";
	strings[BRIGHTNESS]			= "BRIGHTNESS";
	strings[CONTRAST]			= "CONTRAST";
	strings[SATURATION]			= "SATURATION";
	strings[MOTION_BLUR]		= "MOTION_BLUR";
	strings[HALFTONE]			= "HALFTONE";
	strings[DOT]				= "DOT";
	strings[LINE]				= "LINE";
	strings[DITHER]				= "DITHER";
	strings[DITHER_FS]			= "DITHER_FS";
	strings[DITHER_NC]			= "DITHER_NC";
	strings[NOISE]				= "NOISE";
	strings[HUE_SHIFT]			= "HUE_SHIFT";
	strings[DISPERSION]			= "DISPERSION";
	strings[STRENGTH]			= "STRENGTH";
	strings[LUMINANCE_ONLY]		= "LUMINANCE_ONLY";
	strings[WARPSHARP]			= "WARPSHARP";
	strings[LAMBDA]				= "LAMBDA";
	strings[MU]					= "MU";
	strings[NON_MAXIMAL_SUPPRESSION] = "NON_MAXIMAL_SUPPRESSION";
	strings[SOLID_AREA]			= "SOLID_AREA";

	strings[PROPERTY]			= "PROPERTY";
	strings[UNKOWN_PROPERTY]	= "UNKOWN_PROPERTY";

	strings[SWATCHES]			= "SWATCHES";
	strings[ZOOM]				= "ZOOM";
	strings[ALPHA]				= "ALPHA";
	strings[ALPHA_CHANNEL]		= "ALPHA_CHANNEL";
	strings[COLOR]				= "COLOR";
	strings[GRADIENT]			= "GRADIENT";

	// pick objects tool
	strings[SELECTABLE]			= "SELECTABLE";

	// gradient options
	strings[GRADIENT_TYPE]					= "GRADIENT_TYPE";
	strings[GRADIENT_TYPE_LINEAR]			= "GRADIENT_TYPE_LINEAR";
	strings[GRADIENT_TYPE_RADIAL]			= "GRADIENT_TYPE_RADIAL";
	strings[GRADIENT_TYPE_DIAMONT]			= "GRADIENT_TYPE_DIAMONT";
	strings[GRADIENT_TYPE_CONIC]			= "GRADIENT_TYPE_CONIC";
	strings[GRADIENT_TYPE_XY]				= "GRADIENT_TYPE_XY";
	strings[GRADIENT_TYPE_XY_SQRT]			= "GRADIENT_TYPE_XY_SQRT";
	strings[GRADIENT_INTERPOLATION]			= "GRADIENT_INTERPOLATION";
	strings[GRADIENT_INTERPOLATION_LINEAR]	= "GRADIENT_INTERPOLATION_LINEAR";
	strings[GRADIENT_INTERPOLATION_SMOOTH]	= "GRADIENT_INTERPOLATION_SMOOTH";
	strings[GRADIENT_INHERITS_TRANSFORM]	= "GRADIENT_INHERITS_TRANSFORM";

	// brush options
	strings[OPACITY]			= "OPACITY";
	strings[MIN_OPACITY]		= "MIN_OPACITY";
	strings[DYN_OPACITY]		= "DYN_OPACITY";
	strings[RADIUS]				= "RADIUS";
	strings[MIN_RADIUS]			= "MIN_RADIUS";
	strings[DYN_RADIUS]			= "DYN_RADIUS";
	strings[HARDNESS]			= "HARDNESS";
	strings[MIN_HARDNESS]		= "MIN_HARDNESS";
	strings[DYN_HARDNESS]		= "DYN_HARDNESS";
	strings[SPACING]			= "SPACING";
	strings[MIN_SPACING]		= "MIN_SPACING";
	strings[DYN_SPACING]		= "DYN_SPACING";
	strings[SUBPIXELS]			= "SUBPIXELS";
	strings[SOLID]				= "SOLID";
	strings[TILT]				= "TILT";
	strings[BLUR_RADIUS]		= "BLUR_RADIUS";
	// translate options
	strings[X_LABEL]			= "X_LABEL";
	strings[Y_LABEL]			= "Y_LABEL";
	strings[X_SCALE]			= "X_SCALE";
	strings[Y_SCALE]			= "Y_SCALE";
	strings[TRANSLATION_X]		= "TRANSLATION_X";
	strings[TRANSLATION_Y]		= "TRANSLATION_Y";
	strings[SCALE_X]			= "SCALE_X";
	strings[SCALE_Y]			= "SCALE_Y";
	strings[ANGLE]				= "ANGLE";

	strings[X_OFFSET]			= "X_OFFSET";
	strings[Y_OFFSET]			= "Y_OFFSET";
	// crop & select options
	strings[LEFT]				= "LEFT";
	strings[TOP]				= "TOP";
	strings[ENTIRE_CANVAS]		= "ENTIRE_CANVAS";
	// bucket fill options
	strings[TOLERANCE]			= "TOLERANCE";
	strings[SOFTNESS]			= "SOFTNESS";
	strings[CONTIGUOUS_AREA]	= "CONTIGUOUS_AREA";
	// shape options
	strings[PATH]				= "PATH";
	strings[OUTLINE]			= "OUTLINE";
	strings[TRANSFORM]			= "TRANSFORM";
	strings[CLOSED]				= "CLOSED";

	strings[CAP_MODE]			= "CAP_MODE";
	strings[BUTT_CAP]			= "BUTT_CAP";
	strings[SQUARE_CAP]			= "SQUARE_CAP";
	strings[ROUND_CAP]			= "ROUND_CAP";
	strings[JOIN_MODE]			= "JOIN_MODE";
	strings[MITER_JOIN]			= "MITER_JOIN";
	strings[ROUND_JOIN]			= "ROUND_JOIN";
	strings[BEVEL_JOIN]			= "BEVEL_JOIN";

	strings[CIRCLE]				= "CIRCLE";
	strings[RECT]				= "RECT";
	strings[ROUND_CORNERS]		= "ROUND_CORNERS";
	strings[TOGGLE_ROUND_CORNERS] = "TOGGLE_ROUND_CORNERS";
	strings[ROUND_CORNER_RADIUS] = "ROUND_CORNER_RADIUS";
	strings[CHANGE_ROUND_CORNER_RADIUS] = "CHANGE_ROUND_CORNER_RADIUS";
	strings[REVERSE_PATH]		= "REVERSE_PATH";
	// text options
	strings[FONT]				= "FONT";
	strings[SIZE]				= "SIZE";
	strings[ROTATION]			= "ROTATION";
	strings[TEXT_INPUT]			= "TEXT_INPUT";
	strings[ADVANCE_SCALE]		= "ADVANCE_SCALE";
	strings[HINTING]			= "HINTING";
	strings[KERNING]			= "KERNING";
	strings[TEXT_LINE_SPACING]	= "TEXT_LINE_SPACING";
	// color picker tool
	strings[TIP_SIZE]			= "TIP_SIZE";
	strings[PIXEL_1x1]			= "PIXEL_1x1";
	strings[PIXEL_3x3]			= "PIXEL_3x3";
	strings[PIXEL_5x5]			= "PIXEL_5x5";
	strings[INCLUDE_ALL_LAYERS]	= "INCLUDE_ALL_LAYERS";
	// tools
	strings[BRUSH]				= "BRUSH";
	strings[CLONE]				= "CLONE";
	strings[ERASER]				= "ERASER";
	strings[PEN]				= "PEN";
	strings[PEN_ERASER]			= "PEN_ERASER";
	strings[RESTORE]			= "RESTORE";
	strings[DROPPER]			= "DROPPER";
	strings[TEXT]				= "TEXT";
	strings[BLUR]				= "BLUR";
	strings[BITMAP]				= "BITMAP";
	strings[SELECT]				= "SELECT";
	strings[TRANSLATE]			= "TRANSLATE";
	strings[CROP]				= "CROP";
	strings[BUCKET_FILL]		= "BUCKET_FILL";
	strings[SHAPE]				= "SHAPE";
	strings[ELLIPSE]			= "ELLIPSE";
	strings[ROUND_RECT]			= "ROUND_RECT";
	strings[EDIT_GRADIENT]		= "EDIT_GRADIENT";
	strings[GUIDES]				= "GUIDES";
	strings[PICK_OBJECTS]		= "PICK_OBJECTS";
	strings[CLIPBOARD]			= "CLIPBOARD";

	// zoom
	strings[ZOOM_IN]			= "ZOOM_IN";
	strings[ZOOM_OUT]			= "ZOOM_OUT";

	// actions
	strings[ADD_LAYER]			= "ADD_LAYER";
	strings[REMOVE_LAYER]		= "REMOVE_LAYER";
	strings[REMOVE_LAYERS]		= "REMOVE_LAYERS";
	strings[MOVE_LAYER]			= "MOVE_LAYER";
	strings[MOVE_LAYERS]		= "MOVE_LAYERS";
	strings[MERGE_LAYERS]		= "MERGE_LAYERS";
	strings[UNKOWN_MODIFIER]	= "UNKOWN_MODIFIER";
	strings[ADD_MODIFIER]		= "ADD_MODIFIER";
	strings[ADD_MODIFIERS]		= "ADD_MODIFIERS";
	strings[CHANGE_COLOR]		= "CHANGE_COLOR";
	strings[CHANGE_COLORS]		= "REMOVE_MODIFIER";
	strings[REMOVE_MODIFIER]	= "REMOVE_MODIFIERS";
	strings[REMOVE_MODIFIERS]	= "REMOVE_MODIFIERS";
	strings[MOVE_MODIFIER]		= "MOVE_MODIFIER";
	strings[MOVE_MODIFIERS]		= "MOVE_MODIFIERS";
	strings[TRANSLATE_MODIFIER]	= "TRANSLATE_MODIFIER";
	strings[TRANSLATE_MODIFIERS]	= "TRANSLATE_MODIFIERS";
	strings[CHANGE_LAYER_SETTINGS]	= "CHANGE_LAYER_SETTINGS";
	strings[CROP_CANVAS]		= "CROP_CANVAS";
	strings[RESIZE_CANVAS]		= "RESIZE_CANVAS";
	strings[ROTATE_CANVAS]		= "ROTATE_CANVAS";
	strings[INSERT_CANVAS]		= "INSERT_CANVAS";
	strings[CHANGE_OPACITY]		= "CHANGE_OPACITY";
	strings[EDIT_MODIFIER]		= "EDIT_MODIFIER";

	// pasting properties
	strings[PASTE_PROPERTY]		= "PASTE_PROPERTY";
	strings[PASTE_PROPERTIES]	= "PASTE_PROPERTIES";
	strings[MULTI_PASTE_PROPERTY] = "MULTI_PASTE_PROPERTY";
	strings[MUTLI_PASTE_PROPERTIES] = "MUTLI_PASTE_PROPERTIES";

	// shape tool history
	strings[ADD_CONTROL_POINT]	= "ADD_CONTROL_POINT";
	strings[TOGGLE_OUTLINE]		= "TOGGLE_OUTLINE";
	strings[CHANGE_OUTLINE_WIDTH] = "CHANGE_OUTLINE_WIDTH";
	strings[CHANGE_CAP_MODE]	= "CHANGE_CAP_MODE";
	strings[CHANGE_JOIN_MODE]	= "CHANGE_JOIN_MODE";
	strings[MODIFY_CONTROL_POINT] = "MODIFY_CONTROL_POINT";
	strings[CLOSE_PATH_ACTION]	= "CLOSE_PATH_ACTION";
	strings[INSERT_CONTROL_POINT] = "INSERT_CONTROL_POINT";
	strings[REMOVE_CONTROL_POINT] = "REMOVE_CONTROL_POINT";
	strings[REMOVE_CONTROL_POINTS] = "REMOVE_CONTROL_POINTS";
	strings[CHANGE_POINT_SELECTION] = "CHANGE_POINT_SELECTION";
	strings[NUDGE_CONTROL_POINT] = "NUDGE_CONTROL_POINT";
	strings[NUDGE_CONTROL_POINTS] = "NUDGE_CONTROL_POINTS";
	strings[SELECT_PATH]		= "SELECT_PATH";
	strings[CREATE_NEW_PATH]	= "CREATE_NEW_PATH";
	strings[DELETE_PATH]		= "DELETE_PATH";
	strings[TEXT_TO_SHAPE]		= "TEXT_TO_SHAPE";
	strings[TEXTS_TO_SHAPES]	= "TEXTS_TO_SHAPES";
	strings[TRANSFORM_CONTROL_POINTS] = "TRANSFORM_CONTROL_POINTS";
	strings[MERGE_SHAPE_OBJECTS] = "MERGE_SHAPE_OBJECTS";

	strings[FILLING_RULE]		= "FILLING_RULE";
	strings[NON_ZERO]			= "NON_ZERO";
	strings[EVEN_ODD]			= "EVEN_ODD";
	// text tool history
	strings[CHANGE_FONT_SIZE]	= "CHANGE_FONT_SIZE";
	strings[CHANGE_TEXT_ROTATION] = "CHANGE_TEXT_ROTATION";
	strings[MOVE_TEXT]			= "MOVE_TEXT";
	strings[CHANGE_FONT]		= "CHANGE_FONT";
	strings[CHANGE_TEXT]		= "CHANGE_TEXT";
	strings[CHANGE_FONT_ADVANCE_SCALE] = "CHANGE_FONT_ADVANCE_SCALE";
	strings[ALIGNMENT]			= "ALIGNMENT";
	strings[CHANGE_ALIGNMENT]	= "CHANGE_ALIGNMENT";
	strings[ALIGNMENT_LEFT]		= "ALIGNMENT_LEFT";
	strings[ALIGNMENT_CENTER]	= "ALIGNMENT_CENTER";
	strings[ALIGNMENT_RIGHT]	= "ALIGNMENT_RIGHT";
	strings[ALIGNMENT_JUSTIFY]	= "ALIGNMENT_JUSTIFY";
	strings[CHANGE_TEXT_WIDTH]	= "CHANGE_TEXT_WIDTH";
	strings[PARAGRAPH_INSET]	= "PARAGRAPH_INSET";
	strings[CHANGE_PARAGRAPH_INSET]	= "CHANGE_PARAGRAPH_INSET";
	strings[PARAGRAPH_SPACING]	= "PARAGRAPH_SPACING";
	// crop tool
	strings[MODIFY_CROP_RECT]	= "MODIFY_CROP_RECT";
	// select tool
	strings[MODIFY_SELECT_RECT]	= "MODIFY_SELECT_RECT";
	// transformations
	strings[TRANSFORMATION]		= "TRANSFORMATION";
	strings[BEGIN_TRANSFORMATION] = "BEGIN_TRANSFORMATION";
	strings[FINISH_TRANSFORMATION]	= "FINISH_TRANSFORMATION";
	strings[MOVE]				= "MOVE";
	strings[ROTATE]				= "ROTATE";
	strings[SCALE]				= "SCALE";
	strings[MOVE_PIVOT]			= "MOVE_PIVOT";
	strings[TRANSFORM_OBJECT]	= "TRANSFORM_OBJECT";
	strings[TRANSFORM_OBJECTS]	= "TRANSFORM_OBJECTS";
	// guide tool
	strings[SHOW_GUIDES]		= "SHOW_GUIDES";
	strings[HORIZONTAL]			= "HORIZONTAL";
	strings[VERTICAL]			= "VERTICAL";
	strings[UNIT]				= "UNIT";
	strings[UNIT_PERCENT]		= "UNIT_PERCENT";
	strings[UNIT_PIXEL]			= "UNIT_PIXEL";
	strings[UNIT_CM]			= "UNIT_CM";
	strings[UNIT_MM]			= "UNIT_MM";
	strings[UNIT_INCH]			= "UNIT_INCH";
	strings[UNIT_DPI]			= "UNIT_DPI";
	// general
	strings[OK]					= "OK";
	strings[CONFIRM]			= "CONFIRM";
	strings[CANCEL]				= "CANCEL";
	strings[REVERT]				= "REVERT";
	strings[REMOVE]				= "REMOVE";
	strings[NAME]				= "NAME";
	strings[UNNAMED]			= "UNNAMED";
	strings[DEFAULT]			= "DEFAULT";
	strings[MERGED]				= "MERGED";
	strings[TOOL]				= "TOOL";
	strings[ANY_MODIFIER]		= "ANY_MODIFIER";
	strings[RETURN]				= "RETURN";
	strings[ESCAPE]				= "ESCAPE";
	strings[SET]				= "SET";
	strings[COUNT]				= "COUNT";
	strings[INTERPOLATION]		= "INTERPOLATION";
	strings[NEAREST_NEIGHBOR]	= "NEAREST_NEIGHBOR";
	strings[BILINEAR]			= "BILINEAR";
	strings[RETRY]				= "RETRY";
	strings[OVERWRITE]			= "OVERWRITE";
	// panels
	strings[PICK_COLOR]			= "PICK_COLOR";
	strings[WIDTH]				= "WIDTH";
	strings[HEIGHT]				= "HEIGHT";
	strings[BRUSH_PANEL]		= "BRUSH_PANEL";
	strings[BRUSH_NAME]			= "BRUSH_NAME";
	strings[PROGRAM_SETTINGS]	= "PROGRAM_SETTINGS";
	// program settings
	strings[ON_STARTUP]			= "ON_STARTUP";
	strings[DO_NOTHING]			= "DO_NOTHING";
	strings[ASK_OPEN]			= "ASK_OPEN";
	strings[ASK_NEW]			= "ASK_NEW";
	strings[AUTO_SIZE]			= "AUTO_SIZE";
	strings[LIVE_UPDATES]		= "LIVE_UPDATES";
	strings[USE_SYSTEM_LANGUAGE] = "USE_SYSTEM_LANGUAGE";
	strings[STARTUP_ACTION]		= "STARTUP_ACTION";
	strings[INTERFACE]			= "INTERFACE";
	// requestors
	strings[AKS_SAVE_CHANGES]	= "AKS_SAVE_CHANGES";
	strings[DISCARD]			= "DISCARD";
	strings[SAVING_FAILED]		= "SAVING_FAILED";
	strings[ERROR]				= "ERROR";
	strings[BLIP]				= "BLIP";

	strings[DEMO_MODE]			= "DEMO_MODE";
	strings[NEVER_MIND]			= "NEVER_MIND";
	strings[OPEN_TRANSLATIONKIT_ERROR]	= "OPEN_TRANSLATIONKIT_ERROR";
	strings[OPEN_NATIVE_ERROR]	= "OPEN_NATIVE_ERROR";
	strings[UNSUPPORTED_CURSOR_SIZE] = "UNSUPPORTED_CURSOR_SIZE";
	strings[SELECTED_AREA_EMPTY] = "SELECTED_AREA_EMPTY";
	strings[ENTRY_DOES_NOT_EXIST] = "ENTRY_DOES_NOT_EXIST";
	strings[ENTRY_IS_A_FOLDER]	= "ENTRY_IS_A_FOLDER";
	strings[WARN_SVG_DATA_LOSS]	= "WARN_SVG_DATA_LOSS";

	strings[TOOL_TIPS]			= "TOOL_TIPS";
	strings[ALPHA_TIP]			= "ALPHA_TIP";
	strings[VISIBILITY_TIP]		= "VISIBILITY_TIP";
	strings[PRESSURE_CONTROL_TIP]	= "PRESSURE_CONTROL_TIP";
	strings[BRUSH_PREVIEW_TIP]	= "BRUSH_PREVIEW_TIP";
	strings[CURRENT_COLOR_TIP]	= "CURRENT_COLOR_TIP";
	strings[NAVIGATOR_TIP]		= "NAVIGATOR_TIP";
	strings[LAYER_LIST_TIP]		= "LAYER_LIST_TIP";
	strings[HISTORY_LIST_TIP]	= "HISTORY_LIST_TIP";
	strings[PROPERTY_LIST_TIP]	= "PROPERTY_LIST_TIP";
	strings[SUB_PIXEL_TIP]		= "SUB_PIXEL_TIP";
	strings[SOLID_TIP]			= "SOLID_TIP";
	strings[TILT_TIP]			= "TILT_TIP";
	strings[FULLSCREEN_TIP]		= "FULLSCREEN_TIP";
	strings[GRADIENT_CONTROL_TIP] = "GRADIENT_CONTROL_TIP";
	strings[GRADIENT_INHERITS_TRANSFORM_TIP] = "GRADIENT_INHERITS_TRANSFORM_TIP";

	// quick help
	strings[READY]				= "READY";
	strings[NO_CANVAS_HELP]		= "NO_CANVAS_HELP";
	strings[NO_HELP]			= "NO_HELP";
	strings[CROP_HELP]			= "CROP_HELP";
	strings[TRANSLATE_HELP]		= "TRANSLATE_HELP";
	strings[EDIT_GRADIENT_HELP]	= "EDIT_GRADIENT_HELP";
	strings[STROKE_HELP]		= "STROKE_HELP";
	strings[CLONE_HELP]			= "CLONE_HELP";
	strings[BUCKET_FILL_HELP]	= "BUCKET_FILL_HELP";
	strings[TEXT_HELP]			= "TEXT_HELP";
	strings[SHAPE_HELP]			= "SHAPE_HELP";
	strings[SELECT_HELP]		= "SELECT_HELP";
	strings[ELLIPSE_HELP]		= "ELLIPSE_HELP";
	strings[ROUND_RECT_HELP]	= "ROUND_RECT_HELP";
	strings[PICK_OBJECTS_HELP]	= "PICK_OBJECTS_HELP";
	strings[GUIDES_HELP]		= "GUIDES_HELP";

	strings[ENTER_SERIAL_NUMBER] = "ENTER_SERIAL_NUMBER";
	strings[WRONG_SERIAL_NUMBER] = "WRONG_SERIAL_NUMBER";
}

