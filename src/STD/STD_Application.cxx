// Copyright (C) 2005  OPEN CASCADE, CEA/DEN, EDF R&D, PRINCIPIA R&D
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
#include "STD_Application.h"

#include "STD_MDIDesktop.h"

#include "STD_CloseDlg.h"

#include <SUIT_Tools.h>
#include <SUIT_Desktop.h>
#include <SUIT_Session.h>
#include <SUIT_ViewModel.h>
#include <SUIT_Operation.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>

#include <QtxDockAction.h>
#include <QtxActionMenuMgr.h>
#include <QtxActionToolMgr.h>
#include <QtxPopupMenu.h>

#include <qmenubar.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qfiledialog.h>
#include <qapplication.h>

#include <iostream>

/*!Create and return new instance of STD_Application*/
extern "C" STD_EXPORT SUIT_Application* createApplication()
{
  return new STD_Application();
}

/*!Constructor.*/
STD_Application::STD_Application()
: SUIT_Application(),
myEditEnabled( true ),
myActiveViewMgr( 0 )
{
  STD_MDIDesktop* desk = new STD_MDIDesktop();

  setDesktop( desk );
}

/*!Destructor.*/
STD_Application::~STD_Application()
{
}

/*! \retval QString "StdApplication"*/
QString STD_Application::applicationName() const
{
  return QString( "StdApplication" );
}

/*!Start STD_Application*/
void STD_Application::start()
{
  createActions();

  updateDesktopTitle();
  updateCommandsStatus();
  setEditEnabled( myEditEnabled );

  loadPreferences();

  SUIT_Application::start();
}

/*!
  Close the Application
*/
void STD_Application::closeApplication()
{
  if ( desktop() )
    savePreferences();

  SUIT_Application::closeApplication();
}

/*!Event on closing desktop*/
void STD_Application::onDesktopClosing( SUIT_Desktop*, QCloseEvent* e )
{
  if ( SUIT_Session::session()->applications().count() < 2 )
  {
    onExit();
    return;
  }

  if ( !isPossibleToClose() )
  {
    e->ignore();
    return;
  }

  SUIT_Study* study = activeStudy();

  if ( study )
    study->closeDocument();

  setActiveStudy( 0 );
  delete study;

  savePreferences();

  setDesktop( 0 );

  closeApplication();
}

