// LayerContainer.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <Entry.h>
#include <Message.h>

#include "defines.h"

#include "Layer.h"

#include "LayerContainer.h"

// constructor
LayerContainer::LayerContainer(BRect frame)
	: BList(10),
	  BLocker("canvas lock"),
	  fBounds(frame),
	  fMinimized(false)
{
}

// destructor
LayerContainer::~LayerContainer()
{
	int32 count = CountItems();
	while (Layer* layer = LayerAt(--count))
		delete layer;
}

// IsValid
bool
LayerContainer::IsValid() const
{
	return fBounds.IsValid();
}

// MakeEmpty
void
LayerContainer::MakeEmpty()
{
	if (Lock()) {
		for (int32 i = 0; Layer* layer = LayerAt(i); i++)
			delete layer;
		BList::MakeEmpty();
		Unlock();
	}
}

// Minimize
void
LayerContainer::Minimize(bool doIt)
{
	if (fMinimized != doIt) {
		for (int32 i = 0; Layer* layer = LayerAt(i); i++) {
			layer->Minimize(doIt);
		}
		fMinimized = doIt;
	}
}

// IsMinimized
bool
LayerContainer::IsMinimized() const
{
	return fMinimized;
}

// AddLayer
bool
LayerContainer::AddLayer(Layer* layer)
{
	if (layer) {
		layer->AttachedToContainer(this);
		return AddItem((void*)layer);
	}
	return false;
}

// AddLayer
bool
LayerContainer::AddLayer(Layer* layer, int32 index)
{
	if (layer) {
		layer->AttachedToContainer(this);
		return AddItem((void*)layer, index);
	}
	return false;
}

// RemoveLayer
Layer*
LayerContainer::RemoveLayer(int32 index)
{
	Layer* layer = (Layer*)RemoveItem(index);
	if (layer)
		layer->DetachedFromContainer();
	return layer;
}

// RemoveLayer
bool
LayerContainer::RemoveLayer(Layer* layer)
{
	if (RemoveItem((void*)layer)) {
		layer->DetachedFromContainer();
		return true;
	}
	return false;
}

// LayerAt
Layer*
LayerContainer::LayerAt(int32 index) const
{
	return (Layer*)ItemAt(index);
}

// IndexOf
int32
LayerContainer::IndexOf(Layer* layer) const
{
	return BList::IndexOf((void*)layer);
}

// CountLayers
int32
LayerContainer::CountLayers() const
{
	return CountItems();
}

// HasLayer
bool
LayerContainer::HasLayer(Layer* layer) const
{
	return HasItem((void*)layer);
}

// SetBounds
void
LayerContainer::SetBounds(BRect bounds)
{
	if (bounds.IsValid())
		fBounds = bounds;
}

// Bounds
BRect
LayerContainer::Bounds() const
{
	return fBounds;
}

// Compose
void
LayerContainer::Compose(BBitmap* into, BRect area) const
{
	if (into && into->IsValid()
		&& area.IsValid() && area.Intersects(into->Bounds())) {
		area = area & into->Bounds();
		int32 count = CountLayers();
		for (int32 i = count - 1; Layer* layer = LayerAt(i); i--) {
			layer->Compose(into, area);
		}
	}
}

// Bitmap
BBitmap*
LayerContainer::Bitmap() const
{
	BBitmap* bitmap = new BBitmap(fBounds, 0, B_RGBA32);
	if (bitmap->IsValid()) {
		// this bitmap is uninitialized, clear to black/fully transparent
		memset(bitmap->Bits(), 0, bitmap->BitsLength());
		Compose(bitmap, fBounds);
		// remove image data where alpha = 0 to improve compression later on
		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		uint32 width = bitmap->Bounds().IntegerWidth() + 1;
		uint32 height = bitmap->Bounds().IntegerHeight() + 1;
		while (height > 0) {
			uint8* bitsHandle = bits;
			for (uint32 x = 0; x < width; x++) {
				if (!bitsHandle[3]) {
					bitsHandle[0] = 0;
					bitsHandle[1] = 0;
					bitsHandle[2] = 0;
				}
				bitsHandle += 4;
			}
			bits += bpr;
			height--;
		}
	} else {
		delete bitmap;
		bitmap = NULL;
	}
	return bitmap;
}



static const char*	LAYER_KEY			= "layer";
static const char*	BOUNDS_KEY			= "bounds";

// Archive
status_t
LayerContainer::Archive(BMessage* into) const
{
	status_t status = B_BAD_VALUE;
	if (into) {
		// add each layer
		for (int32 i = 0; Layer* layer = LayerAt(i); i++) {
			BMessage layerArchive;
			if ((status = layer->Archive(&layerArchive)) >= B_OK) {
				if ((status = into->AddMessage(LAYER_KEY, &layerArchive)) < B_OK)
					break;
			}
		}
		// add our own properties
		if (status >= B_OK)
			status = into->AddRect(BOUNDS_KEY, fBounds);
	}
	return status;
}

// Unarchive
status_t
LayerContainer::Unarchive(BMessage* archive)
{
	status_t status = B_BAD_VALUE;
	if (archive) {
		// restore bounds
		BRect bounds;
		if ((status = archive->FindRect(BOUNDS_KEY, &bounds)) == B_OK) {
			fBounds = bounds;
			// restore each layer
			BMessage layerMessage;
			for (int32 i = 0; archive->FindMessage(LAYER_KEY, i, &layerMessage) == B_OK; i++) {
				Layer* layer = new Layer();
				if ((status = layer->Unarchive(&layerMessage)) < B_OK
					|| !AddLayer(layer)) {
					// if AddLayer() failed:
					if (status >= B_OK)
						status = B_NO_MEMORY;
					// in any case:
					delete layer;
					break;
				}
			}
		}
	}
	return status;
}

