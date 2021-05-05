/*-----------------------------------------------------------------------------
 * On-Screen Keyboard 2 for uClinux on PSP
 * Created by Jackson Mo, Jan 2, 2008
 *---------------------------------------------------------------------------*/
#include "osk.h"
#include "oskimg.h"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


//-----------------------------------------------------------------------------
// Classes
//-----------------------------------------------------------------------------
class OskImage_Psp;
class OskCanvas_Psp;
class OskInput_Psp;
class OskConsole_Psp;


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const char c_fbDevName[]             = "/dev/fb0";
static const char c_joypadDevName[]         = "/dev/joypad";
static const char c_vcsDevName[]            = "/dev/vcs";
static const int PSP_VCS_IOCTL_PUTCHAR      = 101;
static const int PSP_VCS_IOCTL_CHANGE_CON   = 107;
static const int PSP_VCS_IOCTL_UPDATE_SCR   = 108;


//-----------------------------------------------------------------------------
// Static Data
//-----------------------------------------------------------------------------
extern "C" OskImgData s_imgEng;
extern "C" OskImgData s_imgEngActive;
extern "C" OskImgData s_imgCap;
extern "C" OskImgData s_imgCapActive;
extern "C" OskImgData s_imgNum;
extern "C" OskImgData s_imgNumActive;
extern "C" OskImgData s_imgMouse;

static const OskImgData * const s_imgDataList[ OskImage::IMGID_Count ] =
{
  &s_imgEng,
  &s_imgEngActive,
  &s_imgCap,
  &s_imgCapActive,
  &s_imgNum,
  &s_imgNumActive,
  &s_imgMouse,
};


//-----------------------------------------------------------------------------
// Class: OskImage_Psp
//-----------------------------------------------------------------------------
class OskImage_Psp : public OskImage
{
public:
  OskImage_Psp(ImageId imgId_);
  //virtual ~OskImage_Psp();

  virtual const void * GetData() const
  {
    return m_imgData.bitmap;
  }

protected:
  const OskImgData & m_imgData;

private:
  // Not implemented
  OskImage_Psp();
  OskImage_Psp(const OskImage_Psp &);
  OskImage_Psp & operator = (const OskImage_Psp &);
};


//-----------------------------------------------------------------------------
// Class: OskCanvas_Psp
//-----------------------------------------------------------------------------
class OskCanvas_Psp : public OskCanvas
{
public:
  OskCanvas_Psp();
  virtual ~OskCanvas_Psp();

  virtual bool Initialize(void * param_);

  virtual bool Clear
  (
    int x_,
    int y_,
    int width_,
    int height_
  );

  virtual bool DrawImage
  (
    int destX_,
    int destY_,
    const OskImage & img_,
    int sourX_,
    int sourY_,
    int width_,
    int height_
  );

  virtual bool GetBits(void * buf_, int & size_);

protected:
  static unsigned long convertPixel(unsigned long color_);
  bool flush();
  
  unsigned long * m_vramBase;
  unsigned long m_vramSize;
  int m_virtualWidth;
  int m_virtualHeight;
  int m_fbFd;

private:
  // Not implemented
  OskCanvas_Psp(const OskCanvas_Psp &);
  OskCanvas_Psp & operator = (const OskCanvas_Psp &);
};


//-----------------------------------------------------------------------------
// Class: OskInput_Psp
//-----------------------------------------------------------------------------
class OskInput_Psp : public OskInput
{
public:
  OskInput_Psp();
  virtual ~OskInput_Psp();

  virtual bool Initialize(void * param_);
  virtual unsigned long ReadKeys();

protected:
  int m_joypadFd;

private:
  // Not implemented
  OskInput_Psp(const OskInput_Psp &);
  OskInput_Psp & operator = (const OskInput_Psp &);
};


//-----------------------------------------------------------------------------
// Class: OskConsole_Psp
//-----------------------------------------------------------------------------
class OskConsole_Psp : public OskConsole
{
public:
  OskConsole_Psp();
  virtual ~OskConsole_Psp();

  virtual bool Initialize(void * param_);
  virtual bool SendKey(int key_);
  virtual int ChangeConsole(int con_);
  virtual bool Update();

protected:
  int m_vcsFd;

private:
  // Not implemented
  OskConsole_Psp(const OskConsole_Psp &);
  OskConsole_Psp & operator = (const OskConsole_Psp &);
};


//-----------------------------------------------------------------------------
// Class: OskImage_Psp
//-----------------------------------------------------------------------------
OskImage_Psp::OskImage_Psp(ImageId imgId_)
  : OskImage( imgId_ ),
    m_imgData( *s_imgDataList[ imgId_ ] )
{
  m_width = m_imgData.width;
  m_height = m_imgData.height;
}


