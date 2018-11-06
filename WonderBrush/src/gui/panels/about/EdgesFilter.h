// EdgesFilter.h

#ifndef EDGES_FILTER_H
#define EDGES_FILTER_H

#include <SupportDefs.h>

class BBitmap;

class EdgesFilter {
 public:
								EdgesFilter();

	virtual						~EdgesFilter();

			status_t			Run(const BBitmap* input,
									const BBitmap* output) const;
};

#endif	// EDGES_FILTER_H
