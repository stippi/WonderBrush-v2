//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// scanline_u8 class
//
//----------------------------------------------------------------------------
#ifndef AGG_ALPHA_MASK_U8_H
#define AGG_ALPHA_MASK_U8_H

#include <string.h>

#include <agg_basics.h>
#include <agg_rendering_buffer.h>

#include "blending.h"

namespace agg
{
   //==========================================================amask_no_clip_u8_precise
    template<unsigned Step=1, unsigned Offset=0>
    class amask_no_clip_u8_precise
    {
    public:
        typedef int8u cover_type;
        typedef amask_no_clip_u8_precise<Step, Offset> self_type;
        enum 
        { 
            cover_none  = 0,
            cover_full  = 255
        };

        amask_no_clip_u8_precise() : m_rbuf(0) {}
        amask_no_clip_u8_precise(rendering_buffer& rbuf) : m_rbuf(&rbuf) {}

        void attach(rendering_buffer& rbuf) { m_rbuf = &rbuf; }

        //--------------------------------------------------------------------
        cover_type pixel(int x, int y) const
        {
            return (cover_type)m_mask_function.calculate(
                                   m_rbuf->row(y) + x * Step + Offset);
        }

        
        //--------------------------------------------------------------------
        cover_type combine_pixel(int x, int y, cover_type val) const
        {
        	int t;
            return (cover_type)INT_MULT(val, *(m_rbuf->row(y) + x * Step + Offset), t);
        }


        //--------------------------------------------------------------------
        void fill_hspan(int x, int y, cover_type* dst, int num_pix) const
        {
            const int8u* mask = m_rbuf->row(y) + x * Step + Offset;
            do
            {
                *dst++ = (cover_type)*mask;
                mask += Step;
            }
            while(--num_pix);
        }



        //--------------------------------------------------------------------
        void combine_hspan(int x, int y, cover_type* dst, int num_pix) const
        {
            const int8u* mask = m_rbuf->row(y) + x * Step + Offset;
            int t;
            do
            {
                *dst = (cover_type)INT_MULT((*dst), *mask, t);
                ++dst;
                mask += Step;
            }
            while(--num_pix);
        }


        //--------------------------------------------------------------------
        void fill_vspan(int x, int y, cover_type* dst, int num_pix) const
        {
            const int8u* mask = m_rbuf->row(y) + x * Step + Offset;
            do
            {
                *dst++ = (cover_type)*mask;
                mask += m_rbuf->stride();
            }
            while(--num_pix);
        }


        //--------------------------------------------------------------------
        void combine_vspan(int x, int y, cover_type* dst, int num_pix) const
        {
            const int8u* mask = m_rbuf->row(y) + x * Step + Offset;
            int t;
            do
            {
                *dst = (cover_type)INT_MULT(*dst, *mask, t);
                ++dst;
                mask += m_rbuf->stride();
            }
            while(--num_pix);
        }

    private:
        amask_no_clip_u8_precise(const self_type&);
        const self_type& operator = (const self_type&);

        rendering_buffer* m_rbuf;
    };

    typedef amask_no_clip_u8_precise<1, 0> amask_no_clip_gray8_precise;   //----amask_no_clip_gray8_precise
}



#endif
