/*-----------------------------------------------------------------------------
 * On-Screen Keyboard 2 for uClinux on PSP
 * Created by Jackson Mo, Jan 2, 2008
 *---------------------------------------------------------------------------*/
#ifndef OSK_STATES_H
#error "This header is embedded inside osk.h. DO NOT use it directly."
#endif


//-----------------------------------------------------------------------------
// Class: OskCore::BaseState
//-----------------------------------------------------------------------------
class BaseState
{
public:
  BaseState(OskCore & core_);
  virtual ~BaseState() { }

  virtual BaseState * enterState();
  virtual void exitState();
  virtual BaseState * processKeys();
  virtual bool IsTerminated() { return false; }

protected:
  virtual bool draw() = 0;

  OskCore & m_core;

private:
  // Not implemented
  BaseState();
  BaseState(const BaseState &);
  BaseState & operator = (const BaseState &);
};


//-----------------------------------------------------------------------------
// Class: OskCore::FailedState
//-----------------------------------------------------------------------------
class FailedState : public BaseState
{
public:
  FailedState(OskCore & core_);

  virtual BaseState * enterState();
  virtual BaseState * processKeys();
  virtual bool IsTerminated() { return true; }

protected:
  virtual bool draw()
  {
    return false;
  }

private:
  // Not implemented
  FailedState();
  FailedState(const FailedState &);
  FailedState & operator = (const FailedState &);
};


//-----------------------------------------------------------------------------
// Class: OskCore::MouseState
//-----------------------------------------------------------------------------
class MouseState : public BaseState
{
public:
  MouseState(OskCore & core_);

  virtual BaseState * enterState();
  virtual BaseState * processKeys();

protected:
  virtual bool draw();

private:
  // Not implemented
  MouseState();
  MouseState(const MouseState &);
  MouseState & operator = (const MouseState &);
};


//-----------------------------------------------------------------------------
// Class: OskCore::KbdState
//-----------------------------------------------------------------------------
class KbdState : public BaseState
{
public:
  KbdState(OskCore & core_);

  virtual BaseState * enterState();
  virtual BaseState * processKeys();

protected:
  BaseState * processKeysFinal(OskKeyboardId kbdId_, OskKeySectionId secId_);
  bool update(OskKeySectionId activeSection_);

private:
  // Not implemented
  KbdState();
  KbdState(const KbdState &);
  KbdState & operator = (const KbdState &);
};


//-----------------------------------------------------------------------------
// Class: OskCore::IdleState
//-----------------------------------------------------------------------------
class IdleState : public KbdState
{
public:
  IdleState(OskCore & core_);

  virtual BaseState * enterState();
  virtual BaseState * processKeys();

protected:
  virtual bool draw();

private:
  // Not implemented
  IdleState();
  IdleState(const IdleState &);
  IdleState & operator = (const IdleState &);
};

   
//-----------------------------------------------------------------------------
// Class: OskCore::ActiveState
//-----------------------------------------------------------------------------
class ActiveState : public KbdState
{
public:
  ActiveState
  (
    OskCore & core_,
    OskKeyboardId kbdId_
  );

  virtual BaseState * enterState();
  virtual BaseState * processKeys();

protected:
  bool update(OskKeySectionId activeSection_);
  virtual bool draw();

  const OskKeyboardId c_kbdId;
  OskKeySectionId m_activeSection;
  static const OskImage::ImageId c_kbdImg[ KBID_Count ];
  static const OskImage::ImageId c_kbdImgActive[ KBID_Count ];

private:
  // Not implemented
  ActiveState();
  ActiveState(const ActiveState &);
  ActiveState & operator = (const ActiveState &);
};


//-----------------------------------------------------------------------------
// Class: OskCore::ActiveEngState
//-----------------------------------------------------------------------------
class ActiveEngState : public ActiveState
{
public:
  ActiveEngState(OskCore & core_);

protected:
private:
  // Not implemented
  ActiveEngState();
  ActiveEngState(const ActiveEngState &);
  ActiveEngState & operator = (const ActiveEngState &);
};


//-----------------------------------------------------------------------------
// Class: OskCore::ActiveCapState
//-----------------------------------------------------------------------------
class ActiveCapState : public ActiveState
{
public:
  ActiveCapState(OskCore & core_);

protected:
private:
  // Not implemented
  ActiveCapState();
  ActiveCapState(const ActiveCapState &);
  ActiveCapState & operator = (const ActiveCapState &);
};


//-----------------------------------------------------------------------------
// Class: OskCore::ActiveNumState
//-----------------------------------------------------------------------------
class ActiveNumState : public ActiveState
{
public:
  ActiveNumState(OskCore & core_);

protected:
private:
  // Not implemented
  ActiveNumState();
  ActiveNumState(const ActiveNumState &);
  ActiveNumState & operator = (const ActiveNumState &);
};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