/*!Create actions, menus and tools*/
void STD_Application::createActions()
{
  SUIT_Desktop* desk = desktop();
  SUIT_ResourceMgr* resMgr = resourceMgr();
  if ( !desk || !resMgr )
    return;

  // Create actions

  createAction( FileNewId, tr( "TOT_DESK_FILE_NEW" ),
                resMgr->loadPixmap( "STD", tr( "ICON_FILE_NEW" ) ),
                tr( "MEN_DESK_FILE_NEW" ), tr( "PRP_DESK_FILE_NEW" ),
                CTRL+Key_N, desk, false, this, SLOT( onNewDoc() ) );

  createAction( FileOpenId, tr( "TOT_DESK_FILE_OPEN" ),
                resMgr->loadPixmap( "STD", tr( "ICON_FILE_OPEN" ) ),
                tr( "MEN_DESK_FILE_OPEN" ), tr( "PRP_DESK_FILE_OPEN" ),
                CTRL+Key_O, desk, false, this, SLOT( onOpenDoc() ) );

  createAction( FileCloseId, tr( "TOT_DESK_FILE_CLOSE" ),
                resMgr->loadPixmap( "STD", tr( "ICON_FILE_CLOSE" ) ),
                tr( "MEN_DESK_FILE_CLOSE" ), tr( "PRP_DESK_FILE_CLOSE" ),
                CTRL+Key_W, desk, false, this, SLOT( onCloseDoc() ) );

  createAction( FileExitId, tr( "TOT_DESK_FILE_EXIT" ), QIconSet(),
                tr( "MEN_DESK_FILE_EXIT" ), tr( "PRP_DESK_FILE_EXIT" ),
                CTRL+Key_Q, desk, false, this, SLOT( onExit() ) );

  createAction( FileSaveId, tr( "TOT_DESK_FILE_SAVE" ),
                resMgr->loadPixmap( "STD", tr( "ICON_FILE_SAVE" ) ),
                tr( "MEN_DESK_FILE_SAVE" ), tr( "PRP_DESK_FILE_SAVE" ),
                CTRL+Key_S, desk, false, this, SLOT( onSaveDoc() ) );

  createAction( FileSaveAsId, tr( "TOT_DESK_FILE_SAVEAS" ), QIconSet(),
                tr( "MEN_DESK_FILE_SAVEAS" ), tr( "PRP_DESK_FILE_SAVEAS" ),
                CTRL+Key_A, desk, false, this, SLOT( onSaveAsDoc() ) );

  createAction( EditCopyId, tr( "TOT_DESK_EDIT_COPY" ),
                resMgr->loadPixmap( "STD", tr( "ICON_EDIT_COPY" ) ),
                tr( "MEN_DESK_EDIT_COPY" ), tr( "PRP_DESK_EDIT_COPY" ),
                CTRL+Key_C, desk, false, this, SLOT( onCopy() ) );

  createAction( EditPasteId, tr( "TOT_DESK_EDIT_PASTE" ),
                resMgr->loadPixmap( "STD", tr( "ICON_EDIT_PASTE" ) ),
                tr( "MEN_DESK_EDIT_PASTE" ), tr( "PRP_DESK_EDIT_PASTE" ),
                CTRL+Key_V, desk, false, this, SLOT( onPaste() ) );

  QAction* a = createAction( ViewStatusBarId, tr( "TOT_DESK_VIEW_STATUSBAR" ),
                             QIconSet(), tr( "MEN_DESK_VIEW_STATUSBAR" ),
                             tr( "PRP_DESK_VIEW_STATUSBAR" ), SHIFT+Key_S, desk, true );
  a->setOn( desk->statusBar()->isVisibleTo( desk ) );
  connect( a, SIGNAL( toggled( bool ) ), this, SLOT( onViewStatusBar( bool ) ) );

  createAction( NewWindowId, tr( "TOT_DESK_NEWWINDOW" ), QIconSet(),
                tr( "MEN_DESK_NEWWINDOW" ), tr( "PRP_DESK_NEWWINDOW" ), 0, desk  );

  createAction( HelpAboutId, tr( "TOT_DESK_HELP_ABOUT" ), QIconSet(),
                tr( "MEN_DESK_HELP_ABOUT" ), tr( "PRP_DESK_HELP_ABOUT" ),
                SHIFT+Key_A, desk, false, this, SLOT( onHelpAbout() ) );

  //SRN: BugID IPAL9021, add an action "Load"
  createAction( FileLoadId, tr( "TOT_DESK_FILE_LOAD" ),
                resMgr->loadPixmap( "STD", tr( "ICON_FILE_OPEN" ) ),
		tr( "MEN_DESK_FILE_LOAD" ), tr( "PRP_DESK_FILE_LOAD" ),
		CTRL+Key_L, desk, false, this, SLOT( onLoadDoc() ) );
  //SRN: BugID IPAL9021: End

  QtxDockAction* da = new QtxDockAction( tr( "TOT_DOCK_WINDOWS" ), tr( "MEN_DOCK_WINDOWS" ), desk );
  registerAction( ViewWindowsId, da );
  da->setAutoPlace( false );

  // Create menus

  int fileMenu = createMenu( tr( "MEN_DESK_FILE" ), -1, -1, 0 );
  int editMenu = createMenu( tr( "MEN_DESK_EDIT" ), -1, -1, 10 );
  int viewMenu = createMenu( tr( "MEN_DESK_VIEW" ), -1, -1, 10 );
  int helpMenu = createMenu( tr( "MEN_DESK_HELP" ), -1, -1, 1000 );

  // Create menu items

  createMenu( FileNewId, fileMenu, 0 );
  createMenu( FileOpenId, fileMenu, 0 );
  createMenu( FileLoadId, fileMenu, 0 );  //SRN: BugID IPAL9021, add a menu item "Load"
  createMenu( FileCloseId, fileMenu, 0 );
  createMenu( separator(), fileMenu, -1, 0 );
  createMenu( FileSaveId, fileMenu, 0 );
  createMenu( FileSaveAsId, fileMenu, 0 );
  createMenu( separator(), fileMenu, -1, 0 );

  createMenu( separator(), fileMenu );
  createMenu( FileExitId, fileMenu );

  createMenu( EditCopyId, editMenu );
  createMenu( EditPasteId, editMenu );
  createMenu( separator(), editMenu );

  createMenu( ViewWindowsId, viewMenu );
  createMenu( ViewStatusBarId, viewMenu );
  createMenu( separator(), viewMenu );

  createMenu( HelpAboutId, helpMenu );
  createMenu( separator(), helpMenu );

  // Create tool bars

  int stdTBar = createTool( tr( "INF_DESK_TOOLBAR_STANDARD" ) );

  // Create tool items

  createTool( FileNewId, stdTBar );
  createTool( FileOpenId, stdTBar );
  createTool( FileSaveId, stdTBar );
  createTool( FileCloseId, stdTBar );
  createTool( separator(), stdTBar );
  createTool( EditCopyId, stdTBar );
  createTool( EditPasteId, stdTBar );
}

