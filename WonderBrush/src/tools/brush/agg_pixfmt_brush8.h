//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.1
// Copyright (C) 2002-2004 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//		  mcseemagg@yahoo.com
//		  http://www.antigrain.com
//----------------------------------------------------------------------------

#ifndef AGG_PIXFMT_BRUSH8_INCLUDED
#define AGG_PIXFMT_BRUSH8_INCLUDED

#include <stdio.h>
#include <string.h>

#include <agg_basics.h>
#include <agg_color_gray.h>
#include <agg_rendering_buffer.h>

#include "blending.h"

namespace agg
{

	//======================================================pixfmt_brush8
	class pixfmt_brush8
	{
	public:
		typedef gray8 color_type;
        typedef rendering_buffer::row_data row_data;
        typedef rendering_buffer::span_data span_data;

		//--------------------------------------------------------------------
		pixfmt_brush8(rendering_buffer& rb)
			: m_rbuf(&rb),
			  m_cover_scale(255),
			  m_solid(false)
		{
		}

		//--------------------------------------------------------------------
		unsigned width()  const { return m_rbuf->width();  }
		unsigned height() const { return m_rbuf->height(); }

		//--------------------------------------------------------------------
		color_type pixel(int x, int y)
		{
			return color_type(m_rbuf->row(y)[x]);
		}

		//--------------------------------------------------------------------
		void copy_pixel(int x, int y, const color_type& c)
		{
printf("copy_pixel()\n");
			m_rbuf->row(y)[x] = (int8u)c.v;
		}

		//--------------------------------------------------------------------
		void blend_pixel(int x, int y, const color_type& c, int8u cover)
		{
printf("blend_pixel()\n");
			int8u* p = m_rbuf->row(y) + x;
			int v = *p;
			int alpha = int(cover) * c.a;
			*p = (int8u)((((c.v - v) * alpha) + (v << 16)) >> 16);
		}


		//--------------------------------------------------------------------
		void copy_hline(int x, int y, unsigned len, const color_type& c)
		{
printf("copy_hline()\n");
			int8u* p = m_rbuf->row(y) + x;
			do
			{
				*p = (int8u)c.v; 
				p++;
			}
			while(--len);
		}


		//--------------------------------------------------------------------
		void copy_vline(int x, int y, unsigned len, const color_type& c)
		{
printf("copy_vline()\n");
			int8u* p = m_rbuf->row(y) + x;
			do
			{
				*p = (int8u)c.v; 
				p += m_rbuf->stride();
			}
			while(--len);
		}


		//--------------------------------------------------------------------
		void blend_hline(int x, int y, unsigned len, 
						 const color_type& c, int8u cover)
		{
printf("blend_hline()\n");
			int8u* p = m_rbuf->row(y) + x;
			int alpha = int(cover) * c.a;
			if(alpha == 255*255)
			{
				do
				{
					*p = (int8u)c.v; 
					p ++;
				}
				while(--len);
			}
			else
			{
				do
				{
					int v = *p;
					*p = (int8u)((((c.v - v) * alpha) + (v << 16)) >> 16);
					p ++;
				}
				while(--len);
			}
		}


		//--------------------------------------------------------------------
		void blend_vline(int x, int y, unsigned len, 
						 const color_type& c, int8u cover)
		{
printf("blend_vline()\n");
			int8u* p = m_rbuf->row(y) + x;
			int alpha = int(cover) * c.a;
			if(alpha == 255*255)
			{
				do
				{
					*p = (int8u)c.v; 
					p += m_rbuf->stride();
				}
				while(--len);
			}
			else
			{
				do
				{
					int v = *p;
					*p = (int8u)((((c.v - v) * alpha) + (v << 16)) >> 16);
					p += m_rbuf->stride();
				}
				while(--len);
			}
		}


		//--------------------------------------------------------------------
		void copy_from(const rendering_buffer& from, 
					   int xdst, int ydst,
					   int xsrc, int ysrc,
					   unsigned len)
		{
printf("copy_from()\n");
			memmove(m_rbuf->row(ydst) + xdst, 
					(const void*)(from.row(ysrc) + xsrc), len);
		}


		//--------------------------------------------------------------------
		void blend_solid_hspan(int x, int y, unsigned len, 
							   const color_type& c, const int8u* covers)
		{
//printf("blend_solid_hspan()\n");
			int8u* p = m_rbuf->row(y) + x;
			int t;
			if (m_solid) {
				do 
				{
//					int alpha = (int(*covers++) * c.v) / 255;
					int alpha = INT_MULT(*covers++, c.v, t);

					if (alpha > m_cover_scale / 2)
						*p = max_c(*p, m_cover_scale);

					p++;
				} while(--len);
			} else {
				do {
	/*				int alpha = int(*covers++) * c.a;
	
					if(alpha)
					{
						if(alpha == 255*255)
						{
							*p = (int8u)c.v; 
						}
						else
						{
							int v = *p;
							*p = (int8u)((((c.v - v) * alpha) + (v << 16)) >> 16);
						}
					}*/
//					int alpha = (int(*covers++) * c.v) / 255;
					int alpha = INT_MULT(*covers++, c.v, t);

					*p = max_c(*p, alpha);
	//				covers++;
					p++;
				} while(--len);
			}
		}


