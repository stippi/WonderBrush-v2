// WarpManager.h

#ifndef WARP_MANAGER_H
#define WARP_MANAGER_H

#include "warpsharp.h"

class BBitmap;
class FloatImage;
class MapImage;
class RGB8Image;
class SparseMatrix;

class IMPEXPLIBWARPSHARP WarpManager {
 public:
								WarpManager();
	virtual						~WarpManager();

			bool				IsValid() const;

			void				MakeEmpty();

								// parameter handling
			void				SetEdgeRadius(uint32 radius);
	inline	uint32				EdgeRadius() const
									{ return fEdgeRadius; }

			void				SetLambda(float lambda);
	inline	float				Lambda() const
									{ return fLambda; }

			void				SetMu(float mu);
	inline	float				Mu() const
									{ return fMu; }

			void				SetScale(float scale);
	inline	float				Scale() const
									{ return fScale; }

			void				SetUseTwoComponents(bool two);
	inline	bool				IsUsingTwoComponents() const
									{ return fUseTwoComponents; }

			void				SetUseNonMaximalSupression(bool nms);
	inline	bool				IsUsingNonMaximalSupression() const
									{ return fUseNMS; }

								// image operations
			bool				SetSharpMap(MapImage* map);
	inline	MapImage*			SharpMap() const
									{ return fMapImage; }

			bool				SetEdgeImage(FloatImage* edges);

			bool				SetInputImage(RGB8Image* input);
			void				UnsetInputImage();

								// most importantly...
			RGB8Image*			GetWarpedImage();
								// for the curious
			FloatImage*			GetEdgeImage();

 private:
			void				_ComputeInitial(float* xInit,
												float* yInit,
												uint32 width,
												uint32 height);
			void				_ComputeRightHand(float* bx,
												  float* by,
												  float mu,
												  FloatImage* xClamp,
												  FloatImage* yClamp);
			void				_ComputeSharpMap(FloatImage* input);
			void				_ComputeEdgeImages(FloatImage* input);
			void				_ComputeSparseMatrices(FloatImage* input);
			void				_DeleteEdgeImages();
			void				_DeleteMapImage();
			void				_DeleteSparseMatrices();

			uint32				fEdgeRadius;
			float				fLambda;
			float				fMu;
			float				fScale;
			bool				fUseTwoComponents;
			bool				fUseNMS;

			RGB8Image*			fInputImage;
			FloatImage*			fInputFloatImage;
			FloatImage**		fEdgeImages;

			MapImage*			fMapImage;

			SparseMatrix*		fGX;
			SparseMatrix*		fGY;

			bool				fDeleteSharpMap;
};

#endif // WARP_MANAGER_H