/*!Opens new application*/
void STD_Application::onNewDoc()
{
  QApplication::setOverrideCursor( Qt::waitCursor );

  if ( !activeStudy() )
  {
    createEmptyStudy();
    activeStudy()->createDocument();
    studyCreated( activeStudy() );
  }
  else
  {
    SUIT_Application* aApp = startApplication( 0, 0 );
    if ( aApp->inherits( "STD_Application" ) )
      ((STD_Application*)aApp)->onNewDoc();
    else
    {
      aApp->createEmptyStudy();
      aApp->activeStudy()->createDocument();
    }
  }

  QApplication::restoreOverrideCursor();
}

/*!Put file name from file dialog to onOpenDoc(const QString&) function*/
void STD_Application::onOpenDoc()
{
  // It is preferrable to use OS-specific file dialog box here !!!
  QString aName = getFileName( true, QString::null, getFileFilter(), QString::null, 0 );
  if ( aName.isNull() )
    return;

  onOpenDoc( aName );
}

/*! \retval true, if document was opened successful, else false.*/
bool STD_Application::onOpenDoc( const QString& aName )
{
  QApplication::setOverrideCursor( Qt::waitCursor );

  bool res = true;
  if ( !activeStudy() )
  {
    // if no study - open in current desktop
    res = useFile( aName );
  }
  else
  {
    // if study exists - open in new desktop. Check: is the same file is opened?
    SUIT_Session* aSession = SUIT_Session::session();
    QPtrList<SUIT_Application> aAppList = aSession->applications();
    bool isAlreadyOpen = false;
    SUIT_Application* aApp = 0;
    for ( QPtrListIterator<SUIT_Application> it( aAppList ); it.current() && !isAlreadyOpen; ++it )
    {
      aApp = it.current();
      if ( aApp->activeStudy()->studyName() == aName )
        isAlreadyOpen = true;
    }
    if ( !isAlreadyOpen )
    {
      aApp = startApplication( 0, 0 );
      if ( aApp )
        res = aApp->useFile( aName );
      if ( !res )
        aApp->closeApplication();
    }
    else
      aApp->desktop()->setActiveWindow();
  }

  QApplication::restoreOverrideCursor();

  return res;
}

/*! called on loading the existent study */
void STD_Application::onLoadDoc()
{
}

