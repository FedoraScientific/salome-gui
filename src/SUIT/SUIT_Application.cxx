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
#include "SUIT_Application.h"

#include "SUIT_Session.h"
#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"

#include <qlabel.h>
#include <qtimer.h>
#include <qstatusbar.h>

#include <QtxAction.h>
#include <QtxActionMenuMgr.h>
#include <QtxActionToolMgr.h>

/*!
  Default constructor
*/
SUIT_Application::SUIT_Application()
: QObject( 0 ),
myStudy( 0 ),
myDesktop( 0 ),
myStatusLabel( 0 )
{ 
}

/*!
  Destructor
*/
SUIT_Application::~SUIT_Application() 
{
  delete myStudy;
  myStudy = 0;

  setDesktop( 0 );
}

/*!
  \return main window of application (desktop)
*/
SUIT_Desktop* SUIT_Application::desktop()
{
  return myDesktop;
}

/*!
   \return FALSE if application can not be closed (because of non saved data for example). 
   This method called by SUIT_Session whin closing of application was requested.
*/
bool SUIT_Application::isPossibleToClose()
{
  return true;
}

/*!
  Performs some finalization of life cycle of this application.
  For instance, the application can force its documents(s) to close.
*/
void SUIT_Application::closeApplication()
{
  emit applicationClosed( this );
}

/*!
  \return active Study. If Application supports wirking with several studies this method should be redefined
*/
SUIT_Study* SUIT_Application::activeStudy() const
{
  return myStudy;
}

/*!
  \return version of application
*/
QString SUIT_Application::applicationVersion() const
{
  return QString::null;
}

/*!
  Shows the application's main widget. For non GUI application must be redefined.
*/
void SUIT_Application::start()
{
  if ( desktop() )
    desktop()->show();
}

/*!
  Opens document into active Study. If Study is empty - creates it.
  \param theFileName - name of document file
*/
bool SUIT_Application::useFile( const QString& theFileName )
{
  createEmptyStudy();
  SUIT_Study* study = activeStudy();

  bool status = study ? study->openDocument( theFileName ) : false;

  if ( !status )
  {
    setActiveStudy( 0 );
    delete study;
  }

  return status;
}

/*!
  Opens other study into active Study. If Study is empty - creates it.
  \param theName - name of study
*/
bool SUIT_Application::useStudy( const QString& theName )
{
  return false;
}

/*!
  Creates new empty Study if active Study = 0
*/
void SUIT_Application::createEmptyStudy()
{
  if ( !activeStudy() )
    setActiveStudy( createNewStudy() );
}

/*!
  \return number of Studies. 
  Must be redefined in Applications which support several studies for one Application instance.
*/
int SUIT_Application::getNbStudies() const
{
  return activeStudy() ? 1 : 0;
}

/*!
  \return global resource manager
*/
SUIT_ResourceMgr* SUIT_Application::resourceMgr() const
{
  if ( !SUIT_Session::session() )
    return 0;

  return SUIT_Session::session()->resourceMgr();
}

#define DEFAULT_MESSAGE_DELAY 3000

/*!
  Puts the message to the status bar  
  \param msg - text of message
  \param msec - time in milliseconds, after that the status label will be cleared
*/
void SUIT_Application::putInfo ( const QString& msg, const int msec )
{
  if ( !desktop() )
    return;

  if ( !myStatusLabel )
  {
    myStatusLabel = new QLabel( desktop()->statusBar() );
    desktop()->statusBar()->addWidget( myStatusLabel, 1 );
    myStatusLabel->show();
  }

  myStatusLabel->setText( msg );
  if ( msec != -1 )
    QTimer::singleShot( msec <= 0 ? DEFAULT_MESSAGE_DELAY : msec, myStatusLabel, SLOT( clear() ) );
}

/*!
  Initialize with application arguments
  \param argc - number of application arguments
  \param argv - array of application arguments
*/
SUIT_Application* SUIT_Application::startApplication( int argc, char** argv ) const
{
  return startApplication( name(), argc, argv );
}

/*!
  Initialize with application name and arguments
  \param name - name of application
  \param argc - number of application arguments
  \param argv - array of application arguments
*/
SUIT_Application* SUIT_Application::startApplication( const QString& name, int argc, char** argv ) const
{
  SUIT_Session* session = SUIT_Session::session();
  if ( !session )
    return 0;

  return session->startApplication( name, argc, argv );
}

/*!
  Sets the main window of application
  \param desk - new main window (desktop)
*/
void SUIT_Application::setDesktop( SUIT_Desktop* desk )
{
  if ( myDesktop == desk )
    return;

  delete myDesktop;
  myDesktop = desk;
  if ( myDesktop )
    connect( myDesktop, SIGNAL( activated() ), this, SLOT( onDesktopActivated() ) );
}

/*!
  Creates new instance of study.
  By default, it is called from createEmptyStudy()
  \sa createEmptyStudy()
*/
SUIT_Study* SUIT_Application::createNewStudy()
{
  return new SUIT_Study( this );
}

/*!
  Sets study as active
  \param study - instance of study to be set as active
*/
void SUIT_Application::setActiveStudy( SUIT_Study* study )
{
  if ( myStudy == study )
    return;

  myStudy = study;
}

