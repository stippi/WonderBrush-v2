// NumericalTextControl.h

#ifndef NUMERICAL_TEXT_CONTROL_H
#define NUMERICAL_TEXT_CONTROL_H

#include <MTextControl.h>

class NumericalTextControl : public MTextControl {
 public:
								NumericalTextControl(char* label, char* text);
								NumericalTextControl(char* label, char* text,
													 BMessage* message);
	virtual						~NumericalTextControl();

//	virtual	void				KeyDown(const char* bytes, int32 numBytes);

								// BInvoker
//	virtual	status_t			Invoke(BMessage* message = NULL);

								// NumericalTextControl
			void				SetFloatMode(bool floatingPoint);

			void				SetValue(int32 value);
			void				SetValue(float value);
			void				SetValue(double value);

			int32				IntValue() const;
			float				FloatValue() const;

protected:
								// BTextView
/*	virtual	void				InsertText(const char* inText,
										   int32 inLength,
										   int32 inOffset,
										   const text_run_array* inRuns);
	virtual	void				DeleteText(int32 fromOffset,
										   int32 toOffset);

								// NumericalTextControl
			void				_ToggleAllowChar(char c);
			void				_CheckMinusAllowed();
			void				_CheckDotAllowed();

			void				_ValueChanged();*/
			void				_Init();

			bool				fFloatMode;

	mutable	int32				fIntValueCache;
	mutable	float				fFloatValueCache;

			bool				fWasFocus;
};

#endif // NUMERICAL_TEXT_CONTROL_H


