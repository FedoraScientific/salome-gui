//  LIGHT LightApp
//
//  Copyright (C) 2005  CEA/DEN, EDF R&D
//
//
//
//  File   : LightApp_Operation.h
//  Author : Sergey LITONIN
//  Module : LIGHT


#ifndef LightApp_Operation_H
#define LightApp_Operation_H

#include "LightApp.h"
#include <SUIT_Operation.h>

class LightApp_Module;
class LightApp_Application;
class LightApp_Operation;
class LightApp_SelectionMgr;
class LightApp_Dialog;
class SUIT_Desktop;

/*!
  \class LightApp_Operation
  \brief Base class for all operations
  Base class for all operations (see SUIT_Operation for more description)
*/
class LIGHTAPP_EXPORT LightApp_Operation : public SUIT_Operation
{
  Q_OBJECT

public:
  LightApp_Operation();
  virtual ~LightApp_Operation();

  virtual void              setModule( LightApp_Module* );
  LightApp_Module*          module() const;

  bool                      isAutoResumed() const;

  virtual LightApp_Dialog* dlg() const;

protected:

  // Methods redefined from base class

  virtual void              startOperation();
  virtual void              suspendOperation();
  virtual void              resumeOperation();
  virtual void              abortOperation();
  virtual void              commitOperation();

  // Additional virtual methods may be redefined by derived classes
  
  virtual void              setDialogActive( const bool );
  virtual void              activateSelection();
  virtual void              selectionDone();


  // Axiluary methods
  
  SUIT_Desktop*             desktop() const;
  SUIT_Operation*           activeOperation() const;
  LightApp_SelectionMgr*    selectionMgr() const;
  void                      update( const int );
  void                      setAutoResumed( const bool );
      
private slots:

  virtual void              onSelectionDone();

private:

  LightApp_Module*          myModule;         
  bool                      myIsAutoResumed;
};

#endif






