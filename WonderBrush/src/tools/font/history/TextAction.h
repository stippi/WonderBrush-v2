// TextAction.h

#ifndef TEXT_ACTION_H
#define TEXT_ACTION_H

#include "Action.h"

class TextState;
class TextStroke;

class TextAction : public Action {
 public:
								TextAction(TextState* state,
										   TextStroke* modifier);
	virtual						~TextAction();
	
	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			void				_GetModifierBounds(BRect& renderbounds,
												   BRect& invalidationBounds) const;
			void				_RedrawModifier(BRect renderbounds,
												BRect invalidationBounds) const;

			TextState*			fTextState;
			TextStroke*			fTextModifier;
};

#endif // TEXT_ACTION_H
