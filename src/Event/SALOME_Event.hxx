//  KERNEL SALOME_Event : Define event posting mechanism
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SALOME_Event.hxx
//  Author : Sergey ANIKIN
//  Module : KERNEL
//  $Header$


#ifndef SALOME_Event_HeaderFile
#define SALOME_Event_HeaderFile

#include <qevent.h>

#define SALOME_EVENT QEvent::Type( QEvent::User + 10000 )

class QSemaphore;

/*!
 *  \class SALOME_Event
 *  Description: 
 *  This class encapsulates data and functionality required for 
 *  posting component-specific events to perform arbitrary operations in main GUI thread. 
 *  SALOME_Event objects can be posted by any thread belonging to the GUI process.
 *
 *  It is necessary to derive a custom event class from SALOME_Event and 
 *  re-implement virtual Execute() method. This method should actually perform 
 *  the desirable operation. To pass all the required data to Execute() and store a return value,
 *  arbitrary data fields can be added to the custom event class. There is 
 *  no need to protect such fields with a mutex, for only one thread working with
 *  a SALOME_Event object is active at any moment.
 *
 *  Usage:
 *  - create SALOME_Event. 
 *    Components can derive their own event class from SALOME_Event
 *    in order to pass custom data to the event handler.
 *  - call process() method to post the event. After process() execution
 *    it is possible to examine fields of your custom event object.
 *  - perform delete operator on the event to wake up the desktop (you can also set <autoRelease>
 *    parameter to TRUE to automatically wake up desktop after process()
 * 
 *  processed() method is used by the desktop to signal that event processing 
 *  has been completed.
 *  
 *  Caveats: 
 *    There is no.
 */

#include <Event.h>

class EVENT_EXPORT SALOME_Event{
public:
  SALOME_Event();
  virtual ~SALOME_Event();

  // To do real work
  virtual void Execute() = 0;

  static bool IsSessionThread();
  void process();

protected:
  void processed();
  friend class SalomeApp_EventFilter;

  static void GetSessionThread();
  friend int main(int, char **);

private:
  QSemaphore* mySemaphore;
};


/*!
  \class TMemFunEvent
  \brief Template class for member function
*/
template<class TObject, typename TRes> class /*EVENT_EXPORT */TMemFunEvent: public SALOME_Event{
public:
  typedef TRes TResult;
  TResult myResult;
  typedef TResult (TObject::* TAction)();
  TMemFunEvent(TObject* theObject, TAction theAction, 
	       TResult theResult = TResult()):
    myObject(theObject),
    myAction(theAction),
    myResult(theResult)
  {}
  virtual void Execute(){
    myResult = (myObject->*myAction)();
  }
private:
  TObject* myObject;
  TAction myAction;
};


/*!
  \class TVoidMemFunEvent
  \brief Template class for member function
*/
template<class TObject> class /*EVENT_EXPORT */TVoidMemFunEvent: public SALOME_Event{
public:
  typedef void (TObject::* TAction)();
  TVoidMemFunEvent(TObject* theObject, TAction theAction):
    myObject(theObject),
    myAction(theAction)
  {}
  virtual void Execute(){
    (myObject->*myAction)();
  }
private:
  TObject* myObject;
  TAction myAction;
};


/*!
  \class TMemFun1ArgEvent
  \brief Template for member function with one argument
*/
template<class TObject, typename TRes, typename TArg, typename TStoreArg = TArg> class/* EVENT_EXPORT */TMemFun1ArgEvent:
public SALOME_Event{
public:
  typedef TRes TResult;
  TResult myResult;
  typedef TResult (TObject::* TAction)(TArg);
  TMemFun1ArgEvent(TObject* theObject, TAction theAction, TArg theArg, 
		   TResult theResult = TResult()):
    myObject(theObject),
    myAction(theAction),
    myResult(theResult),
    myArg(theArg)
  {}
  virtual void Execute(){
    myResult = (myObject->*myAction)(myArg);
  }
private:
  TObject* myObject;
  TAction myAction;
  TStoreArg myArg;
};


/*!
  \class TVoidMemFun1ArgEvent
  \brief Template for member function with one argument
*/
template<class TObject, typename TArg, typename TStoreArg = TArg> class /*EVENT_EXPORT */TVoidMemFun1ArgEvent: public SALOME_Event{
public:
  typedef void (TObject::* TAction)(TArg);
  TVoidMemFun1ArgEvent(TObject* theObject, TAction theAction, TArg theArg):
    myObject(theObject),
    myAction(theAction),
    myArg(theArg)
  {}
  virtual void Execute(){
    (myObject->*myAction)(myArg);
  }
private:
  TObject* myObject;
  TAction myAction;
  TStoreArg myArg;
};


/*!
  \class TMemFun2ArgEvent
  \brief Template for member function with two arguments
*/
template<class TObject, typename TRes, typename TArg, typename TArg1, typename TStoreArg = TArg, typename TStoreArg1 = TArg1> class
/*EVENT_EXPORT */TMemFun2ArgEvent: public SALOME_Event{
public:
  typedef TRes TResult;
  TResult myResult;
  typedef TResult (TObject::* TAction)(TArg,TArg1);
  TMemFun2ArgEvent(TObject* theObject, TAction theAction, 
		   TArg theArg, TArg1 theArg1,
		   TResult theResult = TResult()):
    myObject(theObject),
    myAction(theAction),
    myResult(theResult),
    myArg(theArg),
    myArg1(theArg1)
  {}
  virtual void Execute(){
    myResult = (myObject->*myAction)(myArg,myArg1);
  }
private:
  TObject* myObject;
  TAction myAction;
  TStoreArg myArg;
  TStoreArg1 myArg1;
};


/*!
  \class TVoidMemFun2ArgEvent
  \brief Template for member function with two arguments
*/
template<class TObject, typename TArg, typename TArg1, typename TStoreArg = TArg, typename TStoreArg1 = TArg1> class
/*EVENT_EXPORT*/ TVoidMemFun2ArgEvent: public SALOME_Event{
public:
  typedef void (TObject::* TAction)(TArg,TArg1);
  TVoidMemFun2ArgEvent(TObject* theObject, TAction theAction, TArg theArg, TArg1 theArg1):
    myObject(theObject),
    myAction(theAction),
    myArg(theArg),
    myArg1(theArg1)
  {}
  virtual void Execute(){
    (myObject->*myAction)(myArg,myArg1);
  }
private:
  TObject* myObject;
  TAction myAction;
  TStoreArg myArg;
  TStoreArg1 myArg1;
};


/*!
  \fn ProcessEvent
  \brief Template function for processing events with result returing
*/
template<class TEvent> inline typename TEvent::TResult ProcessEvent(TEvent* theEvent){
  typename TEvent::TResult aResult;
  if(SALOME_Event::IsSessionThread()){
    theEvent->Execute();
    aResult = theEvent->myResult;
  }else{
    theEvent->process();
    aResult = theEvent->myResult;
  }
  delete theEvent;
  return aResult;
}


/*!
  \fn ProcessEvent
  \brief Template function for processing events without result
*/
inline void ProcessVoidEvent(SALOME_Event* theEvent){
  if(SALOME_Event::IsSessionThread()){
    theEvent->Execute();
  }else{
    theEvent->process();
  }
  delete theEvent;
}


#endif
