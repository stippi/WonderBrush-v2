// TransformAction.h

#ifndef TRANSFORM_ACTION_H
#define TRANSFORM_ACTION_H

#include <String.h>

#include "Action.h"
#include "Transformable.h"

class TransformAction : public Action {
 public:
//								TransformAction(BPoint translation,
//												double rotation,
//												double xScale,
//												double yScale,
								TransformAction(const Transformable& t,
												BPoint centerOffset,

												const char* actionName,
												uint32 nameIndex);

	virtual						~TransformAction();
	
	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

								// TransformAction
//			void				SetNewTransformation(BPoint translation,
//													 double rotation,
//													 double xScale,
//													 double yScale,
//													 BPoint centerOffset);
			void				SetNewTransformation(const Transformable& t,
													 BPoint centerOffset);

			void				SetNewTranslation(BPoint translation);

			void				SetName(const char* actionName, uint32 nameIndex);

 protected:
	virtual	status_t			_SetTransformation(CanvasView* view,
//												   BPoint translation,
//												   double rotation,
//												   double xScale,
//												   double yScale,
												   const Transformable& t,
												   BPoint centerOffset) const = 0;

//			BPoint				fOldTranslation;
//			double				fOldRotation;
//			double				fOldXScale;
//			double				fOldYScale;
			Transformable		fOldTransform;

			BPoint				fOldCenterOffset;

//			BPoint				fNewTranslation;
//			double				fNewRotation;
//			double				fNewXScale;
//			double				fNewYScale;
			Transformable		fNewTransform;

			BPoint				fNewCenterOffset;

			BString				fName;
			uint32				fNameIndex;
};

#endif // TRANSFORM_ACTION_H