/*!
  Creates new toolbar
  \return identificator of new toolbar in tool manager
  \param name - name of new toolbar
*/
int SUIT_Application::createTool( const QString& name )
{
  if ( !desktop() || !desktop()->toolMgr() )
    return -1;

  return desktop()->toolMgr()->createToolBar( name );
}

/*!
  Creates new toolbutton
  \return SUIT identificator of new action
  \param a - action
  \param tBar - identificator of toolbar
  \param id - proposed SUIT identificator of action (if it is -1, then must be use any free)
  \param idx - index in toolbar
*/
int SUIT_Application::createTool( QAction* a, const int tBar, const int id, const int idx )
{
  if ( !desktop() || !desktop()->toolMgr() )
    return -1;

  int regId = registerAction( id, a );
  int intId = desktop()->toolMgr()->insert( a, tBar, idx );
  return intId != -1 ? regId : -1;
}

/*!
  Creates new toolbutton
  \return SUIT identificator of new action
  \param a - action
  \param tBar - name of toolbar
  \param id - proposed SUIT identificator of action (if it is -1, then must be use any free)
  \param idx - index in toolbar
*/
int SUIT_Application::createTool( QAction* a, const QString& tBar, const int id, const int idx )
{
  if ( !desktop() || !desktop()->toolMgr() )
    return -1;

  int regId = registerAction( id, a );
  int intId = desktop()->toolMgr()->insert( a, tBar, idx );
  return intId != -1 ? regId : -1;
}

/*!
  Creates new toolbutton
  \return "id" if all right or -1 otherwise
  \param id - SUIT identificator of action
  \param tBar - identificator of toolbar
  \param idx - index in toolbar
*/
int SUIT_Application::createTool( const int id, const int tBar, const int idx )
{
  if ( !desktop() || !desktop()->toolMgr() )
    return -1;

  int intId = desktop()->toolMgr()->insert( action( id ), tBar, idx );
  return intId != -1 ? id : -1;
}

/*!
  Creates new toolbutton
  \return "id" if all right or -1 otherwise
  \param id - SUIT identificator of action
  \param tBar - name of toolbar
  \param idx - index in toolbar
*/
int SUIT_Application::createTool( const int id, const QString& tBar, const int idx )
{
  if ( !desktop() || !desktop()->toolMgr() )
    return -1;

  int intId = desktop()->toolMgr()->insert( action( id ), tBar, idx );
  return intId != -1 ? id : -1;
}

/*!
  Creates new menu item
  \return identificator of new action in menu manager
  \param subMenu - menu text of new item
  \param menu - identificator of parent menu item
  \param id - proposed identificator of action
  \param group - group in menu manager
  \param index - index in menu
*/
int SUIT_Application::createMenu( const QString& subMenu, const int menu,
                                  const int id, const int group, const int index )
{
  if ( !desktop() || !desktop()->menuMgr() )
    return -1;

  return desktop()->menuMgr()->insert( subMenu, menu, group, id, index );
}

/*!
  Creates new menu item
  \return identificator of new action in menu manager
  \param subMenu - menu text of new item
  \param menu - menu text of parent menu item
  \param id - proposed identificator of action
  \param group - group in menu manager
  \param index - index in menu
*/
int SUIT_Application::createMenu( const QString& subMenu, const QString& menu,
                                  const int id, const int group, const int index )
{
  if ( !desktop() || !desktop()->menuMgr() )
    return -1;

  return desktop()->menuMgr()->insert( subMenu, menu, group, id, index );
}

/*!
  Creates new menu item
  \return SUIT identificator of new action
  \param a - action
  \param menu - identificator of parent menu item
  \param id - proposed SUIT identificator of action
  \param group - group in menu manager
  \param index - index in menu
*/
int SUIT_Application::createMenu( QAction* a, const int menu, const int id, const int group, const int index )
{
  if ( !a || !desktop() || !desktop()->menuMgr() )
    return -1;

  int regId = registerAction( id, a );
  int intId = desktop()->menuMgr()->insert( a, menu, group, index );
  return intId != -1 ? regId : -1;
}

/*!
  Creates new menu item
  \return SUIT identificator of new action
  \param a - action
  \param menu - menu text of parent menu item
  \param id - proposed SUIT identificator of action
  \param group - group in menu manager
  \param index - index in menu
*/
int SUIT_Application::createMenu( QAction* a, const QString& menu, const int id, const int group, const int index )
{
  if ( !a || !desktop() || !desktop()->menuMgr() )
    return -1;

  int regId = registerAction( id, a );
  int intId = desktop()->menuMgr()->insert( a, menu, group, index );
  return intId != -1 ? regId : -1;
}

/*!
  Creates new menu item
  \return identificator of new action in menu manager
  \param id - SUIT identificator of action
  \param menu - menu text of parent menu item
  \param group - group in menu manager
  \param index - index in menu
*/
int SUIT_Application::createMenu( const int id, const int menu, const int group, const int index )
{
  if ( !desktop() || !desktop()->menuMgr() )
    return -1;

  int intId = desktop()->menuMgr()->insert( action( id ), menu, group, index );
  return intId != -1 ? id : -1;
}

