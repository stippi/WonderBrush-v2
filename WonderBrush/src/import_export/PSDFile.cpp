// PSDFile.cpp

#include <malloc.h>
#include <stdio.h>

#include <ByteOrder.h>

#include "Canvas.h"
#include "Layer.h"

#include "PSDFile.h"

// constructor
PSDFile::PSDFile()
{
}

// destructor
PSDFile::~PSDFile()
{
}

const uint32 kResourceSig = '8BIM';
const uint32 kPhotoshopSig = '8BPS';

enum {
	PRINT_MANAGER			= 0x03e9,	// optional
	RESOLUTION_INFO			= 0x03ed,
	ALPHA_CHANNEL_NAMES		= 0x03ee,
	DISPLAY_INFO			= 0x03ef,
	CAPTION					= 0x03f0,	// optional
	BORDER_INFO				= 0x03f1,	// Border information. Contains a fixed­number
										// for the border width, and 2 bytes for border
										// units (1=inches, 2=cm, 3=points, 4=picas, 5=columns).
	BACKGROUND_COLOR		= 0x03f2,
	PRINT_FLAGS				= 0x03f3,	// A series of one byte boolean values
										// (labels, crop marks, color bars, registration
										//  marks, negative, flip, interpolate, caption.)
	GRAY_HALFTONE_INFO		= 0x03f4,	// Grayscale and multichannel halftoning information.
	COLOR_HALFTONE_INFO		= 0x03F5,	// Color halftoning information.
	DOUTONE_HALFTONE_INFO	= 0x03F6,	// Duotone halftoning information.
	GRAY_TRANSFER_FUNC		= 0x03F7,	// Grayscale and multichannel transfer function.
	COLOR_TRANSFER_FUNC		= 0x03F8,	// Color transfer functions.
	DOUTONE_TRANSFER_FUNC	= 0x03F9,	// Duotone transfer functions.
	DOUTONE_INFO			= 0x03FA,	// Duotone image information.
	DOT_RANGE				= 0x03FB,	// Two bytes for the effective black and white
										// values for the dot range.
//							= 0x03FC,	// Obsolete.
	EPS_OPTIONS				= 0x03FD,	// EPS options.
	QUICK_MASK_INFO			= 0x03FE,	// Quick Mask information. 2 bytes containing
										// Quick Mask channel ID, 1 byte boolean indicating
										// whether the mask was initially empty.
//							= 0x03FF,   // Obsolete.
	LAYER_STATE_INFO		= 0x0400,	// Layer state information. 2 bytes containing the
										// index of target layer. 0 = bottom layer.
	WORKING_PATH			= 0x0401,	// Working path (not saved). See path resource
										// format later in this chapter.
	LAYER_GROUP_INFO		= 0x0402,	// Layers group information. 2 bytes per layer
										// containing a group ID for the dragging groups.
										// Layers in a group have the same group ID.
//							= 0x0403,	// Obsolete.
	FILE_INFO				= 0x0404,	// IPTC­NAA record. This contains the
										// File Info... information. See the IIMV4.pdf document.
	RAW_IMAGE_MODE			= 0x0405,	// Image mode for raw format files.
	JPEG_QUALITY			= 0x0406,	// JPEG quality. Private.
	GRID_GUIDE_INFO			= 0x0408,	// Grid and guides information. See grid and
										// guides resource format later in this chapter.
	THUMBNAIL_RESOURCE1		= 0x0409,	// Thumbnail resource. See thumbnail resource format
										// later in this chapter.
	COPYRIGHT_FLAG			= 0x040A,	// Copyright flag. Boolean indicating whether image
										// is copyrighted. Can be set via Property suite or
										// by user in File Info...
	URL						= 0x040B,	// URL. Handle of a text string with uniform resource
										// locator. Can be set via Property suite or by user
										// in File Info...
	THUMBNAIL_RESOURCE2		= 0x040C,	// Thumbnail resource. See thumbnail resource format
										// later in this chapter.
	GLOBAL_ANGLE			= 0x040D,	// Global Angle. 4 bytes that contain an integer
										// between 0..359 which is the global lighting angle
										// for effects layer. If not present assumes 30.
	COLOR_SAMPLERS_RSRC		= 0x040E,	// Color samplers resource. See color samplers resource
										// format later in this chapter.
	ICC_PROFILE				= 0x040F,	// ICC Profile. The raw bytes of an ICC format profile,
										// see the ICC34.pdf and ICC34.h files from the
										// Internation Color Consortium located in the
										// documentation section.
	WATERMARK				= 0x0410,	// One byte for Watermark. 
	ICC_UNTAGGED			= 0x0411,	// ICC Untagged. 1 byte that disables any assumed
										// profile handling when opening the file.
										// 1 = intentionally untagged.
	EFFECTS_VISIBLE			= 0x0412,	// Effects visible. 1 byte global flag to show/hide all
										// the effects layer. Only present when they are hidden.
	SPOT_HALFTONE			= 0x0413,	// Spot Halftone. 4 bytes for version, 4 bytes for length,
										// and the variable length data.
	DOCUMENT_SPECIFIC_IDS	= 0x0414,	// Document specific IDs, layer IDs will be generated
										// starting at this base value or a greater value if we
										// find existing IDs to already exceed it. It's purpose
										// is to avoid the case where we add layers, flatten,
										// save, open, and then add more layers that end up with
										// the same IDs as the first set. 4 bytes.
	UNICODE_ALPHA_NAMES		= 0x0415,	// Unicode Alpha Names. 4 bytes for length and the
										// string as a unicode string.
	INDEX_TABLE_COLOR_COUNT	= 0x0416,	// Indexed Color Table Count. 2 bytes for the number of
										// colors in table that are actually defined
	TRANSPARENT_INDEX		= 0x0417,	// Tansparent Index. 2 bytes for the index of transparent
										// color, if any.
	GLOBAL_ALTITUDE			= 0x0419,	// Global Altitude. 4 byte entry for altitude
	SLICES					= 0x041A,	// Slices. See description later in this chapter
	WORKFLOW_URL			= 0x041B,	// Workflow URL. Unicode string, 4 bytes of length
										// followed by unicode string.
	JUMP_TO_XPEP			= 0x041C,	// Jump To XPEP. 2 bytes major version, 2 bytes minor
										// version, 4 bytes count. Following is repeated for
										// count: 4 bytes block size, 4 bytes key,
										// if key = 'jtDd' then next is a Boolean for the dirty
										// flag otherwise it's a 4 byte entry for the mod date.
	ALPHA_IDENTIFIERS		= 0x041D,	// Alpha Identifiers. 4 bytes of length, followed by
										// 4 bytes each for every alpha identifier.
	ULR_LIST				= 0x041E,	// URL List. 4 byte count of URLs, followed by 4 byte
										// long, 4 byte ID, and unicode string for each count.
	VERSION_INFO			= 0x0421,	// Version Info. 4 byte version,
										// 1 byte HasRealMergedData, unicode string of writer
										// name, unicode string of reader name, 4 bytes of
										// file version.
	PATH_INFO				= 0x07D0,	// ­0x0BB6 Path Information (saved paths).
										// See path resource format later in this chapter.
	CLIPPING_PATH_NAME		= 0x0BB7,	// Name of clipping path. See path resource format
										// later in this chapter.
	PRINT_FLAGS_INFO		= 0x2710,	// Print flags information. 2 bytes version (=1),
										// 1 byte center crop marks, 1 byte (=0), 4 bytes
										// bleed width value, 2 bytes bleed width scale.

};

