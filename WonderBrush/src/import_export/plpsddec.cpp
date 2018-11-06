/*
/--------------------------------------------------------------------
|
|      $Id: plpsddec.cpp,v 1.9 2004/09/11 12:41:35 uzadow Exp $
|
|      Copyright (c) 1996-1998 Ulrich von Zadow
|
\--------------------------------------------------------------------
*/

#include "plpsddec.h"
#include "plexcept.h"
#include "plpixel32.h"
#include "plpixel8.h"
#include "plfilesrc.h"

#include <stdio.h>

using namespace std;

enum PLPSDMode
{
  Bitmap,
  Greyscale,
  Indexed,
  RGB,
  CYMK,
  Multichannel,
  Duotone,
  Lab
};

struct PLPSDResolutionInfo
{
  int    hRes;       // Actually fixed-point
  PLWORD hResUnit;
  PLWORD widthUnit;
  int    vRes;       // Actually fixed-point
  PLWORD vResUnit;
  PLWORD heightUnit;
};

struct PLPSDChannelInfo
{
  PLWORD ChannelID;
  int ChannelDataLen;
};

struct PLPSDLayerInfo
{
  int Top;
  int Left;
  int Bottom;
  int Right;
  PLWORD NumChannels;
  vector<PLPSDChannelInfo> ChannelInfo;
  char BlendModeSig[4];
  char BlendModeKey[4];
  PLBYTE Opacity;
  PLBYTE Clipping;
  PLBYTE Flags;
  int ExtraDataSize;
};

PLPSDDecoder::PLPSDDecoder
    ()
    : PLPicDecoder()
    // Creates a decoder
{
}


PLPSDDecoder::~PLPSDDecoder
    ()
{
}

int PLPSDDecoder::GetNumLayers
    ()
{
  PLASSERT (m_pDataSrc);
  return m_NumLayers;
}

void PLPSDDecoder::GetNextLayer
    ( PLBmp& Bmp
    )
{
  PLASSERT (m_pDataSrc);
  try
  {
    readLayer (m_pDataSrc, Bmp, m_PSDHeader.Mode);
  }
  catch (PLTextException)
  {
    Close();
    throw;
  }
}

PLPoint PLPSDDecoder::GetLayerOffset
    ()
{
  PLPSDLayerInfo * pLayerInfo = m_pLayerInfo[m_LayersRead-1];
  return PLPoint (pLayerInfo->Left, pLayerInfo->Top);
}

void PLPSDDecoder::GetImage
    ( PLBmpBase& Bmp
    )
{
  // Was Open called?
  PLASSERT (m_pDataSrc);

  if (GetBitsPerPixel() == 8)
    Bmp.SetPalette (m_pPal);

  try
  {
    skipLayerData (m_pDataSrc);
    readImageData (m_pDataSrc, &Bmp, m_PSDHeader.Mode,
                   m_PSDHeader.Rows, m_PSDHeader.Columns, 
                   m_PSDHeader.Channels);
  }
  catch (PLTextException)
  {
    cleanup ();
    Close();
    throw;
  }
  cleanup ();
}

void PLPSDDecoder::Open (PLDataSource * pDataSrc) 
{
  readHeader (pDataSrc, &m_PSDHeader);
  readColorModeData (pDataSrc);

  PLPoint Resolution;
  readImageResData (pDataSrc, Resolution);
  PLBmpInfo * pBmpInfo = createBmpInfo (m_PSDHeader.Mode, m_PSDHeader.Rows, 
          m_PSDHeader.Columns, m_PSDHeader.Channels, Resolution);
  SetBmpInfo (*pBmpInfo);
  delete pBmpInfo;
  readLayerHeader (pDataSrc);
}

void PLPSDDecoder::readHeader
    ( PLDataSource * pDataSrc,
      PLPSDHeader * pPSDHeader
    )
{
  memcpy (pPSDHeader->Signature, pDataSrc->Read4Bytes (), 4);
  pPSDHeader->Version = ReadMWord (pDataSrc);
  memcpy (pPSDHeader->Reserved, pDataSrc->ReadNBytes (6), 6);
  pPSDHeader->Channels = ReadMWord (pDataSrc);
  pPSDHeader->Rows = ReadMLong (pDataSrc);
  pPSDHeader->Columns = ReadMLong (pDataSrc);
  pPSDHeader->BPP = ReadMWord (pDataSrc);
  pPSDHeader->Mode = ReadMWord (pDataSrc);

  if (strncmp (pPSDHeader->Signature, "8BPS", 4))
  {
      raiseError (PL_ERRFORMAT_UNKNOWN,
                  "PSD decoder: This isn't a photoshop file.");
  }
}

