// GradientAction.h

#ifndef GRADIENT_ACTION_H
#define GRADIENT_ACTION_H

#include "Action.h"

class EditGradientState;
class Stroke;

class GradientAction : public Action {
 public:
								GradientAction(EditGradientState* state,
											   Stroke* object);
	virtual						~GradientAction();
	
	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			void				_GetObjectBounds(BRect& renderbounds,
												 BRect& invalidationBounds) const;
			void				_RedrawObject(BRect renderbounds,
											  BRect invalidationBounds) const;

			EditGradientState*	fState;
			Stroke*				fObject;
};

#endif // GRADIENT_ACTION_H
