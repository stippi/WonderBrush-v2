// ReportingTextView.h

#ifndef REPORTING_TEXT_VIEW_H
#define REPORTING_TEXT_VIEW_H

#include <Invoker.h>

#include <MTextView.h>

class ReportingTextView : public BInvoker,
						  public MTextView {
 public:
								ReportingTextView(minimax size = 0);
	virtual						~ReportingTextView();

								// BTextView
	virtual	void				AttachedToWindow();
//	virtual	void				MouseDown(BPoint where);
//	virtual	void				MouseUp(BPoint where);

//	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MakeFocus(bool focus);
	virtual	void				WindowActivated(bool active);

								// BInvoker
	virtual	status_t			Invoke(BMessage* message = NULL);

								// ReportingTextView
			void				SetModificationMessage(BMessage* message);
			BMessage*			ModificationMessage() const
									{ return fTextChangedMessage; }

			void				SetEnabled(bool enable);
			bool				IsEnabled() const
									{ return fEnabled; }


protected:
								// BTextView
	virtual	void				InsertText(const char* inText,
										   int32 inLength,
										   int32 inOffset,
										   const text_run_array* inRuns);
	virtual	void				DeleteText(int32 fromOffset,
										   int32 toOffset);

								// ReportingTextView

			BMessage*			fTextChangedMessage;
			bool				fWasFocus;
			bool				fEnabled;
};

#endif // REPORTING_TEXT_VIEW_H