struct resource_info {
	int16		id;			// unique identifier
	char*		name;		// 0 terminated, padded to be even name of resource (0 name = 2 bytes 0)
	int32		size;		// size of resource data, not including name and id fields
	void*		data;		// actual resource data padded to make size even
};

struct grid_guide_header {
	int32		version;	// = 1 for Photoshop 4.0.
	int64		grid_cycle;	// Future implementation of document­specific grids.
							// Initially, set the grid cycle to every quarter inch.
							// At 72 dpi, that would be 18 * 32 = 576 (0x240).
	int32		guide_count;// Can be 0. Otherwise, number of guide resource blocks.
};

struct guide_resource_block {
	int32		location;	// Location of guide in document coordinates. Since
							// the guide is either vertical or horizontal,
							// this only has to be one component of the coordinate.
	uint8		direction;	// Direction of guide. This is a system type of unsigned char
							// where 0 = vertical, 1 = horizontal.
};

/*
Thumnail resource header (2-5):
4 bytes		format			= 1 (kJpegRGB). Also supports kRawRGB (0).
4 bytes		width			Width of thumbnail in pixels.
4 bytes		height			Height of thumbnail in pixels.
4 bytes		widthbytes		Padded row bytes as (width * bitspixel + 31) / 32 * 4.
4 bytes		size			Total size as widthbytes * height * planes
4 bytes		compressedsize	Size after compression. Used for consistentcy check.
2 bytes		bitspixel		= 24. Bits per pixel.
2 bytes		planes			= 1. Number of planes.
Variable	Data			JFIF data in RGB format.

Note: For resource ID 1033 the data is in BGR format.
*/