/*! \retval true, if document was loaded successful, else false.*/
bool STD_Application::onLoadDoc( const QString& aName )
{
  bool res = true;
  if ( !activeStudy() )
  {
    // if no study - load in current desktop
    res = useStudy( aName );
  }
  else
  {
    // if study exists - load in new desktop. Check: is the same file is loaded?
    SUIT_Session* aSession = SUIT_Session::session();
    QPtrList<SUIT_Application> aAppList = aSession->applications();
    bool isAlreadyOpen = false;
    SUIT_Application* aApp = 0;
    for ( QPtrListIterator<SUIT_Application> it( aAppList ); it.current() && !isAlreadyOpen; ++it )
    {
      aApp = it.current();
      if ( aApp->activeStudy()->studyName() == aName )
        isAlreadyOpen = true;
    }
    if ( !isAlreadyOpen )
    {
      aApp = startApplication( 0, 0 );
      if ( aApp )
        res = aApp->useStudy( aName );
    }
    else
      aApp->desktop()->setActiveWindow();
  }
  return res;
}

/*!Virtual function. Not implemented here.*/
void STD_Application::beforeCloseDoc( SUIT_Study* )
{
}

/*!Virtual function. Not implemented here.*/
void STD_Application::afterCloseDoc()
{
}

/*!Close document, if it's possible.*/
void STD_Application::onCloseDoc( bool ask )
{
  if ( ask && !isPossibleToClose() )
    return;

  SUIT_Study* study = activeStudy();

  beforeCloseDoc( study );

  if ( study )
    study->closeDocument(myClosePermanently);

  clearViewManagers();

  setActiveStudy( 0 );
  delete study;

  int aNbStudies = 0;
  QPtrList<SUIT_Application> apps = SUIT_Session::session()->applications();
  for ( unsigned i = 0; i < apps.count(); i++ )
    aNbStudies += apps.at( i )->getNbStudies();

  // STV: aNbStudies - number of currently existing studies (exclude currently closed)
  // STV: aNbStudies should be compared with 0.
  if ( aNbStudies )
  {
    savePreferences();
    setDesktop( 0 );
  }
  else
  {
    updateDesktopTitle();
    updateCommandsStatus();
  }

  afterCloseDoc();

  if ( !desktop() )
    closeApplication();
}

/*!Check the application on closing.
 * \retval true if possible, else false
 */
bool STD_Application::isPossibleToClose()
{
  myClosePermanently = true; //SRN: BugID: IPAL9021
  if ( activeStudy() )
  {
    activeStudy()->abortAllOperations();
    if ( activeStudy()->isModified() )
    {
      QString sName = activeStudy()->studyName().stripWhiteSpace();
      QString msg = sName.isEmpty() ? tr( "INF_DOC_MODIFIED" ) : tr ( "INF_DOCUMENT_MODIFIED" ).arg( sName );

      //SRN: BugID: IPAL9021: Begin
      STD_CloseDlg dlg(desktop());
      switch( dlg.exec() )
      {
      case 1:
        if ( activeStudy()->isSaved() )
          onSaveDoc();
        else if ( !onSaveAsDoc() )
          return false;
        break;
      case 2:
        break;
      case 3:
	      myClosePermanently = false;
        break;
      case 4:
      default:
        return false;
      }
     //SRN: BugID: IPAL9021: End
    }
  }
  return true;
}

/*!Save document if all ok, else error message.*/
void STD_Application::onSaveDoc()
{
  if ( !activeStudy() )
    return;

  bool isOk = false;
  if ( activeStudy()->isSaved() )
  {
    putInfo( tr( "INF_DOC_SAVING" ) + activeStudy()->studyName() );

    QApplication::setOverrideCursor( Qt::waitCursor );

    isOk = activeStudy()->saveDocument();

    QApplication::restoreOverrideCursor();

    if ( !isOk )
    {
      putInfo( "" );
      SUIT_MessageBox::error1( desktop(), tr( "TIT_FILE_SAVEAS" ),
			                         tr( "MSG_CANT_SAVE" ).arg( activeStudy()->studyName() ), tr( "BUT_OK" ) );
    }
    else
      putInfo( tr( "INF_DOC_SAVED" ).arg( "" ) );
  }

  if ( isOk )
    studySaved( activeStudy() );
  else
    onSaveAsDoc();
}

