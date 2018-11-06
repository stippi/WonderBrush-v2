// GlobalFunctions.h

#ifndef GLOBAL_FUNCTIONS_H
#define GLOBAL_FUNCTIONS_H

#include <Entry.h>
#include <GraphicsDefs.h>

class BMenu;
class BString;
class BRect;
class BWindow;

bool get_path_for_ref(const entry_ref *ref, BString *pathname);
bool get_name_for_ref(const entry_ref *ref, BString *pathname,
					  BString *filename);

// basename() returns a pointer to the substring starting after the last
// '/' or, if path doesn't contain any, a pointer to the beginning of path
const char *basename(const char *path);
const char *basename(const BString *path);

// get_dirname() returns in dirname the substring from the beginning of path
// to the last (inclusively) '/', or, if path doesn't contain any, an empty
// string
void get_dirname(const char *path, char *dirname);
void get_dirname(const char *path, BString *dirname);
void get_dirname(const BString *path, BString *dirname);

// split_path simply splits a path according to the results of basename() and
// get_dirname()
void split_path(const char *path, char *dir, char *file);
void split_path(const char *path, BString *dir, BString *file);
void split_path(const BString *path, BString *dir, BString *file);

struct media_format;
// return, in format, a valid media format
bool build_media_format(int32 width,
						int32 height,
						color_space cspace,
						media_format *format);

// plain_font_height
float plain_font_height();
// bold_font_height
float bold_font_height();

// add_translator_items
status_t add_translator_items(BMenu* intoMenu, uint32 fromType,
							  uint32 command);

void make_sure_frame_is_on_screen(BRect& frame, BWindow* window);
void center_frame_in_screen(BRect& frame);

#endif	// GLOBAL_FUNCTIONS_H
