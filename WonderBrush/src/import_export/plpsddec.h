/*
/--------------------------------------------------------------------
|
|      $Id: plpsddec.h,v 1.5 2004/06/19 16:49:07 uzadow Exp $
|
|      Copyright (c) 1996-2002 Ulrich von Zadow
|
\--------------------------------------------------------------------
*/

#ifndef INCL_PLPSDDEC
#define INCL_PLPSDDEC

#ifndef INCL_PLPICDEC
#include "plpicdec.h"
#endif

#include <vector>

class PLDataSource;
class PLPoint;

struct PLPSDHeader
{
  char     Signature[4];
  PLWORD   Version;
  PLBYTE   Reserved[6];
  PLWORD   Channels;
  int      Rows;
  int      Columns;
  PLWORD   BPP;
  PLWORD   Mode;
};

struct PLPSDLayerInfo;

//! Photoshop file decoder. Besides having the MakeBmpFromFile interface
//! that the other decoders have, the photoshop decoder loads the layers
//! of the image into separate bitmaps so they can be manipulated separately:
//! <pre>
//!   vector&lt;PLAnyBmp&gt; Bmp;
//!   PLAnyBmp BaseBmp);
//!   Decoder-&gt;OpenFile ("face.psd");
//!   int NumLayers = Decoder-&gt;GetNumLayers();
//!   for (int i=0; i&lt;NumLayers; i++)
//!   {
//!     GetNextLayer (Bmp[i]);
//!     LayerOffset = GetLayerOffset();
//!   }
//!   GetImage (&BaseBmp);
//!   Close();
//! </pre>
class PLPSDDecoder : public PLPicDecoder
{
public:
  //! Creates a decoder
  PLPSDDecoder
    ();

  //! Destroys a decoder
  virtual ~PLPSDDecoder
    ();

  //!
  virtual void Open (PLDataSource * pDataSrc);

  //! Fills the bitmap with the main image. This image is a flattened 
  //! version of the layers.
  virtual void GetImage (PLBmpBase & pBmp);

  //! Returns number of layers in the image.
  int GetNumLayers
    ();

  //! Fills the bitmap with the layer data.
  void GetNextLayer
    ( PLBmp& Bmp
    );

  //! Returns the origin of the layer data in the image.
  PLPoint GetLayerOffset
    ();

private:
  void readHeader
    ( PLDataSource * pDataSrc,
      PLPSDHeader * pPSDHeader
    );

  void readColorModeData 
    ( PLDataSource * pDataSrc
    );

  PLBmpInfo* createBmpInfo 
    ( int Mode,
      int Height,
      int Width,
      int Channels,
      const PLPoint& Resolution
    );

  void readImageResData 
    ( PLDataSource * pDataSrc,
      PLPoint& Resolution
    );

  void readLayerHeader 
    ( PLDataSource * pDataSrc
    );

  void skipLayerData
    ( PLDataSource * pDataSrc
    );

  void skipMaskData
    ( PLDataSource * pDataSrc
    );

  void readLayer 
    ( PLDataSource * pDataSrc, 
      PLBmp& Bmp,
      int Mode
    );

  void readImageData
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int Channels 
    );

  void traceCompressionMethod
    ( PLWORD CompressionMethod
    );

  void readUncompressedImage 
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int Channels 
    );

  void readRLEImage 
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int Channels 
    );

  void readRLEChannel 
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int ChannelNum,
      int * pRowLengths 
    );

  void cleanup
    ();

  PLPSDHeader m_PSDHeader;
  PLPoint m_Resolution;
  PLPixel32 m_pPal[256];
  PLBYTE * m_pMiscDataStart;
  int m_MiscDataSize;
  int m_NumLayers;
  int m_LayersRead;
  std::vector<PLPSDLayerInfo *> m_pLayerInfo;
};

#endif

/*
/--------------------------------------------------------------------
|
|      $Log: plpsddec.h,v $
|      Revision 1.5  2004/06/19 16:49:07  uzadow
|      Changed GetImage so it works with PLBmpBase
|
|      Revision 1.4  2002/11/18 14:45:10  uzadow
|      Added PNG compression support, fixed docs.
|
|      Revision 1.3  2002/08/04 20:08:01  uzadow
|      Added PLBmpInfo class, ability to extract metainformation from images without loading the whole image and proper greyscale support.
|
|      Revision 1.2  2002/03/31 13:36:42  uzadow
|      Updated copyright.
|
|      Revision 1.1  2001/10/21 17:12:40  uzadow
|      Added PSD decoder beta, removed BPPWanted from all decoders, added PLFilterPixel.
|
|
\--------------------------------------------------------------------
*/