//-----------------------------------------------------------------------------
// Class: OskCanvas_Psp
//-----------------------------------------------------------------------------
OskCanvas_Psp::OskCanvas_Psp()
  : OskCanvas(),
    m_vramBase( NULL ),
    m_vramSize( 0 ),
    m_virtualWidth( 0 ),
    m_virtualHeight( 0 ),
    m_fbFd( -1 )
{
}
//-----------------------------------------------------------------------------
OskCanvas_Psp::~OskCanvas_Psp()
{
  if ( m_fbFd >= 0 )
  {
    if ( m_vramBase != NULL && m_vramSize != 0 )
    {
      (void)munmap( m_vramBase, m_vramSize );
      m_vramBase = NULL;
      m_vramSize = 0;
    }

    (void)close( m_fbFd );
    m_fbFd = -1;
  }
}
//-----------------------------------------------------------------------------
bool OskCanvas_Psp::Initialize(void * param_)
{
  struct fb_var_screeninfo vinfo;
  int rt;

  m_fbFd = open( c_fbDevName, O_RDWR );
  if ( m_fbFd < 0 )
  {
    DBG(( "OSK: Failed to open framebuffer device for canvas, err=%d\n", m_fbFd ));
    return false;
  }

  rt = ioctl( m_fbFd, FBIOGET_VSCREENINFO, &vinfo );
  if ( rt < 0 )
  {
    DBG(( "OSK: Failed to obtain framebuffer info, err=%d\n", rt ));
    return false;
  }

  m_vramSize = vinfo.xres_virtual *
               vinfo.yres_virtual *
               ( vinfo.bits_per_pixel >> 3 );
  m_width = vinfo.xres;
  m_height = vinfo.yres;
  m_virtualWidth = vinfo.xres_virtual;
  m_virtualHeight = vinfo.yres_virtual;

  m_vramBase = (unsigned long *)mmap( NULL,
                                      m_vramSize,
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      m_fbFd,
                                      0 );
  if ( (unsigned long)m_vramBase == (unsigned long)( -1 ) )
  {
    DBG(( "OSK: Failed to map framebuffer memory, err=%d\n", m_vramBase ));
    m_vramBase = NULL;
    m_vramSize = 0;
    return false;
  }

  return true;
}
//-----------------------------------------------------------------------------
bool OskCanvas_Psp::Clear
(
  int x_,
  int y_,
  int width_,
  int height_
)
{
  // Clear the drawing area first
  unsigned long * dest = m_vramBase + y_ * m_virtualWidth + x_;
  for ( int i = 0; i < height_; i++ )
  {
    memset( dest, 0x0, width_ << 2 );
    dest += m_virtualWidth;
  }

  (void)flush();
  return true;
}
//-----------------------------------------------------------------------------
bool OskCanvas_Psp::DrawImage
(
  int destX_,
  int destY_,
  const OskImage & img_,
  int sourX_,
  int sourY_,
  int width_,
  int height_
)
{
  const int imgWidth = img_.GetWidth();
  unsigned long * dest = m_vramBase + destY_ * m_virtualWidth + destX_;
  const unsigned long * sour = (const unsigned long *)img_.GetData() +
                               sourY_ * imgWidth + sourX_;

  for ( int i = 0; i < height_; i++ )
  {
    memcpy( dest, sour, width_ << 2 );
    dest += m_virtualWidth;
    sour += imgWidth;
  }

  (void)flush();
  return true;
}
//-----------------------------------------------------------------------------
bool OskCanvas_Psp::GetBits(void * buf_, int & size_)
{
  const int size = m_width * m_height * sizeof( unsigned long );

  if ( m_vramBase == NULL || buf_ == NULL || size_ < size )
    return false;

  unsigned long * dest = (unsigned long *)buf_;
  unsigned long * sour = m_vramBase;

  for ( int i = 0; i < m_height; i++ )
  {
    for ( int j = 0; j < m_width; j++ )
      dest[ j ] = convertPixel( sour[ j ] );

    dest += m_width;
    sour += m_virtualWidth;
  }

  size_ = size;
  return true;
}
//-----------------------------------------------------------------------------
unsigned long OskCanvas_Psp::convertPixel(unsigned long color_)
{
  return ( ( color_ & 0xff000000 ) |
         ( ( color_ & 0x00ff0000 ) >> 16 ) |
           ( color_ & 0x0000ff00 ) |
         ( ( color_ & 0x000000ff ) << 16 ) );
}
//-----------------------------------------------------------------------------
bool OskCanvas_Psp::flush()
{
  if ( m_fbFd < 0 )
  {
    return false;
  }

  return ( fsync( m_fbFd ) == 0 );
}


