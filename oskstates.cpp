/*-----------------------------------------------------------------------------
 * On-Screen Keyboard 2 for uClinux on PSP
 * Created by Jackson Mo, Jan 2, 2008
 *---------------------------------------------------------------------------*/
#include "osk.h"


//-----------------------------------------------------------------------------
// Static data
//-----------------------------------------------------------------------------
const OskImage::ImageId OskCore::ActiveState::c_kbdImg[ KBID_Count ] =
{
  OskImage::IMGID_Eng,  // KBID_Eng
  OskImage::IMGID_Cap,  // KBID_Cap
  OskImage::IMGID_Num,  // KBID_Num
};

const OskImage::ImageId OskCore::ActiveState::c_kbdImgActive[ KBID_Count ] =
{
  OskImage::IMGID_EngActive,  // KBID_Eng
  OskImage::IMGID_CapActive,  // KBID_Cap
  OskImage::IMGID_NumActive,  // KBID_Num
};


//-----------------------------------------------------------------------------
// Class: OskCore::BaseState
//-----------------------------------------------------------------------------
OskCore::BaseState::BaseState(OskCore & core_)
  : m_core( core_ )
{
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::BaseState::enterState()
{
  // Do nothing
  return this;
}
//-----------------------------------------------------------------------------
void OskCore::BaseState::exitState()
{
  // Do nothing
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::BaseState::processKeys()
{
  // +
  if ( m_core.m_keys & OskInput::KEY_VOL_UP )
  {
    m_core.m_keys &= ~OskInput::KEY_VOL_UP;
    (void)m_core.changeConsole( 1 );
  }

  // -
  if ( m_core.m_keys & OskInput::KEY_VOL_DN )
  {
    m_core.m_keys &= ~OskInput::KEY_VOL_DN;
    (void)m_core.changeConsole( -1 );
  }

  // LCD
  if ( m_core.m_keys & OskInput::KEY_LCD )
  {
    m_core.m_keys &= ~OskInput::KEY_LCD;
    if ( !m_core.takeScreenshot() )
    {
      DBG(( "OSK: Failed to take screenshot\n" ));
    }
  }

  // HOME + CIRCLE + CROSS
  if ( ( m_core.m_keys & OskInput::KEY_HOME ) &&
       ( m_core.m_keys & OskInput::KEY_CIRCLE ) &&
       ( m_core.m_keys & OskInput::KEY_CROSS ) )
  {
    m_core.m_keys &= ~( OskInput::KEY_HOME |
                        OskInput::KEY_CIRCLE |
                        OskInput::KEY_CROSS );
    (void)m_core.shutdown();
  }

  return this;
}


//-----------------------------------------------------------------------------
// Class: OskCore::FailedState
//-----------------------------------------------------------------------------
OskCore::FailedState::FailedState(OskCore & core_)
  : BaseState( core_ )
{
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::FailedState::enterState()
{
  DBG(( "OSK: Program terminated after encountering an error\n" ));
  return this;
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::FailedState::processKeys()
{
  return this;
}


//-----------------------------------------------------------------------------
// Class: OskCore::MouseState
//-----------------------------------------------------------------------------
OskCore::MouseState::MouseState(OskCore & core_)
  : BaseState( core_ )
{
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::MouseState::enterState()
{
  if ( !draw() )
    return &m_core.m_failedState;

  return this;
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::MouseState::processKeys()
{
  BaseState * newState = BaseState::processKeys();
  if ( newState != this )
  {
    return newState;
  }

  // MOUSE_MODE
  if ( m_core.m_keys & OskInput::KEY_MOUSE_MODE )
  {
    return this;
  }

  return &m_core.m_idleState;
}
//-----------------------------------------------------------------------------
bool OskCore::MouseState::draw()
{
  if ( !m_core.clear() )
    return false;

  if ( !m_core.drawImage( OskImage::IMGID_Mouse ) )
    return false;

  return true;
}


//-----------------------------------------------------------------------------
// Class: OskCore::KbdState
//-----------------------------------------------------------------------------
OskCore::KbdState::KbdState(OskCore & core_)
  : BaseState( core_ )
{
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::KbdState::enterState()
{
  BaseState * newState = processKeys();
  if ( newState != this )
  {
    return newState;
  }

  return this;
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::KbdState::processKeys()
{
  BaseState * newState = BaseState::processKeys();
  if ( newState != this )
  {
    return newState;
  }

  // MOUSE_MODE
  if ( m_core.m_keys & OskInput::KEY_MOUSE_MODE )
  {
    return &m_core.m_mouseState;
  }

  // L + R
  if ( ( m_core.m_keys & OskInput::KEY_LTRG ) &&
       ( m_core.m_keys & OskInput::KEY_RTRG ) )
  {
    return &m_core.m_activeCapState;
  }

  // R
  if ( m_core.m_keys & OskInput::KEY_RTRG )
  {
    return &m_core.m_activeEngState;
  }

  // L
  if ( m_core.m_keys & OskInput::KEY_LTRG )
  {
    return &m_core.m_activeNumState;
  }

  return &m_core.m_idleState;
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::KbdState::processKeysFinal
(
  OskKeyboardId kbdId_,
  OskKeySectionId secId_
)
{
  const OskKeySection & section =
      s_OskKeyboards[ kbdId_ ].sections[ secId_ ];

  // Rectangle
  if ( m_core.m_keys & OskInput::KEY_RECTANGLE )
  {
    m_core.m_keys &= ~OskInput::KEY_RECTANGLE;
    (void)m_core.sendKey( section.keys[ KDID_Left ] );
  }

  // Triangle
  if ( m_core.m_keys & OskInput::KEY_TRIANGLE )
  {
    m_core.m_keys &= ~OskInput::KEY_TRIANGLE;
    (void)m_core.sendKey( section.keys[ KDID_Top ] );
  }

  // Circle
  if ( m_core.m_keys & OskInput::KEY_CIRCLE )
  {
    m_core.m_keys &= ~OskInput::KEY_CIRCLE;
    (void)m_core.sendKey( section.keys[ KDID_Right ] );
  }

  // Cross
  if ( m_core.m_keys & OskInput::KEY_CROSS )
  {
    m_core.m_keys &= ~OskInput::KEY_CROSS;
    (void)m_core.sendKey( section.keys[ KDID_Bottom ] );
  }

  return this;
}


//-----------------------------------------------------------------------------
// Class: OskCore::IdleState
//-----------------------------------------------------------------------------
OskCore::IdleState::IdleState(OskCore & core_)
  : KbdState( core_ )
{
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::IdleState::enterState()
{
  BaseState * newState = KbdState::enterState();
  if ( newState != this )
  {
    return newState;
  }

  if ( !draw() )
    return &m_core.m_failedState;

  return this;
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::IdleState::processKeys()
{
  BaseState * newState = KbdState::processKeys();
  if ( newState != this )
  {
    return newState;
  }

  // Up
  if ( m_core.m_keys & OskInput::KEY_ARROW_UP )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_UP;
    (void)m_core.sendKey( KEY_UP );
  }

  // Right
  if ( m_core.m_keys & OskInput::KEY_ARROW_RT )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_RT;
    (void)m_core.sendKey( KEY_RIGHT );
  }

  // Down
  if ( m_core.m_keys & OskInput::KEY_ARROW_DN )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_DN;
    (void)m_core.sendKey( KEY_DOWN );
  }

  // Left
  if ( m_core.m_keys & OskInput::KEY_ARROW_LT )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_LT;
    (void)m_core.sendKey( KEY_LEFT );
  }

  // Other keys
  return processKeysFinal( KBID_Eng, KSID_Center );
}
//-----------------------------------------------------------------------------
bool OskCore::IdleState::draw()
{
  if ( !m_core.clear() )
    return false;

  if ( !m_core.drawImageSectionSingle( OskImage::IMGID_EngActive, KSID_Center ) )
    return false;

  return true;
}


//-----------------------------------------------------------------------------
// Class: OskCore::ActiveState
//-----------------------------------------------------------------------------
OskCore::ActiveState::ActiveState(OskCore & core_,   OskKeyboardId kbdId_)
  : KbdState( core_ ),
    c_kbdId( kbdId_ ),
    m_activeSection( KSID_Count )
{
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::ActiveState::enterState()
{
  m_activeSection = KSID_Count;

  return KbdState::enterState();
}
//-----------------------------------------------------------------------------
OskCore::BaseState * OskCore::ActiveState::processKeys()
{
  BaseState * newState = KbdState::processKeys();
  if ( newState != this )
  {
    return newState;
  }

  const OskAnalogPos analogPos = m_core.getAnalogPos();

  // Top left
  if (
       (
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_UP ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_LT )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_UPLEFT )
       )
     )
  {
    m_core.m_keys &= ~( OskInput::KEY_ARROW_UP | OskInput::KEY_ARROW_LT );

    if ( !update( KSID_TopLeft ) )
      return &m_core.m_failedState;

    return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Top right
  if (
       (
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_UP ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_RT )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_UPRIGHT )
       )
     )
  {
    m_core.m_keys &= ~( OskInput::KEY_ARROW_UP | OskInput::KEY_ARROW_RT );

    if ( !update( KSID_TopRight ) )
      return &m_core.m_failedState;

    return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Bottom left
  if ( 
       (    
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_DN ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_LT )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_DOWNLEFT )
       )
     )
  {
    m_core.m_keys &= ~( OskInput::KEY_ARROW_DN | OskInput::KEY_ARROW_LT );

    if ( !update( KSID_BottomLeft ) )
      return &m_core.m_failedState;

    return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Bottom right
  if (
       (
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_DN ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_RT )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_DOWNRIGHT )
       )
     )
  {
    m_core.m_keys &= ~( OskInput::KEY_ARROW_DN | OskInput::KEY_ARROW_RT );

    if ( !update( KSID_BottomRight ) )
      return &m_core.m_failedState;

    return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Top
  if (
       (
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_UP )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_UP )
       )
     )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_UP;

    if ( !update( KSID_Top ) )
      return &m_core.m_failedState;

    return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Bottom
  if (
       (
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_DN )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_DOWN )
       )
     )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_DN;
 
    if ( !update( KSID_Bottom ) )
      return &m_core.m_failedState;

   return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Left
  if (
       (
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_LT )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_LEFT )
       )
     )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_LT;
 
    if ( !update( KSID_Left ) )
      return &m_core.m_failedState;

   return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Right
  if (
       (
         ( m_core.c_flags & FLAGS_USE_DPAD ) &&
         ( m_core.m_keys & OskInput::KEY_ARROW_RT )
       ) ||
       (
         ( m_core.c_flags & FLAGS_USE_ANALOG ) &&
         ( analogPos == ANALOG_POS_RIGHT )
       )
     )
  {
    m_core.m_keys &= ~OskInput::KEY_ARROW_RT;
 
    if ( !update( KSID_Right ) )
      return &m_core.m_failedState;

   return processKeysFinal( c_kbdId, m_activeSection );
  }

  // Center
  if ( !update( KSID_Center ) )
    return &m_core.m_failedState;

  return processKeysFinal( c_kbdId, m_activeSection );
}
//-----------------------------------------------------------------------------
bool OskCore::ActiveState::update(OskKeySectionId activeSection_)
{
  if ( activeSection_ != m_activeSection )
  {
    m_activeSection = activeSection_;
    return draw();
  }

  return true;
}
//-----------------------------------------------------------------------------
bool OskCore::ActiveState::draw()
{
  if ( !m_core.drawImage( c_kbdImg[ c_kbdId ] ) )
    return false;

  if ( !m_core.drawImageSection( c_kbdImgActive[ c_kbdId ],
                                 m_activeSection ) )
    return false;

  return true;
}


//-----------------------------------------------------------------------------
// Class: OskCore::ActiveEngState
//-----------------------------------------------------------------------------
OskCore::ActiveEngState::ActiveEngState(OskCore & core_)
  : ActiveState( core_, KBID_Eng )
{
}


//-----------------------------------------------------------------------------
// Class: OskCore::ActiveCapState
//-----------------------------------------------------------------------------
OskCore::ActiveCapState::ActiveCapState(OskCore & core_)
  : ActiveState( core_, KBID_Cap )
{
}


//-----------------------------------------------------------------------------
// Class: OskCore::ActiveNumState
//-----------------------------------------------------------------------------
OskCore::ActiveNumState::ActiveNumState(OskCore & core_)
  : ActiveState( core_, KBID_Num )
{
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