void PLPSDDecoder::readColorModeData 
    ( PLDataSource * pDataSrc
    )
{
  int PalLen = ReadMLong (pDataSrc);
  if (PalLen != 0)
  {
    PLASSERT (PalLen == 768);
    PLBYTE * pSrcPal = pDataSrc->ReadNBytes (768);
    for (int i=0; i<256; i++)
    {
      m_pPal[i].Set (pSrcPal[0], pSrcPal[256], pSrcPal[512], 0xff);
      pSrcPal ++;
    }
  }
}

PLBmpInfo* PLPSDDecoder::createBmpInfo 
    ( int Mode,
      int Height,
      int Width,
      int Channels,
      const PLPoint& Resolution
    )
{
  PLPixelFormat pf;
  switch (PLPSDMode(Mode))
  {
    case Greyscale:
      {        
        for (int i=0; i<256; i++)
          m_pPal[i].Set (i, i, i, 0xFF);        
        pf = PLPixelFormat::L8;
      }
      break;
    case Indexed:      
      pf = PLPixelFormat::I8;
      break;
    case RGB:
      bool bAlpha;  
      bAlpha = (Channels > 3);
      if (bAlpha) {
        pf = PLPixelFormat::A8R8G8B8;
      } else {
        pf = PLPixelFormat::X8R8G8B8;
      }
      break;
    default:
      raiseError (PL_ERRFORMAT_NOT_SUPPORTED, 
                  "This PSD color mode is not supported.");
  }
  return new PLBmpInfo (PLPoint (Width, Height), Resolution, pf);

}

void PLPSDDecoder::readImageResData 
    ( PLDataSource * pDataSrc,
      PLPoint& Resolution
    )
{
  int ResLen = ReadMLong (pDataSrc);
  int BytesRead = 0;
  while (BytesRead < ResLen)
  {
    char ResSignature[4];
    memcpy (ResSignature, pDataSrc->Read4Bytes (), 4);
    if (strncmp (ResSignature, "8BIM", 4))
      raiseError (PL_ERRFORMAT_UNKNOWN,
                  "PSD decoder: Wrong signature in resource data block.");
    PLWORD ResID = ReadMWord (pDataSrc);
    PLBYTE NameLen = ReadByte (pDataSrc);
    char * pszName = new char[NameLen+1];
    memcpy (pszName, pDataSrc->ReadNBytes(NameLen), NameLen);
    pszName[NameLen] = 0;
    Trace (2, "Reading resource:");
    Trace (2, pszName);
    Trace (2, "\n");
    delete[] pszName;
    if (!(NameLen&1))
    {
      ReadByte(pDataSrc);
      BytesRead++;
    }
    int Size = ReadMLong (pDataSrc);
    BytesRead += 4+2+1+NameLen+4;
    switch (ResID)
    {
      case 1005:
        {
          PLPSDResolutionInfo ResInfo;
          ResInfo.hRes = ReadMLong (pDataSrc);
          ResInfo.hResUnit = ReadMWord (pDataSrc);
          ResInfo.widthUnit = ReadMWord (pDataSrc);
          ResInfo.vRes = ReadMLong (pDataSrc);
          ResInfo.vResUnit = ReadMWord (pDataSrc);
          ResInfo.heightUnit = ReadMWord (pDataSrc);
          Resolution = PLPoint (ResInfo.hRes/65536, ResInfo.vRes/65536);
        }
        break;
      default:
        pDataSrc->ReadNBytes (Size);
    }
    if (Size&1)
    {
      ReadByte(pDataSrc);
      BytesRead++;
    }
    BytesRead += Size;
  }
}

