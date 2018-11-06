// support.cpp

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <DataIO.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Screen.h>
#include <String.h>
#include <Path.h>
#include <View.h>

#include <Space.h>

#include "defines.h"

#include "FlattenableProtocol.h"

#include "support.h"

// calc_point_line_distance
double
calc_point_line_distance(double x1, double y1,
						 double x2, double y2,
						 double x,  double y)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	return ((x - x2) * dy - (y - y2) * dx) / sqrt(dx * dx + dy * dy);
}

// calc_angle
double
calc_angle(BPoint origin, BPoint from, BPoint to, bool degree)
{
	double angle = 0.0;

	double d = calc_point_line_distance(from.x, from.y,
										origin.x, origin.y,
										to.x, to.y);
	if (d != 0.0) {
		double a = dist(from, to);
		double b = dist(from, origin);
		double c = dist(to, origin);
		if (a > 0.0 && b > 0.0 && c > 0.0) {
			angle = acos((b*b + c*c - a*a) / (2.0*b*c));
		
			if (d < 0.0)
				angle = -angle;

			if (degree)
				angle = angle * 180.0 / M_PI;
		}
	}
	return angle;
}

// load_settings
status_t
load_settings(BMessage* message, const char* fileName, const char* folder)
{
	status_t ret = B_BAD_VALUE;
	if (message) {
		BPath path;
		if ((ret = find_directory(B_USER_SETTINGS_DIRECTORY, &path)) == B_OK) {
			// passing folder is optional
			if (folder)
				ret = path.Append( folder );
			if (ret == B_OK && (ret = path.Append(fileName)) == B_OK ) {
				BFile file(path.Path(), B_READ_ONLY);
				if ((ret = file.InitCheck()) == B_OK) {
					ret = message->Unflatten(&file);
					file.Unset();
				}
			}
		}
	}
	return ret;
}

// save_settings
status_t
save_settings(BMessage* message, const char* fileName, const char* folder)
{
	status_t ret = B_BAD_VALUE;
	if (message) {
		BPath path;
		if ((ret = find_directory(B_USER_SETTINGS_DIRECTORY, &path)) == B_OK) {
			// passing folder is optional
			if (folder && (ret = path.Append(folder)) == B_OK)
				ret = create_directory(path.Path(), 0777);
			if (ret == B_OK && (ret = path.Append(fileName)) == B_OK) {
				BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
				if ((ret = file.InitCheck()) == B_OK) {
					ret = message->Flatten(&file);
					file.Unset();
				}
			}
		}
	}
	return ret;
}

// store_color_in_message
status_t
store_color_in_message(BMessage* message, rgb_color color)
{
	status_t ret = B_BAD_VALUE;
	if (message) {
		ret = message->AddData("RGBColor", B_RGB_COLOR_TYPE,
							   (void*)&color, sizeof(rgb_color));
	}
	return ret;
}

// restore_color_from_message
status_t
restore_color_from_message(const BMessage* message, rgb_color& color, int32 index)
{
	status_t ret = B_BAD_VALUE;
	if (message) {
			const void* colorPointer;
			ssize_t size = sizeof(rgb_color);
			ret = message->FindData("RGBColor", B_RGB_COLOR_TYPE, index,
									&colorPointer, &size);
			if (ret >= B_OK)
				color = *(const rgb_color*)colorPointer;
	}
	return ret;
}

// stroke_frame
void
stroke_frame(BView* v, BRect r, rgb_color left, rgb_color top,
			 rgb_color right, rgb_color bottom)
{
	if (v && r.IsValid()) {
		v->BeginLineArray(4);
			v->AddLine(BPoint(r.left, r.bottom),
					   BPoint(r.left, r.top), left);
			v->AddLine(BPoint(r.left + 1.0, r.top),
					   BPoint(r.right, r.top), top);
			v->AddLine(BPoint(r.right, r.top + 1.0),
					   BPoint(r.right, r.bottom), right);
			v->AddLine(BPoint(r.right - 1.0, r.bottom),
					   BPoint(r.left + 1.0, r.bottom), bottom);
		v->EndLineArray();
	}
}

// vertical_space
Space*
vertical_space()
{
	return new Space(minimax(0.0, 3.0, 10000.0, 3.0, 1.0));
}

// horizontal_space
Space*
horizontal_space()
{
	return new Space(minimax(3.0, 0.0, 3.0, 10000.0, 1.0));
}