// Photoshop file format:
// file header					(fixed length)
// color mode data				(variable length)
// image resources				(variable length)
// layer and mask information	(variable)
// image data					(variable)


struct file_header {
	int32		signature;		// Always equal to 8BPS. Do not try to read the
								// file if the signature does not match this value.
	int16		version;		// Always equal to 1. Do not try to read the file
								// if the version does not match this value.
	uint8		reserved[6];	// Reserved    Must be zero.
	int16		channels;		// The number of channels in the image, including
								// any alpha channels. Supported range is 1 to 24.
	int32		rows;			// The height of the image in pixels.
								// Supported range is 1 to 30,000.
	int32		columns;		// The width of the image in pixels.
								// Supported range is 1 to 30,000.
	int16		depth;			// The number of bits per channel.
								// Supported values are 1, 8, and 16.
	int16		mode;			// The color mode of the file.
								// Supported values are: Bitmap=0; 
								// Grayscale=1; Indexed=2; RGB=3; CMYK=4;
								// Multichannel=7; Duotone=8; Lab=9.
};

/*
Color mode data section
Only indexed color and duotone have color mode data. For all other modes, 
this section is just 4 bytes: the length field, which is set to zero.
For indexed color images, the length will be equal to 768, and the color data
will contain the color table for the image, in non–interleaved order.
For duotone images, the color data will contain the duotone specification, 
the format of which is not documented. Other applications that read 
Photoshop files can treat a duotone image as a grayscale image, and just 
preserve the contents of the duotone information when reading and writing
the file.

Color mode data (2-13):
4 bytes		Length				The length of the following color data.
Variable	Color data			The color data.

Image resources section
The third section of the file contains image resources. As with the color 
mode data, the section is indicated by a length field followed by the data. 
The image resources in this data area are described in detail earlier in this 

Image resources (2-14):
4 bytes		Length				Length of image resource section.
Variable	Resources			Image resources.

Layer and mask information section
The fourth section contains information about Photoshop 3.0 layers and 
masks. The formats of these records are discussed later in this chapter. If 
there are no layers or masks, this section is just 4 bytes: the length field, 
which is set to zero.

Layer and mask information (2-15):
4 bytes		Length				Length of the miscellaneous information section.
Variable	Layers				Layer info. See table 2–15.
Variable	Global layer mask	Global layer mask info. See table 2–22.

Image data section
The image pixel data is the last section of a Photoshop 3.0 file. Image data is 
stored in planar order, first all the red data, then all the green data, etc. 
Each plane is stored in scanline order, with no pad bytes.
If the compression code is 0, the image data is just the raw image data.
If the compression code is 1, the image data starts with the byte counts for 
all the scan lines (rows * channels), with each count stored as a two–byte 
value. The RLE compressed data follows, with each scan line compressed 
separately. The RLE compression is the same compression algorithm used by 
the Macintosh ROM routine PackBits, and the TIFF standard.

Image data (2-16):
2 bytes		Compression			Compression method. Raw data = 0, RLE compressed = 1.
Variable	Data				The image data. Planar order = RRR GGG BBB, etc.



Layer and mask records
Information about each layer and mask in a document is stored in the fourth 
section of the file. The complete, merged image data is not stored here; it 
resides in the last section of the file.
The first part of this section of the file contains layer information, which is 
divided into layer structures and layer pixel data, as shown in table 2–15. The 
second part of this section contains layer mask data, which is described in 

Layer info section (2-17):
4 bytes		Length				Length of the layers info section, rounded up to a multiple of 2.
Variable	Layers structure	Data about each layer in the document. See table 2–16.
Variable	Pixel data			Channel image data for each channel in the order listed in the 

Layer structure (2-18):
2 bytes		Count				Number of layers. If <0, then number of layers
								is absolute value, and the first alpha channel
								contains the transparency data for the merged result.
Variable	Layer				Information about each layer. See table 2–17.


Layer records (2-19):
4 bytes		Layer top				The rectangle containing the contents of the layer.
4 bytes		Layer left
4 bytes		Layer bottom
4 bytes		Layer right
2 bytes		Number channels			The number of channels in the layer.
Variable	Channel length info		Channel information. This contains a six byte record
									for each channel. See table 2–18.
4 bytes		Blend mode signature	Always 8BIM.
4 bytes		Blend mode key			'norm' = normal
									'dark' = darken
									'lite' = lighten
									'hue ' = hue
									'sat ' = saturation
									'colr' = color
									'lum ' = luminosity
									'mul ' = multiply
									'scrn' = screen
									'diss' = dissolve
									'over' = overlay
									'hLit' = hard light
									'sLit' = soft light
									'diff' = difference
									'smud' = exlusion
									'div ' = color dodge
									'idiv' = color burn
1 byte		Opacity					0 = transparent ... 255 = opaque
1 byte		Clipping				0 = base, 1 = non–base
1 byte		Flags					bit 0 = transparency protected
									bit 1 = visible
									bit 2 = obsolete
									bit 3 = 1 for Photoshop 5.0 and later, tells if 
									bit 4 has useful information
									bit 4 = pixel data irrelevant to appearance of document
1 byte		(filler)				(zero)
4 bytes		Extra data size			Length of the extra data field. This is the total
									length of the next five fields.
24 bytes,	Layer mask data			See table 2–19.
or 4 bytes
if no layer
mask.

Variable	Layer blending ranges	See table 2–20.
Variable	Layer name				Pascal string, padded to a multiple of 4 bytes.
Variable	Adjustment layer info	Series of tagged blocks containing various
									types of data. See table 2–23, 2–24, 2–29, 2–30, and 2–31.

Channel length info (2-20):
2 bytes		Channel ID				0 = red, 1 = green, etc.
									-1 = transparency mask
									-2 = user supplied layer mask
4 bytes		Length					Length of following channel data.


Layer mask / adjustment layer data (2-21):
4 bytes		Size					Size of the data. This will be either 0x14,
									or zero (in which case the following fields
									are not present).
4 bytes		Top						Rectangle enclosing layer mask.
4 bytes		Left
4 bytes		Bottom
4 bytes		Right
1 byte		Default color			0 or 255
1 byte		Flags					bit 0 = position relative to layer
									bit 1 = layer mask disabled
									bit 2 = invert layer mask when blending
2 bytes		Padding					Zeros



Layer blending ranges data (2-22):
4 bytes		Length					Length of layer blending ranges data
4 bytes		Composite				Contains 2 black values followed by 2 white values.
			gray blend source		Present but irrelevant for Lab & Grayscale.
4 bytes 	Composite				Destination Range
			gray blend destination

4 bytes		First channel			First channel source
			source range
4 bytes		First channel			First channel destination
			destination range

4 bytes		Second channel			Second channel source
			source range
4 bytes		Second channel			Second channel destination
			destination range

...			...						...

4 bytes		Nth channel				Nth channel source
			source range
4 bytes		Nth channel				Nth channel destination
			destination range


Channel image data (2-23):
2 bytes		Compression		0 = Raw Data, 1 = RLE compressed.
Variable	Image data		If the compression code is 0,
							the image data is just the raw image 
							data calculated as
							((LayerBottom-LayerTop)*(LayerRight-LayerLeft)). 
							If the compression code is 1, the image data
							starts with the byte counts for all the scan
							lines in the channel (LayerBottom–LayerTop),
							with each count stored as a two-byte value.
							The RLE compressed data follows, with each scan
							line compressed separately. The RLE compression
							is the same compression algorithm used by the
							Macintosh ROM routine PackBits, and the TIFF standard.

							If the Layer's Size, and therefore the data,
							is odd, a pad byte will be inserted at the end
							of the row.
							New since version 4.0 of Adobe Photoshop:
							If the layer is an adjustment layer, the channel
							data is undefined (probably all white.)


Global layer mask info (2-24):
4 bytes		Length			Length of global layer mask info section.
2 bytes		Overlay			Overlay color space (undocumented).
			color space
8 bytes		Color			4 * 2 byte color components components
2 bytes		Opacity			0 = transparent, 100 = opaque.
1 byte		Kind			0=Color selected - i.e. inverted;
							1=Color protected;128=use value 
							stored per layer. This value is preferred.
							The others are for backward compatibility
							with beta versions.
1 byte		(filler)		(zero)

*/

