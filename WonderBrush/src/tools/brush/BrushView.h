// BrushView.h

#ifndef BRUSH_VIEW_H
#define BRUSH_VIEW_H

#include <Invoker.h>
#include <View.h>

#include <layout.h>

class BBitmap;
class Brush;

class BrushView : public MView, public BView, public BInvoker {
 public:
								BrushView(const char* name,
										  BMessage* message,
										  const BHandler* target);
	virtual						~BrushView();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BView
	virtual	void				FrameResized(float width, float height);
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);

								// BrushView
			void				Update(const Brush* brush,
									   uint32 flags);
			void				Update();

			void				SetEnabled(bool enable);

 private:
	BBitmap*					fAlphaBitmap;
	BBitmap*					fDisplayBitmap;
	Brush*						fBrush;
	uint32						fFlags;
	float						fRadiusCache;
	bool						fEnabled;
};

#endif // BRUSH_VIEW_H
