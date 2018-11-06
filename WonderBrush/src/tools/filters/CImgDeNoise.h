#ifndef C_IMG_DE_NOISE_H
#define C_IMG_DE_NOISE_H

#include "FilterObject.h"

class CImgDeNoise : public FilterObject {
 public:
								CImgDeNoise();
								CImgDeNoise(const CImgDeNoise& other);
								CImgDeNoise(BMessage* archive);
	virtual						~CImgDeNoise();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const;

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

 protected:
			float				fAmplitude;
			float				fSharpness;
			float				fAnisotropy;
			float				fAlpha;
			float				fSigma;
			float				fDL; // "spatial discretization"
			float				fDA; // "angular discretization"
			float				fGaussPrecision;
			uint32				fInterpolationMode;
									// 0 = nearest-neighbor
									// 1 = linear
									// 2 = Runge-Kutta
			bool				fFastApproximation;
};

#endif	// C_IMG_DE_NOISE_H
