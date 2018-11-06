// CanvasTabView.h

#ifndef CANVAS_TAB_VIEW_H
#define CANVAS_TAB_VIEW_H

#include <List.h>
#include <View.h>

#include <layout.h>

class Canvas;

class CanvasTabView : public MView, public BView {
 public:
								CanvasTabView(const char* name,
											  BMessage* message,
											  BHandler* target);
	virtual						~CanvasTabView();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BView
	virtual	void				Draw(BRect updateRect);
	virtual	void				MessageReceived(BMessage* message);

	virtual	void				MouseDown(BPoint where);
//	virtual	void				MouseUp(BPoint where);
//	virtual	void				MouseMoved(BPoint where, uint32 transit,
//										   const BMessage* dragMessage);

								// CanvasTabView
								// window needs to RecalcSize()!
			bool				AddCanvas(Canvas* canvas);
			bool				RemoveCanvas(Canvas* canvas);


			void				SetCanvas(Canvas* canvas);

 private:
			void				_Invoke(const BMessage* message);
			void				_StrokeRect(BRect frame, rgb_color leftTop,
											rgb_color rightBottom);
			BRect				_TabFrame(int32 index);

//			bool				fMouseDown;

			BMessage*			fMessage;
			BHandler*			fTarget;

			BList				fCanvasTabs;
			Canvas*				fCurrentCanvas;
};

#endif // CANVAS_TAB_VIEW_H
