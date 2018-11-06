// LayerContainer.h

#ifndef LAYER_CONTAINER_H
#define LAYER_CONTAINER_H

#include <List.h>
#include <Locker.h>
#include <Rect.h>
#include <String.h>

struct entry_ref;

class BBitmap;
class BMessage;
class HistoryManager;
class Layer;

class LayerContainer : public BList, public BLocker {
 public:
								LayerContainer(BRect frame);
	virtual						~LayerContainer();

	virtual	bool				IsValid() const;
	virtual	void				MakeEmpty();

	// use as little memory as possible
	virtual	void				Minimize(bool doIt);
	virtual	bool				IsMinimized() const;

	// list functionality
			bool				AddLayer(Layer* layer);
			bool				AddLayer(Layer* layer, int32 index);

			Layer*				RemoveLayer(int32 index);
			bool				RemoveLayer(Layer* layer);

			Layer*				LayerAt(int32 index) const;
			int32				IndexOf(Layer* layer) const;
			int32				CountLayers() const;
			bool				HasLayer(Layer* layer) const;

			void				SetBounds(BRect bounds);
			BRect				Bounds() const;

								// composes layers ontop of passed bitmap
	virtual	void				Compose(BBitmap* into, BRect area) const;
								// returns entire composition in new bitmap
			BBitmap*			Bitmap() const;

	// loading / saving
	virtual	status_t			Archive(BMessage* into) const;
	virtual	status_t			Unarchive(BMessage* archive);

 private:
	BRect						fBounds;
	bool						fMinimized;
};

#endif // CANVAS_H
