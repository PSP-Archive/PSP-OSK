/*-----------------------------------------------------------------------------
 * On-Screen Keyboard 2 for uClinux on PSP
 * Created by Jackson Mo, Jan 2, 2008
 *---------------------------------------------------------------------------*/
#ifndef OSK_H
#define OSK_H
//-----------------------------------------------------------------------------
#include <stdio.h>


//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#if 1
#define DEBUG   1
#endif

#ifdef DEBUG
  #ifdef WIN32
    extern int eprintf(const char * fmt_, ...);
    #define DBG(args)     eprintf args
  #else
    #define DBG(args)     printf args
  #endif
#else
  #define DBG(args)
#endif


//-----------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------
typedef enum
{
  KDID_Left = 0,
  KDID_Top,
  KDID_Right,
  KDID_Bottom,

  KDID_Count
} OskKeyDirectionId;

typedef enum
{
  KSID_TopLeft = 0,
  KSID_Top,
  KSID_TopRight,
  KSID_Left,
  KSID_Center,
  KSID_Right,
  KSID_BottomLeft,
  KSID_Bottom,
  KSID_BottomRight,

  KSID_Count
} OskKeySectionId;

typedef enum
{
  KBID_Eng = 0,
  KBID_Cap,
  KBID_Num,

  KBID_Count  
} OskKeyboardId;

typedef struct
{
  int keys[ KDID_Count ];
} OskKeySection;

typedef struct
{
  OskKeySection sections[ KSID_Count ];
} OskKeyboard;

typedef enum
{
  KEY_ENTER     = '\n',
  KEY_TAB       = '\t',
  KEY_BACKSPACE = 0x7f,
  KEY_CTRL_C    = 0x3,
  KEY_ESCAPE    = 0x1b,
  KEY_DEL       = 0x7e331b,
  KEY_UP        = 0x415b1b,
  KEY_DOWN      = 0x425b1b,
  KEY_RIGHT     = 0x435b1b,
  KEY_LEFT      = 0x445b1b
} OskSepcialKey;


//-----------------------------------------------------------------------------
// Static data
//-----------------------------------------------------------------------------
extern OskKeyboard s_OskKeyboards[ KBID_Count ];


//-----------------------------------------------------------------------------
// Classes
//-----------------------------------------------------------------------------
class OskImage;
class OskCanvas;
class OskInput;
class OskConsole;
class OskFactory;
class OskCore;


//-----------------------------------------------------------------------------
// Class: OskImage
//-----------------------------------------------------------------------------
class OskImage
{
public:
  typedef enum
  {
    IMGID_First = 0,

    IMGID_Eng = IMGID_First,
    IMGID_EngActive,
    IMGID_Cap,
    IMGID_CapActive,
    IMGID_Num,
    IMGID_NumActive,
    IMGID_Mouse,

    IMGID_Count
  } ImageId;

  OskImage(ImageId imgId_);
  virtual ~OskImage() { }

  virtual const void * GetData() const = 0;

  int GetId() const
  {
    return m_imgId;
  }

  int GetWidth() const
  {
    return m_width;
  }

  int GetHeight() const
  {
    return m_height;
  }

protected:
  const ImageId m_imgId;
  int m_width;
  int m_height;

private:
  // Not implemented
  OskImage();
  OskImage(const OskImage &);
  OskImage & operator = (const OskImage &);
};


//-----------------------------------------------------------------------------
// Class: OskCanvas
//-----------------------------------------------------------------------------
class OskCanvas
{
public:
  OskCanvas();
  virtual ~OskCanvas() { }

  virtual bool Initialize(void * param_) = 0;

  virtual bool Clear
  (
    int x_,
    int y_,
    int width_,
    int height_
  ) = 0;

  virtual bool DrawImage
  (
    int destX_,
    int destY_,
    const OskImage & img_,
    int sourX_,
    int sourY_,
    int width_,
    int height_
  ) = 0;

  virtual bool GetBits(void * buf_, int & size_) = 0;

  int GetWidth() const
  {
    return m_width;
  }

  int GetHeight() const
  {
    return m_height;
  }

protected:
  int m_width;
  int m_height;

private:
  // Not implemented
  OskCanvas(const OskCanvas &);
  OskCanvas & operator = (const OskCanvas &);
};


//-----------------------------------------------------------------------------
// Class: OskInput
//-----------------------------------------------------------------------------
class OskInput
{
public:
  typedef enum
  {
    KEY_ARROW_UP   = 0x00000001,
    KEY_ARROW_RT   = 0x00000002,
    KEY_ARROW_DN   = 0x00000004,
    KEY_ARROW_LT   = 0x00000008,
    KEY_TRIANGLE   = 0x00000010,
    KEY_CIRCLE     = 0x00000020,
    KEY_CROSS      = 0x00000040,
    KEY_RECTANGLE  = 0x00000080,
    KEY_SELECT     = 0x00000100,
    KEY_LTRG       = 0x00000200,
    KEY_RTRG       = 0x00000400,
    KEY_START      = 0x00000800,
    KEY_HOME       = 0x00001000,
    KEY_HOLD       = 0x00002000,
    KEY_WLAN       = 0x00004000,
    KEY_HR_EJ      = 0x00008000,
    KEY_VOL_UP     = 0x00010000,
    KEY_VOL_DN     = 0x00020000,
    KEY_LCD        = 0x00040000,
    KEY_NOTE       = 0x00080000,
    KEY_UMD_EJCT   = 0x00100000,
    KEY_UNKNOWN    = 0x00200000,
    KEY_MOUSE_MODE = 0x00800000,
  } KeyMask;

