// GlobalFunctions.cpp

#include <math.h>
#include <stdio.h>

#include <Entry.h>
#include <Font.h>
#include <MediaDefs.h>
#include <MediaFormats.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Path.h>
#include <Screen.h>
#include <String.h>
#include <TranslationKit.h>

#include "GlobalFunctions.h"

// get_path_for_ref
bool
get_path_for_ref(const entry_ref *ref, BString *pathname)
{
	BEntry entry;
	BPath path;
	const char *name;
	if (entry.SetTo(ref) == B_OK
		 && path.SetTo(&entry) == B_OK
		 && (name = path.Path())) {
		pathname->SetTo(name);
		return true;
	}
	return false;
}

// get_name_for_ref
bool
get_name_for_ref(const entry_ref *ref, BString *pathname,
				 BString *filename)
{
	BEntry entry;
	BPath path;
	const char *name;
	if (entry.SetTo(ref) == B_OK
		 && path.SetTo(&entry) == B_OK
		 && (name = path.Path())) {
		pathname->SetTo(name);
		int32 pos = pathname->FindLast("/");
		// MoveInto() seems to be buggy (probable because Remove() is buggy),
		// we work around
//		pathname->MoveInto(*filename, pos + 1, pathname->Length() - pos);
		pathname->CopyInto(*filename, pos + 1, pathname->Length() - pos);
//		pathname->Remove(pos + 1, pathname->CountChars() - pos);
		pathname->Truncate(pos + 1);

		return true;
	}
	return false;
}

// basename
const char*
basename(const char *path)
{
	if (path) {
		for (int32 i = strlen(path) - 1; i >= 0; i--) {
			if (path[i] == '/')
				return path + i + 1;
		}
	}
	return path;
}

// basename
const char*
basename(const BString *path)
{
	if (path)
		return basename(path->String());
	return 0;
}

// get_dirname
void
get_dirname(const char *path, char *dirname)
{
	if (path && dirname)
		strncpy(dirname, path, basename(path) - path);
}

// get_dirname
void
get_dirname(const char *path, BString *dirname)
{
	if (path && dirname)
		dirname->SetTo(path, basename(path) - path);
}

// get_dirname
void
get_dirname(const BString *path, BString *dirname)
{
	if (path && dirname)
		get_dirname(path->String(), dirname);
}

// split_path
void
split_path(const char *path, char *dir, char *file)
{
	if (path && dir && file) {
		get_dirname(path, dir);
		strcpy(file, basename(path));
	}
}

// split_path
void
split_path(const char *path, BString *dir, BString *file)
{
	if (path && dir && file) {
		get_dirname(path, dir);
		file->SetTo(basename(path));
	}
}

// split_path
void
split_path(const BString *path, BString *dir, BString *file)
{
	if (path && dir && file)
		split_path(path->String(), dir, file);
}

// build_media_format
bool
build_media_format(int32 width,
				   int32 height,
				   color_space cspace,
				   media_format* format)
{
	if (cspace == B_RGB32) {
		media_raw_video_format *rvf = &format->u.raw_video;
		
		memset(format, 0L, sizeof(*format));
		
		format->type = B_MEDIA_RAW_VIDEO;
		rvf->last_active = (uint32)(height -1);
		rvf->orientation = B_VIDEO_TOP_LEFT_RIGHT;
		rvf->pixel_width_aspect = 1;
		rvf->pixel_height_aspect = 1;
		rvf->display.format = cspace;
		rvf->display.line_width = width;
		rvf->display.line_count = height;
		rvf->display.bytes_per_row = 4 * width;
		return true;
	}
	return false;
}

// plain_font_height
float
plain_font_height()
{
	font_height fontHeight;
	be_plain_font->GetHeight(&fontHeight);
	return ceil(1.0 + fontHeight.ascent + fontHeight.descent + 4.0);
}

// bold_font_height
float
bold_font_height()
{
	font_height fontHeight;
	be_bold_font->GetHeight(&fontHeight);
	return ceil(1.0 + fontHeight.ascent + fontHeight.descent + 4.0);
}

// add_translator_items
status_t
add_translator_items(BMenu* intoMenu, uint32 fromType, uint32 command)
{ 
    BTranslatorRoster* use;
    char* translator_type_name;
    const char* translator_id_name;
    
    use = BTranslatorRoster::Default();
	translator_id_name = "be:translator"; 
	translator_type_name = "be:type";
	translator_id* ids = NULL;
	int32 count = 0;
	
	status_t err = use->GetAllTranslators(&ids, &count); 
	if (err < B_OK)
		return err; 
	for (int tix=0; tix<count; tix++) { 
		const translation_format * formats = NULL; 
		int32 num_formats = 0; 
		bool ok = false; 
		err = use->GetInputFormats(ids[tix], &formats, &num_formats); 
		if (err == B_OK) for (int iix=0; iix<num_formats; iix++) { 
			if (formats[iix].type == fromType) { 
				ok = true; 
				break; 
			} 
		} 
		if (!ok) continue; 
			err = use->GetOutputFormats(ids[tix], &formats, &num_formats); 
		if (err == B_OK) for (int oix=0; oix<num_formats; oix++) { 
 			if (formats[oix].type != fromType) { 
				BMessage * itemmsg; 
				itemmsg = new BMessage(command);
				itemmsg->AddInt32(translator_id_name, ids[tix]);
				itemmsg->AddInt32(translator_type_name, formats[oix].type); 
				intoMenu->AddItem(new BMenuItem(formats[oix].name, itemmsg)); 
//printf("%s: id: %ld, type: %ld\n", formats[oix].name, ids[tix], formats[oix].type);
 			} 
		} 
	} 
	delete[] ids; 
	return B_OK; 
}

// make_sure_frame_is_on_screen
void
make_sure_frame_is_on_screen(BRect& frame, BWindow* window)
{
	BScreen screen(window);
	if (frame.IsValid() && screen.IsValid()) {
		if (!screen.Frame().Contains(frame)) {
			// make sure frame fits in the screen
			if (frame.Width() > screen.Frame().Width())
				frame.right -= frame.Width() - screen.Frame().Width() + 10.0;
			if (frame.Height() > screen.Frame().Height())
				frame.bottom -= frame.Height() - screen.Frame().Height() + 30.0;
			// frame is now at the most the size of the screen
			if (frame.right > screen.Frame().right)
				frame.OffsetBy(-(frame.right - screen.Frame().right), 0.0);
			if (frame.bottom > screen.Frame().bottom)
				frame.OffsetBy(0.0, -(frame.bottom - screen.Frame().bottom));
			if (frame.left < screen.Frame().left)
				frame.OffsetBy((screen.Frame().left - frame.left), 0.0);
			if (frame.top < screen.Frame().top)
				frame.OffsetBy(0.0, (screen.Frame().top - frame.top));
		}
	}
}

// center_frame_in_screen
void
center_frame_in_screen(BRect& frame)
{
	BScreen screen;
	if (screen.IsValid()) {
		display_mode mode;
		if (screen.GetMode(&mode) == B_OK) {
			float h_offset = frame.Width() / 2;
			float v_offset = frame.Height() / 2;
			frame.left = mode.virtual_width / 2 - h_offset;
			frame.right = mode.virtual_width / 2 + h_offset;
			frame.top = mode.virtual_height / 2 - v_offset;
			frame.bottom = mode.virtual_height / 2 + v_offset;
		}
	}
}

