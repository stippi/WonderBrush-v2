// DimensionsControl.h

#ifndef DIMENIONS_CONTROL_H
#define DIMENIONS_CONTROL_H

#include <MBorder.h>
#include <MDividable.h>

class BHandler;
class BMessage;
class BubbleHelper;
class LockView;
class MDividable;
class MPopup;
class MTextControl;

class DimensionsControl : public MBorder {
 public:
								DimensionsControl(BMessage* message = NULL,
												  BHandler* handler = NULL);
	virtual						~DimensionsControl();

								// BHandler
	virtual	void				AttachedToWindow();
	virtual void				MessageReceived(BMessage *message);

	virtual	minimax				layoutprefs();

								// DimensionsControl
			void				SetDimensions(uint32 width,
											  uint32 height);
			void				SetWidth(uint32 width);
			void				SetHeight(uint32 height);
			uint32				Width() const;
			uint32				Height() const;
			void				SetWidthLimits(uint32 min, uint32 max);
			void				SetHeightLimits(uint32 min, uint32 max);

			void				SetProportionsLocked(bool lock);
			bool				IsProportionsLocked() const;

			void				SetEnabled(bool enabled);
			bool				IsEnabled() const;

			void				SetLabels(const char* width,
										  const char* height);

			MTextControl*		WidthControl() const;
			MTextControl*		HeightControl() const;

 private:
			uint32				GetLockedWidthFor(uint32 newHeight);
			uint32				GetLockedHeightFor(uint32 newWidth);
			void				_SetDimensions(uint32 width,
											   uint32 height,
											   bool sendMessage = true);

 private:
			BMessage*			fMessage;
			BHandler*			fTarget;
			MTextControl*		fWidthTC;
			MTextControl*		fHeightTC;
			LockView*			fLockView;
			MPopup*				fCommonFormatsPU;
			uint32				fPreviousWidth;
			uint32				fPreviousHeight;
			uint32				fMinWidth;
			uint32				fMaxWidth;
			uint32				fMinHeight;
			uint32				fMaxHeight;
};

#endif	// DIMENIONS_CONTROL_H
