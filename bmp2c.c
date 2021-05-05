/*-----------------------------------------------------------------------------
 * BMP to C utility for uClinux on PSP
 * Created by Jackson Mo, Jan 2, 2008
 *---------------------------------------------------------------------------*/
#include <unistd.h>
#include <stdio.h>
#include "oskimg.h"


/*-----------------------------------------------------------------------------
 * Constants
 *---------------------------------------------------------------------------*/
#define BOOL                int
#define TRUE                1
#define FALSE               0
#define MAX_RC_FILENAME     255


/*-----------------------------------------------------------------------------
 * Prototypes
 *---------------------------------------------------------------------------*/
static char * getTag(char * tagName_, const char * bmpName_);
static BOOL readBitmap(FILE * file_, OskImgData * imgData_);
static BOOL writeRc(FILE * file_, const char * tagName_, const OskImgData * imgData_);
static inline unsigned long convertPixel(unsigned long c_);


/*-----------------------------------------------------------------------------
 * Implementations
 *---------------------------------------------------------------------------*/
int main(int argc_, char * argv_[])
{
  const char * bmpName;
  char tagName[ MAX_RC_FILENAME ];
  char rcName[ MAX_RC_FILENAME ];
  FILE * fBmp;
  FILE * fRc;
  OskImgData imgData;

  printf( "<<< BMP2RC version 0.1 by Jackson Mo >>>\n" );

  if ( argc_ < 2 )
  {
    printf( "Usage: bmp2rc <bitmap_file>\n" );
    return 0;
  }


  bmpName = (const char *)argv_[ 1 ];
  fBmp = fopen( bmpName, "rb" );
  if ( fBmp == NULL )
  {
    printf( "Can not open bitmap %s\n", bmpName );
    return -1;
  }

  strcpy( rcName, getTag( tagName, bmpName ) );
  strcat( rcName, ".c" );
  printf( "Converting %s to %s...\n", bmpName, rcName );

  if ( !readBitmap( fBmp, &imgData ) )
  {
    return -1;
  }
  fclose( fBmp );

  /* Write data to the RC file */
  fRc = fopen( rcName, "w" );
  if ( fRc == NULL )
  {
    printf( "Can not open file %s for writing\n", rcName );
    return -1;
  }

  (void)writeRc( fRc, tagName, &imgData );
  fclose( fRc );
  
  if ( imgData.bitmap != NULL )
  {
    free( imgData.bitmap );
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static char * getTag(char * tagName_, const char * bmpName_)
{
  int i;
  for ( i = 0;
        i < MAX_RC_FILENAME && bmpName_[ i ] != 0 && bmpName_[ i ] != '.';
        i++ )
  {
    tagName_[ i ] = bmpName_[ i ];
  }
  tagName_[ i ] = 0;

  return tagName_;
}
/*---------------------------------------------------------------------------*/
static BOOL readBitmap(FILE * file_, OskImgData * imgData_)
{
  BITMAPFILEHEADER fileHeader;
  BITMAPINFOHEADER infoHeader;
  int i, j;
  int skipLen;
  unsigned long * line;
  unsigned long pixel;

  if ( fread( &fileHeader, 14 /*sizeof( fileHeader )*/, 1, file_ ) != 1 )
  {
    printf( "Failed to read bitmap file header\n" );
    return FALSE;
  }

  if ( fileHeader.bfType != 0x4d42 )
  {
    printf( "Specified file is not a bitmap file\n" );
    return FALSE;
  }
  
  if ( fread( &infoHeader, sizeof( infoHeader ), 1, file_ ) != 1 )
  {
    printf( "Failed to read bitmap info header\n" );
    return FALSE;
  }

/*
  printf( "  biSize = %d\n"
          "  biWidth = %d\n" 
          "  biHeight = %d\n" 
          "  biPlanes = %d\n" 
          "  biBitCount = %d\n" 
          "  biCompression = %d\n" 
          "  biSizeImage = %d\n" 
          "  biXPelsPerMeter = %d\n" 
          "  biYPelsPerMeter = %d\n" 
          "  biClrUsed = %d\n" 
          "  biClrImportant = %d\n",
          infoHeader.biSize,
          infoHeader.biWidth,
          infoHeader.biHeight,
          infoHeader.biPlanes,
          infoHeader.biBitCount,
          infoHeader.biCompression,
          infoHeader.biSizeImage,
          infoHeader.biXPelsPerMeter,
          infoHeader.biYPelsPerMeter,
          infoHeader.biClrUsed,
          infoHeader.biClrImportant );
*/

  if ( infoHeader.biBitCount != 24 )
  {
    printf( "Specified bitmap is not a 24-bit but a %d-bit format bitmap\n",
            infoHeader.biBitCount );
    return FALSE;
  }

  imgData_->width = infoHeader.biWidth;
  imgData_->height = infoHeader.biHeight;

  imgData_->bitmap = (unsigned long *)malloc( infoHeader.biWidth *
                                              infoHeader.biHeight *
                                              sizeof( unsigned long ) );
  if ( imgData_->bitmap == NULL )
  {
    printf( "Failed to allocate buffer for storing bitmap bits\n" );
    return FALSE;
  }

  skipLen = 4 - ( ( infoHeader.biWidth * 3 ) % 4 );
  line = imgData_->bitmap + ( infoHeader.biHeight - 1 ) * infoHeader.biWidth;

  for ( i = 0; i < infoHeader.biHeight; i++ )
  {
    for ( j = 0; j < infoHeader.biWidth; j++ )
    {
      if ( fread( &pixel, 3, 1, file_ ) != 1 )
      {
        printf( "Failed to read bitmap bits (%d, %d)\n", j, i );
        free( imgData_->bitmap );
        imgData_->bitmap = NULL;
        return FALSE;
      }

      line[ j ] = convertPixel( pixel );
    }

    line -= infoHeader.biWidth;

    if ( skipLen < 4 )
    {
      fseek( file_, skipLen, SEEK_CUR );
    }
  }

  return TRUE;
}
/*---------------------------------------------------------------------------*/
static BOOL writeRc
(
  FILE * file_,
  const char * tagName_,
  const OskImgData * imgData_
)
{
  int size, i, count;

  size = imgData_->width * imgData_->height;

  fprintf( file_, 
           "// This file is auto-generated. DO NOT edit.\n"
           "#include \"oskimg.h\"\n"
           "\n"
           "static unsigned long s_bitmap%s[ %d ] =\n"
           "{\n  ",
           tagName_, size );

  for ( i = 0, count = 0; i < size; i++ )
  {
    fprintf( file_, "0x%08x, ", imgData_->bitmap[ i ] );

    if ( ++count >= 4)
    {
      count = 0;
      fprintf( file_, "\n  " );
    }
  }

  fprintf( file_,
           "\n  "
           "// End of bitmap\n"
           "};\n"
           "\n"
           "OskImgData s_img%s =\n"
           "{\n"
           "  .width = %d,\n"
           "  .height = %d,\n"
           "  .bitmap = s_bitmap%s,\n"
           "};\n"
           "\n",
           tagName_, imgData_->width, imgData_->height, tagName_ );
           
  return TRUE;
}
/*---------------------------------------------------------------------------*/
static inline unsigned long convertPixel(unsigned long c_)
{
  return ( ( c_ & 0x0000ff ) << 16 ) |
           ( c_ & 0x00ff00 ) |
         ( ( c_ & 0xff0000 ) >> 16 );
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