  OskInput() { }
  virtual ~OskInput() { }

  virtual bool Initialize(void * param_) = 0;
  virtual unsigned long ReadKeys() = 0;

protected:

private:
  // Not implemented
  OskInput(const OskInput &);
  OskInput & operator = (const OskInput &);
};


//-----------------------------------------------------------------------------
// Class: OskConsole
//-----------------------------------------------------------------------------
class OskConsole
{
public:
  OskConsole() { }
  virtual ~OskConsole() { }

  virtual bool Initialize(void * param_) = 0;
  virtual bool SendKey(int key_) = 0;
  virtual int ChangeConsole(int con_) = 0;
  virtual bool Update() = 0;

protected:

private:
  // Not implemented
  OskConsole(const OskConsole &);
  OskConsole & operator = (const OskConsole &);
};


//-----------------------------------------------------------------------------
// Class: OskFactory
//-----------------------------------------------------------------------------
class OskFactory
{
public:
  static OskImage * CreateImage(OskImage::ImageId imgId_);
  static OskCanvas * CreateCanvas();
  static OskInput * CreateInput();
  static OskConsole * CreateConsole();

protected:
private:
  OskFactory();
  OskFactory(const OskFactory &);
  OskFactory & operator = (const OskFactory &);
};


//-----------------------------------------------------------------------------
// Class: OskCore
//-----------------------------------------------------------------------------
class OskCore
{
public:
  typedef enum
  {
    FLAGS_USE_DPAD    = 0x00000001,
    FLAGS_USE_ANALOG  = 0x00000002,
    FLAGS_EXIT        = 0xffffffff,
  } OskFlags;

  typedef enum
  {
    ANALOG_POS_CENTER,
    ANALOG_POS_LEFT,
    ANALOG_POS_UPLEFT,
    ANALOG_POS_UP,
    ANALOG_POS_UPRIGHT,
    ANALOG_POS_RIGHT,
    ANALOG_POS_DOWNRIGHT,
    ANALOG_POS_DOWN,
    ANALOG_POS_DOWNLEFT,
  } OskAnalogPos;

  OskCore(const OskFlags flags_, const int numVts_);
  virtual ~OskCore();

  static bool ParseFlags(const char * cmdline_, OskFlags & flags_, int & numVts_);

  bool Initialize(void * param1_, void * param2_);
  void Main();

protected:
  // Internal states
  class BaseState;
    class FailedState;
    class KbdState;
      class IdleState;
      class ActiveState;
        class ActiveEngState;
        class ActiveCapState;
        class ActiveNumState;
    class MouseState;
  
  #define OSK_STATES_H
  #include "oskstates.h"
  #undef  OSK_STATES_H

  void changeState(BaseState * newState_);
  bool clear(OskImage::ImageId imgId_);
  bool clear();
  bool drawImage(OskImage::ImageId imgId_);
  bool drawImageSection
  (
    OskImage::ImageId imgId_,
    OskKeySectionId sectionId_
  );
  bool drawImageSectionSingle
  (
    OskImage::ImageId imgId_,
    OskKeySectionId sectionId_
  );
  bool sendKey(int key_);
  bool changeConsole(int gain_);
  static int normalizePos(unsigned long p_);
  OskAnalogPos getAnalogPos();
  bool takeScreenshot();
  static FILE * createNewFile();
  static void shutdown();
  static void showVersion();
  static void showHelp();
  
  const OskFlags  c_flags;
  const int       c_numVts;

  bool            m_initialized;
  OskImage *      m_images[ OskImage::IMGID_Count ];
  OskCanvas *     m_canvas;
  OskInput *      m_input;
  OskConsole *    m_console;
  BaseState *     m_currentState;
  unsigned long   m_keys;
  int             m_activeConsole;

  FailedState     m_failedState;
  IdleState       m_idleState;
  ActiveEngState  m_activeEngState;
  ActiveCapState  m_activeCapState;
  ActiveNumState  m_activeNumState;
  MouseState      m_mouseState;

private:
  // Not implemented
  OskCore();
  OskCore(const OskCore &);
  OskCore & operator = (const OskCore &);

  friend class BaseState;
  friend class FailedState;
  friend class IdleState;
  friend class ActiveState;
  friend class ActiveEngState;
  friend class ActiveCapState;
  friend class ActiveNumState;
  friend class MouseState;
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif
