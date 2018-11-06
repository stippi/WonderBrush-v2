// PathAction.h

#ifndef PATH_ACTION_H
#define PATH_ACTION_H

#include "Action.h"

class ShapeState;
class ShapeStroke;
class VectorPath;

class PathAction : public Action {
 public:
								PathAction(ShapeState* state,
										   ShapeStroke* modifier);
	virtual						~PathAction();
	
	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			void				_GetPathBounds(BRect& pathBounds,
											   BRect& controlPointBounds) const;
			void				_RedrawPath(BRect pathBounds,
											BRect controlPointBounds) const;
			void				_Select(const int32* indices, int32 count,
										bool extend = false) const;

			ShapeState*			fShapeState;
			ShapeStroke*		fShapeModifier;
			VectorPath*			fPath;
};

#endif // PATH_ACTION_H