//-----------------------------------------------------------------------------
// Class: OskInput_Psp
//-----------------------------------------------------------------------------
OskInput_Psp::OskInput_Psp()
  : OskInput(),
    m_joypadFd( -1 )
{
}
//-----------------------------------------------------------------------------
OskInput_Psp::~OskInput_Psp()
{
  if ( m_joypadFd >= 0 )
  {
    (void)close( m_joypadFd );
    m_joypadFd = -1;
  }
}
//-----------------------------------------------------------------------------
bool OskInput_Psp::Initialize(void * param_)
{
  m_joypadFd = open( c_joypadDevName, O_RDONLY );
  if ( m_joypadFd < 0 )
  {
    DBG(( "OSK: Failed to open device for input, err=%d\n", m_joypadFd ));
    return false;
  }

  return true;
}
//-----------------------------------------------------------------------------
unsigned long OskInput_Psp::ReadKeys()
{
  if ( m_joypadFd < 0 )
  {
    DBG(( "OSK: Invalid device to read\n" ));
    return 0;
  }

  unsigned long keys;
  int rt = read( m_joypadFd, &keys, sizeof( keys ) );
  if ( rt < sizeof( keys ) )
  {
    DBG(( "OSK: Failed to read device, err=%d\n", rt ));
    return 0;
  }

  return keys;
}


//-----------------------------------------------------------------------------
// Class: OskConsole_Psp
//-----------------------------------------------------------------------------
OskConsole_Psp::OskConsole_Psp()
  : m_vcsFd( -1 )
{
}
//-----------------------------------------------------------------------------
OskConsole_Psp::~OskConsole_Psp()
{
  if ( m_vcsFd >= 0 )
  {
    (void)close( m_vcsFd );
    m_vcsFd = -1;
  }
}
//-----------------------------------------------------------------------------
bool OskConsole_Psp::Initialize(void * param_)
{
  m_vcsFd = open( c_vcsDevName, O_RDONLY );
  if ( m_vcsFd < 0 )
  {
    DBG(( "OSK: Failed to open device for console, err=%d\n", m_vcsFd ));
    return false;
  }

  return true;
}
//-----------------------------------------------------------------------------
bool OskConsole_Psp::SendKey(int key_)
{
  if ( m_vcsFd < 0 )
  {
    DBG(( "OSK: Invalid device to send key\n" ));
    return false;
  }

  char * c = (char *)&key_;
  for ( int i = 0; i < sizeof( key_ ); i++ )
  {
    if ( c[ i ] == 0 )
      continue;

    int rt = ioctl( m_vcsFd, PSP_VCS_IOCTL_PUTCHAR, (int)( c[ i ] ) );
    if ( rt < 0 )
    {
      DBG(( "OSK: Failed to send key %08x, err=%d\n", key_, rt ));
      return false;
    }
  } // end for

  return true;
}
//-----------------------------------------------------------------------------
int OskConsole_Psp::ChangeConsole(int con_)
{
  if ( m_vcsFd < 0 )
  {
    DBG(( "OSK: Invalid device to change console\n" ));
    return -1;
  }

  int rt = ioctl( m_vcsFd, PSP_VCS_IOCTL_CHANGE_CON, con_ );
  if ( rt < 0 )
  {
    DBG(( "OSK: Failed to change console to %d, err=%d\n", con_, rt ));
    return rt;
  }

  return rt;
}
//-----------------------------------------------------------------------------
bool OskConsole_Psp::Update()
{
  int rt = ioctl( m_vcsFd, PSP_VCS_IOCTL_UPDATE_SCR );
  if ( rt < 0 )
  {
    DBG(( "OSK: Failed to update screen, err=%d\n", rt ));
    return false;
  }

  return true;
}


//-----------------------------------------------------------------------------
// Class: OskFactory
//-----------------------------------------------------------------------------
OskImage * OskFactory::CreateImage(OskImage::ImageId imgId_)
{
  return new OskImage_Psp( imgId_ );
}
//-----------------------------------------------------------------------------
OskCanvas * OskFactory::CreateCanvas()
{
  return new OskCanvas_Psp();
}
//-----------------------------------------------------------------------------
OskInput * OskFactory::CreateInput()
{
  return new OskInput_Psp();
}
//-----------------------------------------------------------------------------
OskConsole * OskFactory::CreateConsole()
{
  return new OskConsole_Psp();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
