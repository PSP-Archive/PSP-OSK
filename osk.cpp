/*-----------------------------------------------------------------------------
 * On-Screen Keyboard 2 for uClinux on PSP
 * Created by Jackson Mo, Jan 2, 2008
 *---------------------------------------------------------------------------*/
#include "osk.h"
#include <unistd.h>
#include <string.h>


//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
typedef struct
{
  int xoff, yoff;
} OskImageSectionOffset;


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const unsigned long AnalogPosLowerThreshold = 0x3;
static const unsigned long AnalogPosUpperThreshold = 0xc;
static const int DefaultNumVirtualTerminals = 4;
static const char PowerOffCommand[] = "/sbin/poweroff";


//-----------------------------------------------------------------------------
// Static Data
//-----------------------------------------------------------------------------
OskKeyboard s_OskKeyboards[ KBID_Count ] =
{
  // KBID_Eng
  {{
    { 'e', 'f', 'g', 'h' },   // KSID_TopLeft
    { 'i', 'j', 'k', 'l' },   // KSID_Top
    { 'm', 'n', 'o', 'p' },   // KSID_TopRight
    { 'a', 'b', 'c', 'd' },   // KSID_Left
    { KEY_BACKSPACE, ' ', KEY_ENTER, KEY_ESCAPE },  // KSID_Center
    { 'q', 'r', 's', 't' },   // KSID_Right
    { '<', '[', '>', ']' },   // KSID_BottomLeft
    { 'y', '.', 'z', ',' },   // KSID_Bottom
    { 'u', 'v', 'w', 'x' },   // KSID_BottomRight
  }},

  // KBID_Cap
  {{
    { 'E', 'F', 'G', 'H' },   // KSID_TopLeft
    { 'I', 'J', 'K', 'L' },   // KSID_Top
    { 'M', 'N', 'O', 'P' },   // KSID_TopRight
    { 'A', 'B', 'C', 'D' },   // KSID_Left
    { KEY_DEL, KEY_TAB, KEY_ENTER, KEY_CTRL_C },  // KSID_Center
    { 'Q', 'R', 'S', 'T' },   // KSID_Right
    { '(', '{', ')', '}' },   // KSID_BottomLeft
    { 'Y', '.', 'Z', ',' },   // KSID_Bottom
    { 'U', 'V', 'W', 'X' },   // KSID_BottomRight
    }},

  // KBID_Num
  {{
    { '1', '2', '3', '4' },   // KSID_TopLeft    
    { '5', '6', '7', '8' },   // KSID_Top        
    { '9', '\"', '0', '\'' }, // KSID_TopRight   
    { '+', '-', '*', '\\' },  // KSID_Left       
    { KEY_DEL, KEY_TAB, KEY_ENTER, KEY_CTRL_C },  // KSID_Center
    { '@', '|', '?', '/' },   // KSID_Right      
    { '#', '~', '!', '`' },   // KSID_BottomLeft 
    { ';', '.', ':', '$' },   // KSID_Bottom     
    { '&', '^', '%', '=' },    // KSID_BottomRight
  }},
};

static OskImageSectionOffset s_sectionOffset[ KSID_Count ] =
{
  { 0, 0 },   // KSID_TopLeft
  { 1, 0 },   // KSID_Top
  { 2, 0 },   // KSID_TopRight
  { 0, 1 },   // KSID_Left
  { 1, 1 },   // KSID_Center
  { 2, 1 },   // KSID_Right
  { 0, 2 },   // KSID_BottomLeft
  { 1, 2 },   // KSID_Bottom
  { 2, 2 },   // KSID_BottomRight
};