void PLPSDDecoder::readLayerHeader 
    ( PLDataSource * pDataSrc
    )
{
  m_MiscDataSize = ReadMLong (pDataSrc);
  if(m_MiscDataSize > 0)
  {
    m_pMiscDataStart = pDataSrc->GetBufferPtr(0);
    int LayerDataSize = ReadMLong (pDataSrc);
    if (LayerDataSize > 0)
    {
      m_NumLayers = ReadMWord (pDataSrc);
      if (m_NumLayers > 32767)
        m_NumLayers -= 65536;
      if (m_NumLayers<0)
        m_NumLayers = -m_NumLayers;
      m_LayersRead = 0;
      char sz[256];
      sprintf (sz, "Number of layers: %i\n", m_NumLayers);
      Trace (2, sz);
      
      for (int i=0; i<m_NumLayers; i++)
      {
        PLPSDLayerInfo* pLayerInfo = new PLPSDLayerInfo;
        m_pLayerInfo.push_back (pLayerInfo);    
        pLayerInfo->Top = ReadMLong (pDataSrc);
        pLayerInfo->Left = ReadMLong (pDataSrc);
        pLayerInfo->Bottom = ReadMLong (pDataSrc);
        pLayerInfo->Right = ReadMLong (pDataSrc);
        pLayerInfo->NumChannels = ReadMWord (pDataSrc);
        for (int j=0; j<pLayerInfo->NumChannels; j++)
        {
          PLPSDChannelInfo ChannelInfo;
          ChannelInfo.ChannelID = ReadMWord (pDataSrc);
          ChannelInfo.ChannelDataLen = ReadMLong (pDataSrc);
          pLayerInfo->ChannelInfo.push_back(ChannelInfo);
        }
        memcpy (pLayerInfo->BlendModeSig, pDataSrc->Read4Bytes (), 4);
        memcpy (pLayerInfo->BlendModeKey, pDataSrc->Read4Bytes (), 4);
        pLayerInfo->Opacity = ReadByte (pDataSrc);
        pLayerInfo->Clipping = ReadByte (pDataSrc);
        pLayerInfo->Flags = ReadByte (pDataSrc);
        ReadByte (pDataSrc);
        pLayerInfo->ExtraDataSize = ReadMLong (pDataSrc);
        pDataSrc->Skip (pLayerInfo->ExtraDataSize);
      }
    } else {
        m_NumLayers = 0;
    }
  } else {
    m_NumLayers=0;
  }
}

void PLPSDDecoder::skipLayerData
    ( PLDataSource * pDataSrc
    )
{
  if (m_MiscDataSize > 0)
  {
    int BytesLeft = m_MiscDataSize + (m_pMiscDataStart - pDataSrc->GetBufferPtr(0));
    pDataSrc->Skip (BytesLeft);
  }
  
}

void PLPSDDecoder::skipMaskData
    ( PLDataSource * pDataSrc
    )
{
  int MaskDataLen = ReadMLong (pDataSrc);
  pDataSrc->ReadNBytes (MaskDataLen);
}

void PLPSDDecoder::readLayer 
    ( PLDataSource * pDataSrc, 
      PLBmp& Bmp,
      int Mode
    )
{
  m_LayersRead++;
  PLASSERT (m_LayersRead <= m_NumLayers);
  int CurLayer = m_LayersRead-1;
  PLPSDLayerInfo * pLayerInfo = m_pLayerInfo[CurLayer];
  
  char sz[256];
  sprintf (sz, "Reading Layer %i\n", CurLayer);
  Trace (2, sz);

  int Width = pLayerInfo->Right-pLayerInfo->Left;
  int Height = pLayerInfo->Bottom-pLayerInfo->Top;
  PLBmpInfo *pBmpInfo = createBmpInfo (Mode, Height, Width, pLayerInfo->NumChannels, m_Resolution);
  Bmp.Create (*pBmpInfo);
  delete pBmpInfo;
  int * pRowLengths = new int [Height];

  for (int l=0; l<pLayerInfo->NumChannels; l++)
  {
    char sz[256];
    sprintf (sz, "Reading Channel %i\n", l);
    Trace (2, sz);
    
    PLWORD CompressionMethod = ReadMWord (pDataSrc);
    traceCompressionMethod (CompressionMethod);
    if (CompressionMethod == 1)
    {
      for (int y=0; y<Height; y++)
        pRowLengths[y] = ReadMWord (pDataSrc);
      readRLEChannel (pDataSrc, &Bmp, Mode, Height, Width, l, pRowLengths);
    }
    else
    raiseError (PL_ERRFORMAT_UNKNOWN,
                "PSD decoder: Compression type not supported.");
  }

  delete[] pRowLengths;
}

void PLPSDDecoder::readImageData
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int Channels 
    )
{
  PLWORD CompressionMethod = ReadMWord (pDataSrc);
  traceCompressionMethod (CompressionMethod);
  switch (CompressionMethod) 
  {
    case 0: 
      readUncompressedImage (pDataSrc, pBmp, Mode, Height,
                             Width, Channels);
	  break;
    case 1: 
      readRLEImage (pDataSrc, pBmp, Mode, Height, 
                    Width, Channels);
      break;
    default:
      raiseError (PL_ERRFORMAT_UNKNOWN,
                  "PSD decoder: Compression type not supported.");
  }
}

void PLPSDDecoder::traceCompressionMethod
    ( PLWORD CompressionMethod
    )
{
  switch (CompressionMethod)
  {
    case 0: 
      Trace (2, "No compression\n");
      break;
    case 1:
      Trace (2, "RLE compression\n");
      break;
    case 2:
      Trace (2, "ZIP compression, no prediction\n");
      break;
    case 3:
      Trace (2, "ZIP compression, prediction\n");
      break;
    default:
      raiseError (PL_ERRFORMAT_UNKNOWN,
                  "PSD decoder: Compression unknown.");
  }
}

