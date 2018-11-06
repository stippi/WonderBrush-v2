#ifndef GAUSS_FILTER
#define GAUSS_FILTER

#include <SupportDefs.h>

class BBitmap;

class GaussFilter {
 public:
								GaussFilter();
								~GaussFilter();

			void				Filter(BBitmap* bitmap, double radius);

 private:
			typedef float calc_type;

			void				_Init(double radius);

 			void				_Filter(calc_type* buffer, int32 count);

			void				_FilterRow4(uint8* buffer, int32 count,
											bool alpha);
			void				_FilterColumn4(uint8* buffer,
									uint32 skip, int32 count,
									bool alpha);

			void				_FilterRow1(uint8* buffer, int32 count);
			void				_FilterColumn1(uint8* buffer,
									uint32 skip, int32 count);

			calc_type			b0;
			calc_type			b1;
			calc_type			b2;
			calc_type			b3;
			calc_type			B;
};

#endif // GAUSS_FILTER