// make_color_drop_message
BMessage
make_color_drop_message(rgb_color color, BBitmap* bitmap)
{
	// prepare message
	BMessage message(B_PASTE);
	char hexstr[7];
	sprintf(hexstr, "#%.2X%.2X%.2X", color.red, color.green, color.blue);
	message.AddData("text/plain", B_MIME_TYPE, &hexstr, sizeof(hexstr));
	message.AddData("RGBColor", B_RGB_COLOR_TYPE, &color, sizeof(color));
	// prepare bitmap
	if (bitmap && bitmap->IsValid()
		&& (bitmap->ColorSpace() == B_RGB32
			|| bitmap->ColorSpace() == B_RGBA32)) {
		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		uint32 width = bitmap->Bounds().IntegerWidth() + 1;
		uint32 height = bitmap->Bounds().IntegerHeight() + 1;
		for (uint32 y = 0; y < height; y++) {
			uint8* bitsHandle = bits;
			for (uint32 x = 0; x < width; x++) {
				if (x == 0 || y == 0 ) {
					// top or left border
					bitsHandle[0] = (uint8)min_c(255, color.blue * 1.2 + 40);
					bitsHandle[1] = (uint8)min_c(255, color.green * 1.2 + 40);
					bitsHandle[2] = (uint8)min_c(255, color.red * 1.2 + 40);
					bitsHandle[3] = 180;
				} else if ((x == width - 2 || y == height - 2)
						   && !(x == width - 1 || y == height - 1)) {
					// bottom or right border
					bitsHandle[0] = (uint8)(color.blue * 0.8);
					bitsHandle[1] = (uint8)(color.green * 0.8);
					bitsHandle[2] = (uint8)(color.red * 0.8);
					bitsHandle[3] = 180;
				} else if (x == width - 1 || y == height - 1) {
					// shadow
					bitsHandle[0] = 0;
					bitsHandle[1] = 0;
					bitsHandle[2] = 0;
					bitsHandle[3] = 100;
				} else {
					// color
					bitsHandle[0] = color.blue;
					bitsHandle[1] = color.green;
					bitsHandle[2] = color.red;
					bitsHandle[3] = 180;
				}
				if ((x == 0 && y == height - 1) || (y == 0 && x == width - 1)) {
					// spare pixels of shadow
					bitsHandle[0] = 0;
					bitsHandle[1] = 0;
					bitsHandle[2] = 0;
					bitsHandle[3] = 50;
				}
				bitsHandle += 4;
			}
			bits += bpr;
		}
	}
	return message;
}

// print_modifiers
void
print_modifiers()
{
	uint32 mods = modifiers();
	if (mods & B_SHIFT_KEY)
		printf("B_SHIFT_KEY\n");
	if (mods & B_COMMAND_KEY)
		printf("B_COMMAND_KEY\n");
	if (mods & B_CONTROL_KEY)
		printf("B_CONTROL_KEY\n");
	if (mods & B_CAPS_LOCK)
		printf("B_CAPS_LOCK\n");
	if (mods & B_SCROLL_LOCK)
		printf("B_SCROLL_LOCK\n");
	if (mods & B_NUM_LOCK)
		printf("B_NUM_LOCK\n");
	if (mods & B_OPTION_KEY)
		printf("B_OPTION_KEY\n");
	if (mods & B_MENU_KEY)
		printf("B_MENU_KEY\n");
	if (mods & B_LEFT_SHIFT_KEY)
		printf("B_LEFT_SHIFT_KEY\n");
	if (mods & B_RIGHT_SHIFT_KEY)
		printf("B_RIGHT_SHIFT_KEY\n");
	if (mods & B_LEFT_COMMAND_KEY)
		printf("B_LEFT_COMMAND_KEY\n");
	if (mods & B_RIGHT_COMMAND_KEY)
		printf("B_RIGHT_COMMAND_KEY\n");
	if (mods & B_LEFT_CONTROL_KEY)
		printf("B_LEFT_CONTROL_KEY\n");
	if (mods & B_RIGHT_CONTROL_KEY)
		printf("B_RIGHT_CONTROL_KEY\n");
	if (mods & B_LEFT_OPTION_KEY)
		printf("B_LEFT_OPTION_KEY\n");
	if (mods & B_RIGHT_OPTION_KEY)
		printf("B_RIGHT_OPTION_KEY\n");
}

// push_data
status_t
push_data(BDataIO* object, const void* data, ssize_t size, ssize_t& total)
{
	status_t status = object->Write(data, size);
	total += size;
	return status;
}