static unsigned char s_screenshotHeader[ 0x36 ] = {
	0x42, 0x4D, 0x38, 0xF8, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
	0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0xF0, 0xFE,
  0xFF, 0xFF, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x0B,
  0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static char s_screenshotFileName[] = "/usr/screenshots/screenshot%04d.bmp";


//-----------------------------------------------------------------------------
// Class: OskImage
//-----------------------------------------------------------------------------
OskImage::OskImage(ImageId imgId_)
  : m_imgId( imgId_ ),
    m_width( 0 ),
    m_height( 0 )
{
}


//-----------------------------------------------------------------------------
// Class: OskCanvas
//-----------------------------------------------------------------------------
OskCanvas::OskCanvas()
  : m_width( 0 ),
    m_height( 0 )
{
}


//-----------------------------------------------------------------------------
// Class: OskInput
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Class: OskConsole
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Class: OskFactory
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Class: OskCore
//-----------------------------------------------------------------------------
OskCore::OskCore(const OskFlags flags_, const int numVts_)
  : c_flags( flags_ ),
    c_numVts( numVts_ ),
    m_initialized( false ),
    m_canvas( NULL ),
    m_input( NULL ),
    m_console( NULL ),
    m_currentState( &m_failedState ),
    m_keys( 0 ),
    m_activeConsole( 0 ),
    // Internal states
    m_failedState( *this ),
    m_idleState( *this ),
    m_activeEngState( *this ),
    m_activeCapState( *this ),
    m_activeNumState( *this ),
    m_mouseState( *this )
{
}
//-----------------------------------------------------------------------------
OskCore::~OskCore()
{
  for ( int id = OskImage::IMGID_First; id < OskImage::IMGID_Count; id++ )
  {
    if ( m_images[ id ] != NULL )
    {
      delete m_images[ id ];
      m_images[ id ] = NULL;
    }
  }

  if ( m_canvas != NULL )
  {
    delete m_canvas;
    m_canvas = NULL;
  }

  if ( m_input != NULL )
  {
    delete m_input;
    m_input = NULL;
  }

  if ( m_console != NULL )
  {
    delete m_console;
    m_console = NULL;
  }
}
//-----------------------------------------------------------------------------
bool OskCore::ParseFlags
(
  const char * cmdline_,
  OskFlags & flags_,
  int & numVts_
)
{
  if ( cmdline_ == NULL || strcmp( cmdline_, "--help" ) == 0 )
  {
    showHelp();
    return false;
  }
  else if ( strcmp( cmdline_, "--version" ) == 0 )
  {
    showVersion();
    return false;
  }

  unsigned long flags = (unsigned long)FLAGS_USE_ANALOG;
  int numVts = DefaultNumVirtualTerminals;

  for ( const char * c = cmdline_; *c != 0; c++ )
  {
    if ( *c == '-' || *c == 's' )
    {
      continue;
    }
    else if ( *c == 'd' )
    {
      flags &= ~(unsigned long)FLAGS_USE_ANALOG;
      flags |= (unsigned long)FLAGS_USE_DPAD;
    }
    else if ( *c == 'D' )
    {
      flags |= ( (unsigned long)FLAGS_USE_DPAD | (unsigned long)FLAGS_USE_ANALOG );
    }
    else if ( *c == 'v' )
    {
      c++;
      if ( '1' <= *c && *c <= '6' )
      {
        numVts = (int)( *c - '0' );
      }
    }
  }

  flags_ = (OskFlags)flags;
  numVts_ = numVts;

  return true;
}
//-----------------------------------------------------------------------------
bool OskCore::Initialize(void * param1_, void * param2_)
{
  if ( m_initialized )
  {
    DBG(( "OSK: OskCore has been initialized\n" ));
    return true;
  }

  for ( int id = OskImage::IMGID_First; id < OskImage::IMGID_Count; id++ )
  {
    m_images[ id ] = OskFactory::CreateImage( (OskImage::ImageId)id );
    if ( m_images[ id ] == NULL )
    {
      DBG(( "OSK: Failed to create image: %d\n", id ));
      return false;
    }
  }

  m_canvas = OskFactory::CreateCanvas();
  if ( m_canvas == NULL )
  {
    DBG(( "OSK: Failed to create agent: canvas\n" ));
    return false;
  }

  if ( !m_canvas->Initialize( param1_ ) )
  {
    DBG(( "OSK: Failed to initialize agent: canvas\n" ));
    return false;
  }

  m_input = OskFactory::CreateInput();
  if ( m_input == NULL )
  {
    DBG(( "OSK: Failed to create agent: input\n" ));
    return false;
  }

  if ( !m_input->Initialize( param2_ ) )
  {
    DBG(( "OSK: Failed to initialize agent: input\n" ));
    return false;
  }

  m_console = OskFactory::CreateConsole();
  if ( m_console == NULL )
  {
    DBG(( "OSK: Failed to create agent: console\n" ));
    return false;
  }

  if ( !m_console->Initialize( param1_ ) )
  {
    DBG(( "OSK: Failed to initialize agent: console\n" ));
    return false;
  }

  // Get the initial console index
  m_activeConsole = m_console->ChangeConsole( -1 );

  m_initialized = true;
  return true;
}
//-----------------------------------------------------------------------------
void OskCore::Main()
{
  if ( !m_initialized )
    return;

  // Starts from the Idle state
  changeState( &m_idleState );

  while ( !m_currentState->IsTerminated() )
  {
    m_keys = m_input->ReadKeys();
    changeState( m_currentState->processKeys() );
  }
}
//-----------------------------------------------------------------------------
void OskCore::changeState(BaseState * newState_)
{
  while ( m_currentState != newState_ )
  {
    m_currentState->exitState();
    m_currentState = newState_;
    newState_ = m_currentState->enterState();
  }
}
//-----------------------------------------------------------------------------
bool OskCore::clear(OskImage::ImageId imgId_)
{
  const OskImage * const img = m_images[ imgId_ ];

  if ( m_canvas == NULL || m_console == NULL || img == NULL )
    return false;

  const int x = m_canvas->GetWidth() - img->GetWidth();
  const int y = 0;
  const int width = img->GetWidth();
  const int height = img->GetHeight();

  if ( !m_canvas->Clear( x, y, width, height ) )
    return false;

  if ( !m_console->Update() )
    return false;

  return true;
}
//-----------------------------------------------------------------------------
bool OskCore::clear()
{
  return clear( OskImage::IMGID_First );
}
//-----------------------------------------------------------------------------
bool OskCore::drawImage(OskImage::ImageId imgId_)
{
  const OskImage * const img = m_images[ imgId_ ];

  if ( m_canvas == NULL || img == NULL )
    return false;

  const int x = m_canvas->GetWidth() - img->GetWidth();
  const int y = 0;
  const int width = img->GetWidth();
  const int height = img->GetHeight();

  return m_canvas->DrawImage( x, y, *img, 0, 0, width, height );
}
//-----------------------------------------------------------------------------
bool OskCore::drawImageSection
(
  OskImage::ImageId imgId_,
  OskKeySectionId sectionId_
)
{
  const OskImage * const img = m_images[ imgId_ ];

  if ( m_canvas == NULL || img == NULL )
    return false;

  const int x = m_canvas->GetWidth() - img->GetWidth();
  const int y = 0;
  const int sourWidth = img->GetWidth() / 3;
  const int sourHeight = img->GetHeight() / 3;
  const int sourX = sourWidth * s_sectionOffset[ sectionId_ ].xoff;
  const int sourY = sourHeight * s_sectionOffset[ sectionId_ ].yoff;

  return m_canvas->DrawImage( x + sourX, y + sourY,
                              *img, 
                              sourX, sourY, sourWidth, sourHeight );
}
//-----------------------------------------------------------------------------
bool OskCore::drawImageSectionSingle
(
  OskImage::ImageId imgId_,
  OskKeySectionId sectionId_
)
{
  const OskImage * const img = m_images[ imgId_ ];

  if ( m_canvas == NULL || img == NULL )
    return false;

  const int sourWidth = img->GetWidth() / 3;
  const int sourHeight = img->GetHeight() / 3;
  const int sourX = sourWidth * s_sectionOffset[ sectionId_ ].xoff;
  const int sourY = sourHeight * s_sectionOffset[ sectionId_ ].yoff;
  const int x = m_canvas->GetWidth() - sourWidth;
  const int y = 0;

  return m_canvas->DrawImage( x, y,
                              *img,
                              sourX, sourY, sourWidth, sourHeight );
}
//-----------------------------------------------------------------------------
bool OskCore::sendKey(int key_)
{
  if ( m_console == NULL )
  {
    return false;
  }

  return m_console->SendKey( key_ );
}
//-----------------------------------------------------------------------------
bool OskCore::changeConsole(int gain_)
{
  if ( m_console == NULL )
  {
    return false;
  }

  m_activeConsole += gain_;
  if ( m_activeConsole < 0 )
  {
    m_activeConsole = c_numVts - 1;
  }
  else if ( m_activeConsole >= c_numVts )
  {
    m_activeConsole = 0;
  }

  m_activeConsole = m_console->ChangeConsole( m_activeConsole );
  return ( m_activeConsole >= 0 );
}
//-----------------------------------------------------------------------------
int OskCore::normalizePos(unsigned long p_)
{
  if ( p_ < AnalogPosLowerThreshold )
  {
    return -1;
  }
  else if ( AnalogPosLowerThreshold <= p_ && p_ <= AnalogPosUpperThreshold )
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
//-----------------------------------------------------------------------------
OskCore::OskAnalogPos OskCore::getAnalogPos()
{
  int x = normalizePos( ( m_keys >> 24 ) & 0xf );
  int y = normalizePos( ( m_keys >> 28 ) & 0xf );

  if ( x < 0 && y == 0 )
  {
    return ANALOG_POS_LEFT;
  }

  if ( x < 0 && y < 0 )
  {
    return ANALOG_POS_UPLEFT;
  }

  if ( x == 0 && y < 0 )
  {
    return ANALOG_POS_UP;
  }

  if ( x > 0 && y < 0 )
  {
    return ANALOG_POS_UPRIGHT;
  }

  if ( x > 0 && y == 0 )
  {
    return ANALOG_POS_RIGHT;
  }

  if ( x > 0 && y > 0 )
  {
    return ANALOG_POS_DOWNRIGHT;
  }

  if ( x == 0 && y > 0 )
  {
    return ANALOG_POS_DOWN;
  }

  if ( x < 0 && y > 0 )
  {
    return ANALOG_POS_DOWNLEFT;
  }

  return ANALOG_POS_CENTER;
}
//-----------------------------------------------------------------------------
bool OskCore::takeScreenshot()
{
  if ( m_canvas == NULL )
  {
    return false;
  }

  const int screenSize = m_canvas->GetWidth() * m_canvas->GetHeight();
  unsigned long * buf = new unsigned long[ screenSize ];
  if ( buf == NULL )
  {
    return false;
  }

  int bufSize = screenSize * sizeof( unsigned long );
  if ( !m_canvas->GetBits( buf, bufSize ) )
  {
    delete[] buf;
    return false;
  }

  FILE * newFile = createNewFile();
  if ( newFile == NULL )
  {
    delete[] buf;
    return false;
  }

  bool rt;
  do {
    // Write the header first
    if ( sizeof( s_screenshotHeader ) !=
         fwrite( s_screenshotHeader, 1, sizeof( s_screenshotHeader ), newFile ) )
    {
      rt = false;
      break;
    }

    // Write the buffer
    if ( bufSize != fwrite( buf, 1, bufSize, newFile ) )
    {
      rt = false;
      break;
    }

    // Write the ending
    unsigned short ending = 0;
    if ( sizeof( ending ) != fwrite( &ending, 1, sizeof( ending ), newFile ) )
    {
      rt = false;
      break;
    }

    // Succeed if it gets here
    rt = true;
  } while ( false );

  delete[] buf;
  (void)fclose( newFile );

  return rt;
}
//-----------------------------------------------------------------------------
FILE * OskCore::createNewFile()
{
  for ( int i = 1; i < 10000; i++ )
  {
    const size_t fileNameLen = 80;
    char fileName[ fileNameLen + 1 ];

    (void)snprintf( fileName, fileNameLen, s_screenshotFileName, i );
    fileName[ fileNameLen ] = 0;

    // See if the file already exists
    FILE * newFile = fopen( fileName, "rb" );
    if ( newFile != NULL )
    {
      (void)fclose( newFile );
      continue;
    }

    // The file does not exist
    newFile = fopen( fileName, "wb" );
    if ( newFile != NULL )
    {
      return newFile;
    }
  }

  return NULL;
}
//-----------------------------------------------------------------------------
void OskCore::shutdown()
{
  if ( fork() == 0 )
  {
    // In the child process
    _exit( execl( PowerOffCommand, PowerOffCommand, NULL ) );
  }
}
//-----------------------------------------------------------------------------
void OskCore::showVersion()
{
  printf( "On-Screen Keyboard 2.2 for PSP, created by Jackson Mo\n" );
}
//-----------------------------------------------------------------------------
void OskCore::showHelp()
{
  showVersion();
  printf( "Usage: psposk2 [--help|--version|-dDv<num>s]\n"
          "  --help     Print this help\n"
          "  --version  Print version info\n"
          "  -d         Use only dpad in keyboard mode\n"
          "  -D         Use both dpad and analog in keyboard mode\n"
          "  -v<num>    Specify the number (1-6) of virtual terminals you want to have\n"
          "  -s         Silent mode\n" );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