/*! \retval TRUE, if doument saved successful, else FALSE.*/
bool STD_Application::onSaveAsDoc()
{
  SUIT_Study* study = activeStudy();
  if ( !study )
    return false;

  bool isOk = false;
  while ( !isOk )
  {
    QString aName = getFileName( false, study->studyName(), getFileFilter(), QString::null, 0 );
    if ( aName.isNull() )
      return false;

    QApplication::setOverrideCursor( Qt::waitCursor );

    putInfo( tr( "INF_DOC_SAVING" ) + aName );
    isOk = study->saveDocumentAs( aName );

    putInfo( isOk ? tr( "INF_DOC_SAVED" ).arg( aName ) : "" );

    QApplication::restoreOverrideCursor();

    if ( !isOk )
      SUIT_MessageBox::error1( desktop(), tr( "ERROR" ),
                             tr( "INF_DOC_SAVING_FAILS" ).arg( aName ), tr( "BUT_OK" ) );
  }

  studySaved( activeStudy() );

  return isOk;
}

/*!Closing session.*/
void STD_Application::onExit()
{
  int aAnswer = SUIT_MessageBox::info2( desktop(), tr( "INF_DESK_EXIT" ), tr( "QUE_DESK_EXIT" ),
                                        tr( "BUT_OK" ), tr( "BUT_CANCEL" ), 1, 2, 2 );
  if ( aAnswer == 1 )
    SUIT_Session::session()->closeSession();
}

/*!Virtual slot. Not implemented here.*/
void STD_Application::onCopy()
{
}

/*!Virtual slot. Not implemented here.*/
void STD_Application::onPaste()
{
}

/*!Sets \a theEnable for menu manager and for tool manager.*/
void STD_Application::setEditEnabled( bool theEnable )
{
  myEditEnabled = theEnable;

  QtxActionMenuMgr* mMgr = desktop()->menuMgr();
  QtxActionToolMgr* tMgr = desktop()->toolMgr();

  for ( int i = EditCopyId; i <= EditPasteId; i++ )
  {
    mMgr->setShown( i, myEditEnabled );
    tMgr->setShown( i, myEditEnabled );
  }
}

/*!\retval true, if document opened successful, else false.*/
bool STD_Application::useFile(const QString& theFileName)
{
  bool res = SUIT_Application::useFile( theFileName );

  if ( res )
    studyOpened( activeStudy() );

  return res;
}

/*!Update desktop title.*/
void STD_Application::updateDesktopTitle()
{
  QString aTitle = applicationName();
  QString aVer = applicationVersion();
  if ( !aVer.isEmpty() )
    aTitle += QString( " " ) + aVer;

  if ( activeStudy() )
  {
    QString sName = SUIT_Tools::file( activeStudy()->studyName().stripWhiteSpace(), false );
    if ( !sName.isEmpty() )
      aTitle += QString( " - [%1]" ).arg( sName );
  }

  desktop()->setCaption( aTitle );
}

/*!Update commands status.*/
void STD_Application::updateCommandsStatus()
{
  bool aHasStudy = activeStudy() != 0;
  bool aIsNeedToSave = false;
  if ( aHasStudy )
    aIsNeedToSave = !activeStudy()->isSaved() || activeStudy()->isModified();

  if ( action( FileSaveId ) )
    action( FileSaveId )->setEnabled( aIsNeedToSave );
  if ( action( FileSaveAsId ) )
    action( FileSaveAsId )->setEnabled( aHasStudy );
  if ( action( FileCloseId ) )
    action( FileCloseId )->setEnabled( aHasStudy );
  if ( action( NewWindowId ) )
    action( NewWindowId )->setEnabled( aHasStudy );
}

/*!\retval SUIT_ViewManager by viewer manager type name.*/
SUIT_ViewManager* STD_Application::viewManager( const QString& vmType ) const
{
  SUIT_ViewManager* vm = 0;
  for ( QPtrListIterator<SUIT_ViewManager> it( myViewMgrs ); it.current() && !vm; ++it )
  {
    if ( it.current()->getType() == vmType )
      vm = it.current();
  }
  return vm;
}

/*! \param vmType - input view manager type name
 * \param lst - output list of view managers with types \a vmType.
 */