enum {
	BLEND_NORMAL		= 'norm',
	BLEND_DARKEN		= 'dark',
	BLEND_LIGHTEN		= 'lite',
	BLEND_HUE			= 'hue ',
	BLEND_SATURATION	= 'sat ',
	BLEND_COLOR			= 'colr',
	BLEND_LUMINOSITY	= 'lum ',
	BLEND_MULTIPLY		= 'mul ',
	BLEND_SCREEN		= 'scrn',
	BLEND_DISSOLVE		= 'diss',
	BLEND_OVERLAY		= 'over',
	BLEND_HARD_LIGHT	= 'hLit',
	BLEND_SOFT_LIGHT	= 'sLit',
	BLEND_DIFFERENCE	= 'diff',
	BLEND_EXLUSION		= 'smud',
	BLEND_COLOR_DODGE	= 'div ',
	BLEND_COLOR_BURN	= 'idiv',
};

enum {
	FLAG_TRANSPARENCY_PROTECTED		= 0x00,
	FLAG_VISIBLE					= 0x01,
	FLAG_OBSOLETE					= 0x02,	// layer obsolete or flag?
	FLAG_HAS_USEFUL_INFO			= 0x04,	// 1 for Photoshop 5.0 and later, tells if 
											// bit 4 has useful information
	FLAG_IRRELEVANT_FOR_APPEARANCE	= 0x08,	// pixel data irrelevant to appearance of document
};

