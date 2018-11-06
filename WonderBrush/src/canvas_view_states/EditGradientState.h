// EditGradientState.h

#ifndef EDIT_GRADIENT_STATE_H
#define EDIT_GRADIENT_STATE_H

#include "CanvasViewState.h"
#include "ObjectObserver.h"

class Gradient;
class GradientBox;
class GradientControl;
class HistoryManager;
class MasterGradientAction;
class Stroke;

class EditGradientState : public CanvasViewState,
						  public ObjectObserver {
 public:
								EditGradientState(CanvasView* parent);
	virtual						~EditGradientState();

								// CanvasViewState
	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
	virtual	void				MouseUp(BPoint where, Point canvasWhere);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				ModifierSelectionChanged(bool itemsSelected);

	virtual	void				Draw(BView* into, BRect updateRect);
	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);
	virtual	void				UpdateToolCursor();
	virtual	bool				RebuildLayer(Layer* layer, BRect area) const;

	virtual	void				PrepareForObjectPropertyChange();
	virtual	void				ObjectPropertyChangeDone();

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// EditGradientState
			void				SetGradientControl(GradientControl* control);
			void				SetGradient(const Gradient* gradient);
 private:
	friend	class GradientBox;

			void				_Perform();
			void				_Cancel();
			void				_SetConfirmationEnabled(bool enable) const;

			void				_InitPrivateHistory();

			void				_UpdateControls();

			void				_SetModifier(Stroke* object);
			void				_SetBox(GradientBox* box);

			void				_RebuildLayer(BRect r, bool ignorBusy = false) const;

			BRect				_ControlPointRect() const;

			Canvas*				fCanvas;
			Layer*				fLayer;
			HistoryManager*		fPrivateHistory;
			MasterGradientAction* fGradientAction;

			Stroke*				fObject;

			Gradient*			fGradient;
			GradientControl*	fControl;

			GradientBox*		fBox;
			BPoint				fLastCanvasPos;

			bool				fCancelling;
			bool				fItemsSelected;
			bool				fGenerateAction;
};

#endif	// EDIT_GRADIENT_STATE_H