void STD_Application::viewManagers( const QString& vmType, ViewManagerList& lst ) const
{
  for ( QPtrListIterator<SUIT_ViewManager> it( myViewMgrs ); it.current(); ++it )
    if ( it.current()->getType() == vmType )
      lst.append( it.current() );
}

/*!\param lst - output list of all view managers.*/
void STD_Application::viewManagers( ViewManagerList& lst ) const
{
  for ( QPtrListIterator<SUIT_ViewManager> it( myViewMgrs ); it.current(); ++it )
    lst.append( it.current() );
}

/*!\retval ViewManagerList - const list of all view managers.*/
ViewManagerList STD_Application::viewManagers() const
{
  ViewManagerList lst;
  viewManagers( lst );
  return lst;
}

/*!\retval SUIT_ViewManager - return pointer to active view manager.*/
SUIT_ViewManager* STD_Application::activeViewManager() const
{
  return myActiveViewMgr;
}

/*!Add view manager to view managers list, if it not already there.*/
void STD_Application::addViewManager( SUIT_ViewManager* vm )
{
  if ( !vm )
    return;

  if ( !containsViewManager( vm ) )
  {
    myViewMgrs.append( vm );
    connect( vm, SIGNAL( activated( SUIT_ViewManager* ) ),
             this, SLOT( onViewManagerActivated( SUIT_ViewManager* ) ) );
    vm->connectPopupRequest( this, SLOT( onConnectPopupRequest( SUIT_PopupClient*, QContextMenuEvent* ) ) );

    emit viewManagerAdded( vm );
  }
/*
  if ( !activeViewManager() && myViewMgrs.count() == 1 )
    setActiveViewManager( vm );
*/
}

/*!Remove view manager from view managers list.*/
void STD_Application::removeViewManager( SUIT_ViewManager* vm )
{
  if ( !vm )
    return;

  vm->closeAllViews();

  emit viewManagerRemoved( vm );

  vm->disconnectPopupRequest( this, SLOT( onConnectPopupRequest( SUIT_PopupClient*, QContextMenuEvent* ) ) );
  vm->disconnect();
  myViewMgrs.removeRef( vm );

  if ( myActiveViewMgr == vm )
    myActiveViewMgr = 0;
}

/*!Remove all view managers from view managers list.*/
void STD_Application::clearViewManagers()
{
  ViewManagerList lst;
  viewManagers( lst );

  for ( QPtrListIterator<SUIT_ViewManager> it( lst ); it.current(); ++it )
    removeViewManager( it.current() );
}

/*!\retval TRUE, if view manager \a vm, already in view manager list (\a myViewMgrs).*/
bool STD_Application::containsViewManager( SUIT_ViewManager* vm ) const
{
  return myViewMgrs.contains( vm ) > 0;
}

/*!Private slot, sets active manager to \vm, if \vm in view managers list.*/
void STD_Application::onViewManagerActivated( SUIT_ViewManager* vm )
{
  setActiveViewManager( vm );
}

/*!Sets status bar show, if \on = true, else status bar hide.*/
void STD_Application::onViewStatusBar( bool on )
{
  if ( on )
    desktop()->statusBar()->show();
  else
    desktop()->statusBar()->hide();
}

/*!Call SUIT_MessageBox::info1(...) with about information.*/
void STD_Application::onHelpAbout()
{
  SUIT_MessageBox::info1( desktop(), tr( "About" ), tr( "ABOUT_INFO" ), "&OK" );
}

/*!Create empty study. \n
 * Create new view manager and adding it to view managers list.
 */
void STD_Application::createEmptyStudy()
{
  SUIT_Application::createEmptyStudy();

  SUIT_ViewManager* vm = new SUIT_ViewManager( activeStudy(), desktop(), new SUIT_ViewModel() );

  addViewManager( vm );
}

/*!Sets active manager to \vm, if \vm in view managers list.*/
void STD_Application::setActiveViewManager( SUIT_ViewManager* vm )
{
  if ( !containsViewManager( vm ) )
    return;

  myActiveViewMgr = vm;
  emit viewManagerActivated( vm );
}