enum {
	CLIPPING_BASE		= 0x00,
	CLIPPING_NON_BASE	= 0x01,
};

enum {
	COMPRESSION_RAW		= 0x00,
	COMPRESSION_RLE		= 0x01,
};
/*
// table 2-19
struct layer_records {
	int32					layer_top,
	int32					layer_left,
	int32					layer_bottom,
	int32					layer_right,
	int16					num_channels,
	channel_length_info*	channel_infos,
	uint32					blend_mode_sig,
	uint32					blend_mode_key,
	uint8					opacity,			// 0 = transparent ... 255 = opaque
	uint8					clipping,
	uint8					flags,
	uint8					filler,				// zero
	int32					extra_data_size,	// Length of the extra data field. This is the total
												// length of the next five fields.
	layer_mask_data*		mask,				// See table 2-19 -> 24 bytes or 4 bytes if no layer mask.
	layer_blending_range*	blending_ranges,	// See table 2-20
	char*					name,
	adjustment_layer_info*	adjustment_info		// Series of tagged blocks containing various
												// types of data. See table 2-23, 2-24, 2-29, 2-30, and 2-31.
};
*/
// Read
status_t
PSDFile::Read(BPositionIO* stream, Canvas** canvas)
{
	status_t status = B_BAD_VALUE;
	if (stream && canvas) {
	}
	return status;
}

