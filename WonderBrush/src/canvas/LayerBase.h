// LayerBase.h

#ifndef LAYER_BASE_H
#define LAYER_BASE_H

#include <Rect.h>
#include <String.h>

// property flags
enum {
	FLAG_INVISIBLE		= 0x01,
};

// blending modes
enum {
	MODE_NORMAL					= 0,	// Never change the value of those constants,
	MODE_MULTIPLY				= 1,	// or you will break compatibility with existing
	MODE_INVERSE_MULTIPLY		= 2,	// project files! Only add at the bottom...
	MODE_LUMINANCE				= 3,
	MODE_MULTIPLY_ALPHA			= 4,
	MODE_MULTIPLY_INVERSE_ALPHA	= 5,

	MODE_REPLACE_RED			= 6,
	MODE_REPLACE_GREEN			= 7,
	MODE_REPLACE_BLUE			= 8,

	MODE_DARKEN					= 9,
	MODE_LIGHTEN				= 10,

	MODE_HARD_LIGHT				= 11,
	MODE_SOFT_LIGHT				= 12,
};

class BBitmap;
class BMessage;
class LayerContainer;

class LayerBase {
 public:
								LayerBase(float alpha = 1.0,
									  uint32 mode = MODE_NORMAL,
									  uint32 flags = 0);
	virtual						~LayerBase();

	virtual	status_t			InitCheck() const;

	virtual	void				AttachedToContainer(LayerContainer* container);
	virtual	void				DetachedFromContainer();

								// this function can be used to
								// minimize the memory usage of
								// the layer to the bare minimum
	virtual	void				Minimize(bool doIt) = 0;
			bool				IsMinimized() const;

	virtual	BBitmap*			Bitmap() const = 0;

	// layer properties
			void				SetName(const char* name);
			const char*			Name() const;
			void				SetAlpha(float alpha);
			float				Alpha() const;
			void				SetMode(uint32 mode);
			uint32				Mode() const;
			void				SetFlags(uint32 flags);
			uint32				Flags() const;

			void				SetCurrent(bool current);
			bool				IsCurrent() const;

	// active area of layer
	virtual	void				Touch(BRect area) = 0;
	virtual	BRect				ActiveBounds() const = 0;

	// composing
	virtual	status_t			Compose(const BBitmap* into,
										BRect area);

	// loading / saving
	virtual	status_t			Archive(BMessage* into) const;
	virtual	status_t			Unarchive(BMessage* archive);

 protected:
			BString				fName;
			float				fAlpha;
			uint32				fMode;
			uint32				fFlags;
			bool				fMinimized;
			// this does not need to be preserved
			bool				fCurrent;
};

#endif // LAYER_BASE_H