/*!
  Creates new menu item
  \return identificator of new action in menu manager
  \param id - SUIT identificator of action
  \param menu - menu text of parent menu item
  \param group - group in menu manager
  \param index - index in menu
*/
int SUIT_Application::createMenu( const int id, const QString& menu, const int group, const int index )
{
  if ( !desktop() || !desktop()->menuMgr() )
    return -1;

  int intId = desktop()->menuMgr()->insert( action( id ), menu, group, index );
  return intId != -1 ? id : -1;
}

/*!
  Show/hide menu item corresponding to action
  \param a - action
  \param on - if it is true, the item will be shown, otherwise it will be hidden
*/
void SUIT_Application::setMenuShown( QAction* a, const bool on )
{
  setMenuShown( actionId( a ), on );
}

/*!
  Show/hide menu item corresponding to action
  \param id - identificator of action in menu manager
  \param on - if it is true, the item will be shown, otherwise it will be hidden
*/
void SUIT_Application::setMenuShown( const int id, const bool on )
{
  if ( desktop() && desktop()->menuMgr() )
    desktop()->menuMgr()->setShown( id, on );
}

/*!
  Show/hide tool button corresponding to action
  \param a - action
  \param on - if it is true, the button will be shown, otherwise it will be hidden
*/
void SUIT_Application::setToolShown( QAction* a, const bool on )
{
  setToolShown( actionId( a ), on );
}

/*!
  Show/hide menu item corresponding to action
  \param id - identificator of action in tool manager
  \param on - if it is true, the button will be shown, otherwise it will be hidden
*/
void SUIT_Application::setToolShown( const int id, const bool on )
{
  if ( desktop() && desktop()->toolMgr() )
    desktop()->toolMgr()->setShown( id, on );
}

/*!
  Show/hide both menu item and tool button corresponding to action
  \param a - action
  \param on - if it is true, the item will be shown, otherwise it will be hidden
*/
void SUIT_Application::setActionShown( QAction* a, const bool on )
{
  setMenuShown( a, on );
  setToolShown( a, on );
}

/*!
  Show/hide both menu item and tool button corresponding to action
  \param id - identificator in both menu manager and tool manager
  \param on - if it is true, the item will be shown, otherwise it will be hidden
*/
void SUIT_Application::setActionShown( const int id, const bool on )
{
  setMenuShown( id, on );
  setToolShown( id, on );
}

/*!
  \return action by it's SUIT identificator
  \param id - SUIT identificator
*/
QAction* SUIT_Application::action( const int id ) const
{
  QAction* a = 0;
  if ( myActionMap.contains( id ) )
    a = myActionMap[id];
  return a;
}

/*!
  \return SUIT identificator of action
  \param a - action
*/
int SUIT_Application::actionId( const QAction* a ) const
{
  int id = -1;
  for ( QMap<int, QAction*>::ConstIterator it = myActionMap.begin(); 
	it != myActionMap.end() && id == -1;
	++it ) {
    if ( it.data() == a )
      id = it.key();
  }
  return id;
}

/*!
  Creates action and registers it both in menu manager and tool manager
  \return new instance of action
  \param id - proposed SUIT identificator
  \param text - description
  \param icon - icon for toolbar
  \param menu - menu text
  \param tip - tool tip
  \param key - shortcut
  \param parent - parent object
  \param toggle - if it is TRUE the action will be a toggle action, otherwise it will be a command action
  \param reciever - object that contains slot
  \param member - slot to be called when action is activated
*/
QAction* SUIT_Application::createAction( const int id, const QString& text, const QIconSet& icon,
                                         const QString& menu, const QString& tip, const int key,
                                         QObject* parent, const bool toggle, QObject* reciever, const char* member )
{
  QtxAction* a = new QtxAction( text, icon, menu, key, parent, 0, toggle );
  a->setStatusTip( tip );

  if ( reciever && member )
    connect( a, SIGNAL( activated() ), reciever, member );

  registerAction( id, a );

  return a;
}

/*!
  Registers action both in menu manager and tool manager
  \param id - proposed SUIT identificator (if it is -1, auto generated one is used)
  \param a - action
*/
int SUIT_Application::registerAction( const int id, QAction* a )
{
  int ident = actionId( a );
  if ( ident != -1 )
    return ident;

  static int generatedId = -1;
  ident = id == -1 ? --generatedId : id;

  if ( action( ident ) ) 
    qWarning( "Action registration id is already in use: %d", ident );

  myActionMap.insert( ident, a );

  if ( desktop() && desktop()->menuMgr() )
    desktop()->menuMgr()->registerAction( a );

  if ( desktop() && desktop()->toolMgr() )
    desktop()->toolMgr()->registerAction( a );

  return ident;
}

/*!
  \return global action used as separator
*/
QAction* SUIT_Application::separator()
{
  return QtxActionMgr::separator();
}

/*!
  SLOT: it is called when desktop is activated
*/

void SUIT_Application::onDesktopActivated()
{
  emit activated( this );
}