/*!Public slot. */
void STD_Application::onConnectPopupRequest( SUIT_PopupClient* client, QContextMenuEvent* e )
{
  QtxPopupMenu* popup = new QtxPopupMenu();
  // fill popup by own items
  QString title;
  contextMenuPopup( client->popupClientType(), popup, title );
  popup->setTitleText( title );

  popup->insertSeparator();
  // add items from popup client
  client->contextMenuPopup( popup );

  SUIT_Tools::simplifySeparators( popup );

  if ( popup->count() )
    popup->exec( e->globalPos() );
  delete popup;
}

#include <qregexp.h>

/*!\retval QString - return file name from dialog.*/
QString STD_Application::getFileName( bool open, const QString& initial, const QString& filters,
				      const QString& caption, QWidget* parent )
{
  if ( !parent )
    parent = desktop();
  if ( open )
  {
    return QFileDialog::getOpenFileName( initial, filters, parent, 0, caption );
  }
  else
  {
    QString aName;
    QString aUsedFilter;
    QString anOldPath = initial;

    bool isOk = false;
    while ( !isOk )
    {
      // It is preferrable to use OS-specific file dialog box here !!!
      aName = QFileDialog::getSaveFileName( anOldPath, filters, parent, 0, caption, &aUsedFilter );

      if ( aName.isNull() )
        isOk = true;
      else
      {
	      int aEnd = aUsedFilter.findRev( ')' );
	      int aStart = aUsedFilter.findRev( '(', aEnd );
	      QString wcStr = aUsedFilter.mid( aStart + 1, aEnd - aStart - 1 );

        int idx = 0;
        QStringList extList;
        QRegExp rx( "[\b\\*]*\\.([\\w]+)" );
        while ( ( idx = rx.search( wcStr, idx ) ) != -1 )
        {
          extList.append( rx.cap( 1 ) );
          idx += rx.matchedLength();
        }

        if ( !extList.isEmpty() && !extList.contains( QFileInfo( aName ).extension() ) )
          aName += QString( ".%1" ).arg( extList.first() );

	      if ( QFileInfo( aName ).exists() )
        {
	        int aAnswer = SUIT_MessageBox::warn3( desktop(), tr( "TIT_FILE_SAVEAS" ),
					                                      tr( "MSG_FILE_EXISTS" ).arg( aName ),
					                                      tr( "BUT_YES" ), tr( "BUT_NO" ), tr( "BUT_CANCEL" ), 1, 2, 3, 1 );
	        if ( aAnswer == 3 )
          {     // cancelled
            aName = QString::null;
	          isOk = true;
          }
	        else if ( aAnswer == 2 ) // not save to this file
	          anOldPath = aName;             // not to return to the same initial dir at each "while" step
	        else                     // overwrite the existing file
	          isOk = true;
        }
	      else
	        isOk = true;
      }
    }
    return aName;
  }
}

/*!\retval QString - return directory name from dialog.*/
QString STD_Application::getDirectory( const QString& initial, const QString& caption, QWidget* parent )
{
  if ( !parent )
    parent = desktop();
  return QFileDialog::getExistingDirectory( initial, parent, 0, caption, true );
}

void STD_Application::setDesktop( SUIT_Desktop* desk )
{
  SUIT_Desktop* prev = desktop();

  SUIT_Application::setDesktop( desk );

  if ( prev != desk && desk )
    connect( desk, SIGNAL( closing( SUIT_Desktop*, QCloseEvent* ) ),
             this, SLOT( onDesktopClosing( SUIT_Desktop*, QCloseEvent* ) ) );
}

/*!
  Allow to load preferences before the desktop will be shown.
*/
void STD_Application::loadPreferences()
{
}

/*!
  Allow to save preferences before the application will be closed.
*/
void STD_Application::savePreferences()
{
}

void STD_Application::studyCreated( SUIT_Study* )
{
  updateDesktopTitle();
  updateCommandsStatus();
}

void STD_Application::studyOpened( SUIT_Study* )
{
  updateDesktopTitle();
  updateCommandsStatus();
}

void STD_Application::studySaved( SUIT_Study* )
{
  updateDesktopTitle();
  updateCommandsStatus();
}
