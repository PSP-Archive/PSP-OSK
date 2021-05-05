/*-----------------------------------------------------------------------------
 * On-Screen Keyboard 2 for uClinux on PSP
 * Created by Jackson Mo, Jan 2, 2008
 *---------------------------------------------------------------------------*/
#include <unistd.h>


//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
typedef struct
{ 
  unsigned short  bfType; 
  unsigned long   bfSize; 
  unsigned short  bfReserved1; 
  unsigned short  bfReserved2; 
  unsigned long   bfOffBits; 
} BITMAPFILEHEADER, *PBITMAPFILEHEADER; 

typedef struct
{
  unsigned long   biSize; 
  int             biWidth; 
  int             biHeight; 
  unsigned short  biPlanes; 
  unsigned short  biBitCount; 
  unsigned long   biCompression; 
  unsigned long   biSizeImage; 
  int             biXPelsPerMeter; 
  int             biYPelsPerMeter; 
  unsigned long   biClrUsed; 
  unsigned long   biClrImportant; 
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct
{
  int width;
  int height;
  unsigned long * bitmap;
} OskImgData;


//-----------------------------------------------------------------------------
// Here is an example of XXXX.rc
//-----------------------------------------------------------------------------
/*
// This file is auto-generated. DO NOT edit.

#inlcude "oskimg.h"

static unsigned long s_bitmapXXXX[ %d * %d ] =
{
  0x12345678, 0x12345678, 0x12345678, 0x12345678, 
  0x12345678, 0x12345678, 0x12345678, 0x12345678, 
  0x12345678, 0x12345678, 0x12345678, 0x12345678, 
  0x12345678, 0x12345678, 0x12345678, 0x12345678, 
  ...
  0x12345678, 0x12345678, 0x12345678, 0x12345678, 
  // End of bitmap
};

OskImgData s_imgXXXX =
{
  .width = %d,
  .height = %d,
  .bitmap = s_bitmapXXXX,
};
*/


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
