// YBMScrollView.h

#ifndef YB_M_SCROLL_VIEW_H
#define YB_M_SCROLL_VIEW_H

#include <layout.h>
#include "ScrollView.h"

class YBMScrollView : public MView, public ScrollView {
 public:
								YBMScrollView(BView* child,
											uint32 scrollingFlags,
											const char *name);

// MView
					minimax		layoutprefs();
					BRect		layout(BRect rect);
};



#endif	// YB_M_SCROLL_VIEW_H