// Write
status_t
PSDFile::Write(BPositionIO* stream, const Canvas* canvas)
{
	status_t status = B_BAD_VALUE;
	if (stream/* && canvas*/) {
		uint32 length;
		int32 width = 800;//canvas->Bounds().IntegerHeight() + 1;
		int32 height = 600;//canvas->Bounds().IntegerWidth() + 1;
		// header
		file_header header;
		header.signature = B_HOST_TO_BENDIAN_INT32(kPhotoshopSig);
		header.version = B_HOST_TO_BENDIAN_INT16(1);
		for (int32 i = 0; i < 6; i++)
			header.reserved[i] = 0;
		header.channels = B_HOST_TO_BENDIAN_INT16(4);
		header.rows = B_HOST_TO_BENDIAN_INT32(height);
		header.columns =  B_HOST_TO_BENDIAN_INT32(width);
		header.depth = B_HOST_TO_BENDIAN_INT16(8);
		header.mode = B_HOST_TO_BENDIAN_INT16(3);
//		stream->Write((void*)&header, sizeof(header));
		_WriteHeader(stream, header);
		// color mode data
		length = 0;
		stream->Write((void*)&length, sizeof(length));
		// image resources
		length = 0;
		stream->Write((void*)&length, sizeof(length));
		// layer and masks
		length = 0;
		stream->Write((void*)&length, sizeof(length));
/*		off_t layerLengthOffset = stream->Position();
		stream->SeekTo(layerLengthOffset + 4);
		for (int32 i = 0; Layer* layer = canvas->LayerAt(i); i++) {
			layer_info info;
			info
			if (BBitmap* bitmap = layer->Bitmap()) {
				ssize_t bitmapLength = bitmap->BitsLength();
				stream->Write(bitmap->Bits(), bitmapLength);
				length += bitmapLength;
			}
		}*/
		// image data
//		length = B_HOST_TO_BENDIAN_INT32(2 + width * height * 4);
//		stream->Write((void*)&length, sizeof(length));
		int16 compression = B_HOST_TO_BENDIAN_INT16(COMPRESSION_RAW);
		stream->Write((void*)&compression, sizeof(compression));
		void* scanlineBuffer = malloc(width * 4);
		if (scanlineBuffer) {
			for (int32 line = 0; line < height; line++) {
//				memcpy();
				memset(scanlineBuffer, 255, width * 4);
				stream->Write((void*)scanlineBuffer, width * 4);
			}
			free(scanlineBuffer);
		} else
			status = B_NO_MEMORY;
	}
	return status;
}

status_t
PSDFile::_WriteHeader(BPositionIO* stream, file_header& header) const
{
	stream->Write(&header.signature, 4);
	stream->Write(&header.version, 2);
	stream->Write(&header.reserved, 6);
	stream->Write(&header.channels, 2);
	stream->Write(&header.rows, 4);
	stream->Write(&header.columns, 4);
	stream->Write(&header.depth, 2);
	stream->Write(&header.mode, 2);
	return B_OK;
}