		//--------------------------------------------------------------------
		void blend_solid_vspan(int x, int y, unsigned len, 
							   const color_type& c, const int8u* covers)
		{
printf("blend_solid_vspan()\n");
			int8u* p = m_rbuf->row(y) + x;
			do 
			{
				int alpha = int(*covers++) * c.a;

				if(alpha)
				{
					if(alpha == 255*255)
					{
						*p = (int8u)c.v; 
					}
					else
					{
						int v = *p;
						*p = (int8u)((((c.v - v) * alpha) + (v << 16)) >> 16);
					}
				}
				p += m_rbuf->stride();
			}
			while(--len);
		}


		//--------------------------------------------------------------------
		void blend_color_hspan(int x, int y, unsigned len, 
							   const color_type* colors,
							   const int8u* covers, int8u cover)
		{
			int8u* p = m_rbuf->row(y) + x;
			if(covers) {
//printf("blend_color_hspan()\n");
				int t;
				if (m_solid) {
					do {
//						int alpha = (int(*covers++) * colors->v * m_cover_scale) / (255 * 255);
						int alpha = INT_MULT(INT_MULT(*covers++, colors->v, t), m_cover_scale, t);

						if (alpha > m_cover_scale / 2)
							*p = max_c(*p, m_cover_scale);
	
						++p;
						++colors;
					} while(--len);
				} else {
					do {
//						int alpha = (int(*covers++) * colors->v * m_cover_scale) / (255 * 255);
						int alpha = INT_MULT(INT_MULT(*covers++, colors->v, t), m_cover_scale, t);
	
						if (*p < m_cover_scale)
							*p += (alpha * (m_cover_scale - *p)) / m_cover_scale;
	
						++p;
						++colors;
					} while(--len);
				}
			} else {
printf("blend_color_hspan() - no covers\n");
				do 
				{
					if(colors->a)
					{
						if(colors->a == 255)
						{
							*p = (int8u)colors->v; 
						}
						else
						{
							int v = *p;
							*p = (int8u)((((colors->v - v) * colors->a) + (v << 8)) >> 8);
						}
					}
					++p;
					++colors;
				}
				while(--len);
			}
/*			int8u* p = m_rbuf->row(y) + x;
            do 
            {
                int alpha = colors->a * (covers ? int(*covers++) : int(cover));

                if(alpha)
                {
                    if(alpha == 255*255)
                    {
                        *p = (int8u)colors->v; 
                    }
                    else
                    {
                        int v = *p;
                        *p = (int8u)((((colors->v - v) * alpha) + (v << 16)) >> 16);
                    }
                }
                ++p;
                ++colors;
            }
            while(--len);*/
		}



		//--------------------------------------------------------------------
		void blend_color_vspan(int x, int y, unsigned len, 
							   const color_type* colors,
							   const int8u* covers, int8u cover)
		{
printf("blend_color_vspan()\n");
			int8u* p = m_rbuf->row(y) + x;
			if(covers)
			{
				do 
				{
					int alpha = int(*covers++) * colors->a;

					if(alpha)
					{
						if(alpha == 255*255)
						{
							*p = (int8u)colors->v; 
						}
						else
						{
							int v = *p;
							*p = (int8u)((((colors->v - v) * alpha) + (v << 16)) >> 16);
						}
					}
					p += m_rbuf->stride();
					++colors;
				}
				while(--len);
			}
			else
			{
				do 
				{
					if(colors->a)
					{
						if(colors->a == 255)
						{
							*p = (int8u)colors->v; 
						}
						else
						{
							int v = *p;
							*p = (int8u)((((colors->v - v) * colors->a) + (v << 8)) >> 8);
						}
					}
					p += m_rbuf->stride();
					++colors;
				}
				while(--len);
			}
/*            int8u* p = m_rbuf->row(y) + x * Step + Offset;
            do 
            {
                int alpha = colors->a * (covers ? int(*covers++) : int(cover));

                if(alpha)
                {
                    if(alpha == 255*255)
                    {
                        *p = (int8u)colors->v; 
                    }
                    else
                    {
                        int v = *p;
                        *p = (int8u)((((colors->v - v) * alpha) + (v << 16)) >> 16);
                    }
                }
                p += m_rbuf->stride();
                ++colors;
            }
            while(--len);*/
		}

		void cover_scale(int8u scale)
		{
			m_cover_scale = scale;
		}

		void solid(bool solid)
		{
			m_solid = solid;
		}

	private:
		rendering_buffer*	m_rbuf;
		int8u				m_cover_scale;
		bool				m_solid;
	};
}

#endif // AGG_PIXFMT_BRUSH8_INCLUDED

