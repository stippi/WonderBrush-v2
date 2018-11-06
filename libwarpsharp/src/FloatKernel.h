// FloatKernel.h

#ifndef FLOAT_KERNEL_H
#define FLOAT_KERNEL_H

#include <SupportDefs.h>

#include "warpsharp.h"

typedef double (*kernel_func)(double x);

class FloatImage;

class IMPEXPLIBWARPSHARP FloatKernel {
 public:
	enum {
		KERNEL_GENERAL		= 0,
		KERNEL_SYMETRIC		= 1,
		KERNEL_ANTISYMETRIC	= 2,
	};
								FloatKernel();
								FloatKernel(kernel_func funtion,
											int32 radius,
											bool horizontal);
								FloatKernel(float* element,
											int32 direction,
											bool horizontal,
											uint32 symetry);
								FloatKernel(FloatKernel& from);
	virtual						~FloatKernel();

			bool				IsValid() const;

	inline	float*				Element() const
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

	inline	float				operator[](uint32 i)
									{ return fElement[i]; }
			FloatKernel&		operator=(FloatKernel& from);
			FloatImage*			operator*(FloatImage* from) const;
	inline						operator float*()
									{ return fElement; }

								// debugging
			void				PrintToStream() const;

 private:
			float*				fElement;
			int32				fDiameter;
			bool				fHorizontal;
			uint32				fSymetry;
};

#endif // FLOAT_KERNEL_H
