// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File:      SalomeApp_Application.h
// Created:   10/22/2004 3:37:25 PM
// Author:    Sergey LITONIN
//
#ifndef SALOMEAPP_APPLICATION_H
#define SALOMEAPP_APPLICATION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SalomeApp.h"
#include <LightApp_Application.h>

#include <SUIT_DataObject.h>

#include <CORBA.h>

//#include <SALOMEconfig.h>
//#include CORBA_CLIENT_HEADER(SALOMEDS)
#include <SALOME_NamingService.hxx>

#include "SALOMEDSClient.hxx"

class LightApp_Preferences;
class SalomeApp_Study;
class SalomeApp_NoteBookDlg;
class SUIT_Desktop;

class SUIT_ViewModel;
class SALOME_LifeCycleCORBA;


#ifdef WIN32
#pragma warning( disable:4251 )
#endif

/*!
  \class SalomeApp_Application
  \brief Application containing SalomeApp module or LightApp module
*/

class SALOMEAPP_EXPORT SalomeApp_Application : public LightApp_Application
{
  Q_OBJECT

public:
  enum { MenuToolsId = 5 };
  enum { DumpStudyId = LightApp_Application::UserID, LoadScriptId, PropertiesId,
         CatalogGenId, RegDisplayId, SaveGUIStateId, FileLoadId, NoteBookId, UserID };

protected:
  enum { OpenRefresh = LightApp_Application::OpenReload + 1 };
  enum { CloseUnload = LightApp_Application::CloseDiscard + 1 };
  enum { LoadStudyId = LightApp_Application::OpenStudyId + 1, NewAndScriptId };

public:
  SalomeApp_Application();
  virtual ~SalomeApp_Application();

  virtual void                        updateObjectBrowser( const bool = true );

  virtual QString                     getFileFilter() const;

  virtual void                        start();

  virtual void                        contextMenuPopup( const QString&, QMenu*, QString& );

  virtual bool                        checkDataObject(LightApp_DataObject* theObj);

  virtual void                        setDesktop( SUIT_Desktop* );

  static CORBA::ORB_var               orb();
  static SALOMEDSClient_StudyManager* studyMgr();
  static SALOME_NamingService*        namingService();
  static SALOME_LifeCycleCORBA*       lcc();

  SUIT_ViewManager*                   newViewManager(const QString&);
  void                                updateSavePointDataObjects( SalomeApp_Study* );

  virtual bool                        isPossibleToClose( bool& );

  virtual bool                        useStudy( const QString& );
  virtual void                        updateDesktopTitle();
  
  virtual void                        setNoteBook(SalomeApp_NoteBookDlg* theNoteBook);
  virtual SalomeApp_NoteBookDlg*      getNoteBook() const;

 //! update visibility state of objects
  void                                updateVisibilityState( DataObjectList& theList,
                                                             SUIT_ViewModel* theViewModel );  

  virtual bool                        renameAllowed( const QString& ) const;
  virtual bool                        renameObject( const QString&, const QString& );
  
public slots:
  virtual void                        onLoadDoc();
  virtual void                        onNewWithScript();
  virtual bool                        onLoadDoc( const QString& );
  virtual void                        onCloseDoc( bool ask = true);

  virtual void                        onExit();
  virtual void                        onCopy();
  virtual void                        onPaste();
  void                                onSaveGUIState();// called from VISU

protected slots:
  void                                onStudyCreated( SUIT_Study* );
  void                                onStudySaved( SUIT_Study* );
  void                                onStudyOpened( SUIT_Study* );
  void                                onStudyClosed( SUIT_Study* );
  
  void                                onViewManagerRemoved( SUIT_ViewManager* );

protected:
  virtual void                        createActions();
  virtual SUIT_Study*                 createNewStudy();
  virtual QWidget*                    createWindow( const int );

  virtual void                        updateCommandsStatus();
  virtual void                        onSelectionChanged();

  virtual void                        createPreferences( LightApp_Preferences* );

  virtual bool                        closeAction( const int, bool& );
  virtual int                         closeChoice( const QString& );

  virtual int                         openChoice( const QString& );
  virtual bool                        openAction( const int, const QString& );

  virtual QMap<int, QString>          activateModuleActions() const;
  virtual void                        moduleActionSelected( const int );

  void                                objectBrowserColumnsVisibility();

private slots:
  void                                onDeleteInvalidReferences();
  void                                onDblClick( SUIT_DataObject* );
  void                                onProperties();
  void                                onDumpStudy();
  void                                onNoteBook();
  void                                onLoadScript();

  void                                onDeleteGUIState();
  void                                onRestoreGUIState();

  void                                onCatalogGen();
  void                                onRegDisplay();
  void                                onOpenWith();
  void                                onExtAction();

  void                                onWindowActivated( SUIT_ViewWindow* theViewWindow );

private:
  void                                createExtraActions();

private:
  SalomeApp_NoteBookDlg*              myNoteBook;
  QMap<QString, QAction*>             myExtActions; // Map <AttributeUserID, QAction>
};

#ifdef WIN32
#pragma warning( default:4251 )
#endif

#endif