// push_bitmap
status_t
push_bitmap(BDataIO* object, BBitmap* bitmap, ssize_t& total)
{
	uint32 width = bitmap->Bounds().IntegerWidth() + 1;
	uint32 height = bitmap->Bounds().IntegerHeight() + 1;
	color_space format = bitmap->ColorSpace();
	uint32 bytesPerRow = bitmap->BytesPerRow();
	uint32 size = bitmap->BitsLength();
	
	uint32 tagSize = sizeof(width) + sizeof(height)
					 + sizeof(format) + sizeof(bytesPerRow) + size;
	
	uint32 tag = TAG_BITMAP;
	status_t status = push_data(object, &tag, sizeof(tag), total);
	if (status >= B_OK) {
		status = push_data(object, &tagSize, sizeof(tagSize), total);
		if (status >= B_OK) {
			status = push_data(object, &width, sizeof(width), total);
			if (status >= B_OK) {
				status = push_data(object, &height, sizeof(height), total);
				if (status >= B_OK) {
					status = push_data(object, &format, sizeof(format), total);
					if (status >= B_OK) {
						status = push_data(object, &bytesPerRow,
										   sizeof(bytesPerRow), total);
						if (status >= B_OK) {
							status = push_data(object, bitmap->Bits(),
											   size, total);
						}
					}
				}
			}
		}
	}
	if (status > B_OK)
		status = B_OK;
	return status;
}

// pull_bitmap
status_t
pull_bitmap(BDataIO* object, BBitmap** bitmapPointer, ssize_t tagSize)
{
	// assumtion:
	// objects position is at the byte after the tag size 
	// (at the beginning of the actual tag data)
	uint32 header[4];
	status_t status = object->Read(header, 16);
	if (status >= B_OK) {
		tagSize -= 16;
		uint32 width = header[0];
		uint32 height = header[1];
		uint32 format = header[2];
		uint32 bytesPerRow = header[3];
		BBitmap* bitmap = new BBitmap(BRect(0.0, 0.0, width - 1.0, height - 1.0),
									  (color_space)format);
		status = bitmap->InitCheck();
		if (status >= B_OK) {
			if (tagSize == bitmap->BitsLength()
				&& (uint32)bitmap->BytesPerRow() == bytesPerRow) {
				status = object->Read(bitmap->Bits(), tagSize);
				if (status >= B_OK) {
					*bitmapPointer = bitmap;
					status = B_OK;
				}
			} else {
				// TODO: handle this!
				// read line by line or something
				status = B_ERROR;
			}
		}
		if (status < B_OK)
			delete bitmap;
	}
	return status;
}

// write_string
status_t
write_string(BPositionIO* stream, BString& string)
{
	if (!stream)
		return B_BAD_VALUE;

	ssize_t written = stream->Write(string.String(), string.Length());
	if (written > B_OK && written < string.Length())
		written = B_ERROR;
	string.SetTo("");
	return written;
}

// append_float
void
append_float(BString& string, float n, int32 maxDigits = 4)
{
	int32 rounded = n >= 0.0 ? (int32)fabs(floorf(n)) : (int32)fabs(ceilf(n));

	if (n < 0.0) {
		string << "-";
		n *= -1.0;
	}
	string << rounded;

	if ((float)rounded != n) {
		// find out how many digits remain
		n = n - rounded;
		rounded = (int32)(n * pow(10, maxDigits));
		char tmp[maxDigits + 1];
		sprintf(tmp, "%0*ld", (int)maxDigits, rounded);
		tmp[maxDigits] = 0;
		int32 digits = strlen(tmp);
		for (int32 i = strlen(tmp) - 1; i >= 0; i--) {
			if (tmp[i] == '0')
				digits--;
			else
				break;
		}
		// write after decimal
		if (digits > 0) {
			string << ".";
			for (int32 i = 0; i < digits; i++) {
				string << tmp[i];
			}
		}
	}
}

// gauss
double
gauss(double f)
{ 
	// this aint' a real gauss function
/*	if (f >= -1.0 && f <= 1.0) {
		if (f < -0.5) {
			f = -1.0 - f;
			return (2.0 * f*f);
		}

		if (f < 0.5)
			return (1.0 - 2.0 * f*f);

		f = 1.0 - f;
		return (2.0 * f*f);
	}*/
	if (f > 0.0) {
		if (f < 0.5)
			return (1.0 - 2.0 * f*f);

		f = 1.0 - f;
		return (2.0 * f*f);
	}
	return 1.0;
}

// convert_cap_mode
agg::line_cap_e
convert_cap_mode(uint32 mode)
{
	agg::line_cap_e aggMode = agg::butt_cap;
	switch (mode) {
		case CAP_MODE_BUTT:
			aggMode = agg::butt_cap;
			break;
		case CAP_MODE_SQUARE:
			aggMode = agg::square_cap;
			break;
		case CAP_MODE_ROUND:
			aggMode = agg::round_cap;
			break;
	}
	return aggMode;
}

// convert_cap_mode
agg::line_join_e
convert_join_mode(uint32 mode)
{
	agg::line_join_e aggMode = agg::miter_join;
	switch (mode) {
		case JOIN_MODE_MITER:
			aggMode = agg::miter_join;
			break;
		case JOIN_MODE_ROUND:
			aggMode = agg::round_join;
			break;
		case JOIN_MODE_BEVEL:
			aggMode = agg::bevel_join;
			break;
	}
	return aggMode;
}

