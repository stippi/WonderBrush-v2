// BlurFilter.h

#ifndef BLUR_FILTER_H
#define BLUR_FILTER_H

#include <SupportDefs.h>

class BBitmap;

class BlurFilter {
 public:
								BlurFilter();
	virtual						~BlurFilter();

	status_t					Run(const BBitmap* input,
									const BBitmap* output) const;
};

#endif	// BLUR_FILTER_H
