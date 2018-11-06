// NummericalTextView.h

#ifndef NUMMERICAL_TEXT_VIEW_H
#define NUMMERICAL_TEXT_VIEW_H

#include <Invoker.h>
#include <TextView.h>

class NummericalTextView : public BTextView,
						   public BInvoker {
 public:
								NummericalTextView(BRect frame,
												   const char* name,
												   BRect textRect,
												   uint32 resizingMode,
												   uint32 flags);
	virtual						~NummericalTextView();

								// BTextView
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);

	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MakeFocus(bool focus);

								// BInvoker
	virtual	status_t			Invoke(BMessage* message = NULL);

								// NummericalTextView
			void				SetFloatMode(bool floatingPoint);

			void				SetValue(int32 value);
			void				SetValue(float value);
			int32				IntValue() const;
			float				FloatValue() const;

protected:
								// BTextView
	virtual	void				InsertText(const char* inText,
										   int32 inLength,
										   int32 inOffset,
										   const text_run_array* inRuns);
	virtual	void				DeleteText(int32 fromOffset,
										   int32 toOffset);

								// NummericalTextView
			void				_ToggleAllowChar(char c);
			void				_CheckMinusAllowed();
			void				_CheckDotAllowed();

			void				_ValueChanged();

			bool				fFloatMode;

	mutable	int32				fIntValueCache;
	mutable	float				fFloatValueCache;

			bool				fWasFocus;
};

#endif // NUMMERICAL_TEXT_VIEW_H


