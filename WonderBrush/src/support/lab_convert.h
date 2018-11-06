// lab_convert.h

#ifndef LAB_CONVERT_H
#define LAB_CONVERT_H

#include <SupportDefs.h>

void lab2rgb(float L, float a, float b, uint8& R, uint8& G, uint8& B);

// lab2rgb
inline void
lab2rgb(uint8 L, uint8 a, uint8 b, uint8& R, uint8& G, uint8& B)
{
	float CIEL = ((float)L / 255.0) * 100.0;
	float CIEa = ((float)a - 128.0);
	float CIEb = ((float)b - 128.0);
//	float CIEa = ((float)a / 255.0) * (87.313422 + 99.039299) - 87.313422;
//	float CIEb = ((float)b / 255.0) * (107.861747 + 94.475782) - 107.861747;
	lab2rgb(CIEL, CIEa, CIEb, R, G, B);
}

void rgb2lab(uint8 R, uint8 G, uint8 B, float& L, float& a, float& b);

// rgb2lab
inline void
rgb2lab(uint8 R, uint8 G, uint8 B, uint8& L, uint8& a, uint8& b)
{
	float CIEL, CIEa, CIEb;
	rgb2lab(R, G, B, CIEL, CIEa, CIEb);
	L = uint8((CIEL / 100.0) * 255.0);
//	a = uint8(((CIEa + 87.313422) / (87.313422 + 99.039299)) * 255.0);
//	b = uint8(((CIEb + 107.861747) / (107.861747 + 94.475782)) * 255.0);
	a = uint8(CIEa + 128);
	b = uint8(CIEb + 128);
}

void replace_luminance(uint8& r1, uint8& g1, uint8& b1, uint8 r2, uint8 g2, uint8 b2);

#endif // LAB_CONVERT_H
