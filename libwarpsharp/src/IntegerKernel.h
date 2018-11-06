// IntegerKernel.h

#ifndef INTEGER_KERNEL_H
#define INTEGER_KERNEL_H

#include <SupportDefs.h>

#include "warpsharp.h"

typedef double (*kernel_func)(double x);

class Gray8Image;

class IMPEXPLIBWARPSHARP IntegerKernel {
 public:
	enum {
		KERNEL_GENERAL		= 0,
		KERNEL_SYMETRIC		= 1,
		KERNEL_ANTISYMETRIC	= 2,
	};
								IntegerKernel();
								IntegerKernel(kernel_func funtion,
											  int32 radius,
											  bool horizontal);
								IntegerKernel(int32* element,
											  int32 direction,
											  bool horizontal,
											  uint32 symetry,
											  int32 scale);
								IntegerKernel(IntegerKernel& from);
	virtual						~IntegerKernel();

			bool				IsValid() const;

	inline	int32*				Element() const
									{ return fElement; }
	inline	void				SetSymetry(uint32 symetry)
									{ fSymetry = symetry; }
	inline	uint32				Symetry() const
									{ return fSymetry; }
	inline	int32				Radius() const
									{ return fDiameter / 2; }
	inline	int32				Diameter() const
									{ return fDiameter; }
	inline	bool				IsHorizontal() const
									{ return fHorizontal; }
	inline	int32				Scale() const
									{ return fScale; }

	inline	int32				operator[](uint32 i)
									{ return fElement[i]; }
			IntegerKernel&		operator=(IntegerKernel& from);
			Gray8Image*			operator*(Gray8Image* from) const;
	inline						operator int32*()
									{ return fElement; }

								// debugging
			void				PrintToStream();

 private:
			int32*				fElement;
			int32				fDiameter;
			bool				fHorizontal;
			uint32				fSymetry;
			int32				fScale; // all elements are scaled so as to allow integer values.
};

#endif // INTEGER_KERNEL_H