void PLPSDDecoder::readUncompressedImage 
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int Channels 
    )
{
  for (int l=0; l<Channels; l++)
  {
    for (int y=0; y<Height; y++)
    {
      PLBYTE * pLine = pBmp->GetLineArray()[y];
      if (Channels == 1)
        memcpy (pLine, pDataSrc->ReadNBytes (Width), Width);
      else
      {
        // Convert photoshop channel order to paintlib channel order.
        int DestChannel;
        switch (l)
        {
          case 0:
            DestChannel = PL_RGBA_RED;
            break;
          case 1:
            DestChannel = PL_RGBA_GREEN;
            break;
          case 2:
            DestChannel = PL_RGBA_BLUE;
            break;
          default:
            DestChannel = 3;
        }

        for (int x=0; x<Width; x++)
          pLine[4*x+DestChannel]=*(pDataSrc->Read1Byte());
      }
    }
  }
}


void PLPSDDecoder::readRLEImage 
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int Channels 
    )
{
  // Read row lengths into temp array;
  int * pRowLengths = new int [Height*Channels];
  int l;
  for (l=0; l<Channels; l++)
    for (int y=0; y<Height; y++)
      pRowLengths[l*Height+y] = ReadMWord (pDataSrc);

  for (l=0; l<Channels; l++)
  {
    readRLEChannel (pDataSrc, pBmp, Mode, Height, Width, l, pRowLengths +l*Height);
  }

  delete[] pRowLengths;
}


void PLPSDDecoder::readRLEChannel 
    ( PLDataSource * pDataSrc,
      PLBmpBase * pBmp,
      int Mode,
      int Height,
      int Width,
      int ChannelNum,
      int * pRowLengths 
    )
{
  PLBYTE * pLineBuf = new PLBYTE [Width];
  for (int y=0; y<Height; y++)
  {
    PLBYTE * pRawLine = unpackPictRow (pLineBuf, pDataSrc, Width, 
                                       Width, pRowLengths[y]);
    int x;
    switch (Mode)
    {
      case Indexed:
      case Greyscale:
        if (ChannelNum == 0)
        {
          PLPixel8 * pBmpLine = (PLPixel8*)pBmp->GetLineArray()[y];
          for (x=0; x<Width; x++)
            pBmpLine[x].Set (pRawLine[x]);
        }
        break;
      case RGB:
        {
          PLPixel32 * pBmpLine = pBmp->GetLineArray32()[y];
          switch (ChannelNum) 
          {
            case 0:
              for (x=0; x<Width; x++)
                pBmpLine[x].SetR (pRawLine[x]);
              break;
            case 1:
              for (x=0; x<Width; x++)
                pBmpLine[x].SetG (pRawLine[x]);
              break;
            case 2:
              for (x=0; x<Width; x++)
                pBmpLine[x].SetB (pRawLine[x]);
              break;
            case 3:
              for (x=0; x<Width; x++)
                pBmpLine[x].SetA (pRawLine[x]);
              break;
          }
        }
        break;
      default:
        // We shouldn't have arrived here.
        PLASSERT (false);
    }
  }
  delete[] pLineBuf;
}

void PLPSDDecoder::cleanup
    ()
{
  for (unsigned int i=0; i<m_pLayerInfo.size(); i++)
    delete m_pLayerInfo[i];
  m_pLayerInfo.clear();
}

/*
/--------------------------------------------------------------------
|
|      $Log: plpsddec.cpp,v $
|      Revision 1.9  2004/09/11 12:41:35  uzadow
|      removed plstdpch.h
|
|      Revision 1.8  2004/09/09 16:52:49  artcom
|      refactored PixelFormat
|
|      Revision 1.7  2004/06/19 16:49:07  uzadow
|      Changed GetImage so it works with PLBmpBase
|
|      Revision 1.6  2003/04/06 12:20:16  uzadow
|      Fixed bug in 8 bpp PSD decoder.
|
|      Revision 1.5  2002/12/02 22:07:15  uzadow
|      Fixed bug decoding uncompressed images.
|
|      Revision 1.4  2002/08/07 20:32:31  uzadow
|      Updated Makefile.in, fixed psd decoder layer stuff.
|
|      Revision 1.3  2002/08/04 20:08:01  uzadow
|      Added PLBmpInfo class, ability to extract metainformation from images without loading the whole image and proper greyscale support.
|
|      Revision 1.2  2001/10/21 17:54:40  uzadow
|      Linux compatibility
|
|      Revision 1.1  2001/10/21 17:12:40  uzadow
|      Added PSD decoder beta, removed BPPWanted from all decoders, added PLFilterPixel.
|
|
\--------------------------------------------------------------------
*/
