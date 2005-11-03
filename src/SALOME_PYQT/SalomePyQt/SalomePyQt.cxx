//=============================================================================
// File      : SalomePyQt.cxx
// Created   : 25/04/05
// Author    : Vadim SANDLER
// Project   : SALOME
// Copyright : 2003-2005 CEA/DEN, EDF R&D
// $Header   : $
//=============================================================================

#include "SALOME_PYQT_Module.h" // this include must be first!!!
#include "SalomePyQt.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qstringlist.h>

#include "SALOME_Event.hxx"

#include "SUIT_Session.h"
#include "SUIT_Desktop.h"
#include "SUIT_ResourceMgr.h"
#include "SUIT_Tools.h"
#include "STD_MDIDesktop.h"
#include "SalomeApp_Application.h"
#include "SalomeApp_Study.h"
#include "LightApp_SelectionMgr.h"
#include "OB_Browser.h"
#include "QtxAction.h"

using namespace std;

//====================================================================================
// static functions
//====================================================================================
/*!
  getApplication()
  Returns active application object [ static ]
*/
static SalomeApp_Application* getApplication() {
  if ( SUIT_Session::session() )
    return dynamic_cast<SalomeApp_Application*>( SUIT_Session::session()->activeApplication() );
  return NULL;
}

/*!
  getActiveStudy()
  Gets active study or 0 if there is no study opened [ static ]
*/
static SalomeApp_Study* getActiveStudy()
{
  if ( getApplication() )
    return dynamic_cast<SalomeApp_Study*>( getApplication()->activeStudy() );
  return 0;
}

//====================================================================================
// SALOME_Selection class.
//====================================================================================
static QMap<SalomeApp_Application*, SALOME_Selection*> SelMap;

/*!
  SALOME_Selection::GetSelection
  Creates or finds the selection object (one per study).
*/
SALOME_Selection* SALOME_Selection::GetSelection( SalomeApp_Application* app )
{
  SALOME_Selection* sel = 0;
  if ( app && SelMap.find( app ) != SelMap.end() )
    sel = SelMap[ app ];
  else 
    sel = SelMap[ app ] = new SALOME_Selection( app );
  return sel;
}

/*!
  SALOME_Selection::SALOME_Selection
  Selection constructor.
*/
SALOME_Selection::SALOME_Selection( QObject* p ) : QObject( p ), mySelMgr( 0 )
{
  SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( p );
  if ( app ) {
    mySelMgr = app->selectionMgr();
    connect( mySelMgr, SIGNAL( selectionChanged() ), this, SIGNAL( currentSelectionChanged() ) );
    connect( mySelMgr, SIGNAL( destroyed() ),        this, SLOT  ( onSelMgrDestroyed() ) );
  }
}
/*!
  SALOME_Selection::~SALOME_Selection
  Selection destructor. Removes selection object from the map.
*/
SALOME_Selection::~SALOME_Selection()
{
  SalomeApp_Application* app = dynamic_cast<SalomeApp_Application*>( parent() );
  if ( app && SelMap.find( app ) != SelMap.end() )
    SelMap.remove( app );
}

/*!
  SALOME_Selection::onSelMgrDestroyed
  Watches for the selection manager destroying when study is closed.
*/
void SALOME_Selection::onSelMgrDestroyed()
{
  mySelMgr = 0;
}

/*!
  SALOME_Selection::Clear
  Clears the selection.
*/
void SALOME_Selection::Clear()
{
  class TEvent: public SALOME_Event {
    LightApp_SelectionMgr* mySelMgr;
  public:
    TEvent( LightApp_SelectionMgr* selMgr ) 
      : mySelMgr( selMgr ) {}
    virtual void Execute() {
      if ( mySelMgr )
        mySelMgr->clearSelected();
    }
  };
  ProcessVoidEvent( new TEvent( mySelMgr ) );
}

/*!
  SALOME_Selection::ClearIObjects
  Clears the selection.
*/
void SALOME_Selection::ClearIObjects()
{
  Clear();
}

/*!
  SALOME_Selection::ClearFilters
  Removes all selection filters.
*/
void SALOME_Selection::ClearFilters()
{
  class TEvent: public SALOME_Event {
    LightApp_SelectionMgr* mySelMgr;
  public:
    TEvent( LightApp_SelectionMgr* selMgr ) 
      : mySelMgr( selMgr ) {}
    virtual void Execute() {
      if ( mySelMgr )
        mySelMgr->clearFilters();
    }
  };
  ProcessVoidEvent( new TEvent( mySelMgr ) );
}

//====================================================================================
// SalomePyQt class
//====================================================================================

/*!
  SalomePyQt::getDesktop
  Gets desktop. Returns 0 in error.
*/
class TGetDesktopEvent: public SALOME_Event {
public:
  typedef QWidget* TResult;
  TResult myResult;
  TGetDesktopEvent() : myResult( 0 ) {}
  virtual void Execute() {
    if ( getApplication() )
      myResult = (QWidget*)( getApplication()->desktop() );
  }
};
QWidget* SalomePyQt::getDesktop()
{
  return ProcessEvent( new TGetDesktopEvent() );
}

/*!
  SalomePyQt::getMainFrame
  Gets workspace widget. Returns 0 in error.
*/
class TGetMainFrameEvent: public SALOME_Event {
public:
  typedef QWidget* TResult;
  TResult myResult;
  TGetMainFrameEvent() : myResult( 0 ) {}
  virtual void Execute() {
    if ( getApplication() ) {
      SUIT_Desktop* aDesktop = getApplication()->desktop();
      myResult = (QWidget*)( aDesktop->centralWidget() );
    }
  }
};
QWidget* SalomePyQt::getMainFrame()
{
  return ProcessEvent( new TGetMainFrameEvent() );
}

/*!
  SalomePyQt::getMainMenuBar
  Gets main menu. Returns 0 in error.
*/
class TGetMainMenuBarEvent: public SALOME_Event {
public:
  typedef QMenuBar* TResult;
  TResult myResult;
  TGetMainMenuBarEvent() : myResult( 0 ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      myResult = anApp->desktop()->menuBar();
    }
  }
};
QMenuBar* SalomePyQt::getMainMenuBar() 
{
  return ProcessEvent( new TGetMainMenuBarEvent() );
}

/*!
  SalomePyQt::getPopupMenu
  Gets an main menu's child popup menu by its id
*/
class TGetPopupMenuEvent: public SALOME_Event {
public:
  typedef QPopupMenu* TResult;
  TResult  myResult;
  MenuName myMenuName;
  TGetPopupMenuEvent( const MenuName menu ) : myResult( 0 ), myMenuName( menu ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      QMenuBar* menuBar = anApp->desktop()->menuBar();
      if ( menuBar ) {
        QString menu;
        switch( myMenuName) {
        case File:
          menu = QObject::tr( "MEN_DESK_FILE" );        break;
        case View:
          menu = QObject::tr( "MEN_DESK_VIEW" );        break;
        case Edit:
          menu = QObject::tr( "MEN_DESK_EDIT" );        break;
        case Preferences:
          menu = QObject::tr( "MEN_DESK_PREFERENCES" ); break;
        case Tools:
          menu = QObject::tr( "MEN_DESK_TOOLS" );       break;
        case Window:
          menu = QObject::tr( "MEN_DESK_WINDOW" );      break;
        case Help:
          menu = QObject::tr( "MEN_DESK_HELP" );        break;
        }
        for ( int i = 0; i < menuBar->count() && !myResult; i++ ) {
          QMenuItem* item = menuBar->findItem( menuBar->idAt( i ) );
	  if ( item && item->text() == menu && item->popup() )
            myResult = item->popup();
        }
      }
    }
  }
};
QPopupMenu* SalomePyQt::getPopupMenu( const MenuName menu )
{
  return ProcessEvent( new TGetPopupMenuEvent( menu ) );
}

/*!
  SalomePyQt::getStudyId
  Returns active study's ID or 0 if there is no active study.
*/
class TGetStudyIdEvent: public SALOME_Event {
public:
  typedef int TResult;
  TResult myResult;
  TGetStudyIdEvent() : myResult( 0 ) {}
  virtual void Execute() {
    if ( SalomeApp_Study* aStudy = getActiveStudy() ) {
      myResult = aStudy->studyDS()->StudyId();
    }
  }
};
int SalomePyQt::getStudyId()
{
  return ProcessEvent( new TGetStudyIdEvent() );
}

/*!
  SalomePyQt::getSelection
  Creates a Selection object (to provide a compatibility with previous SALOME GUI).
*/
class TGetSelectionEvent: public SALOME_Event {
public:
  typedef SALOME_Selection* TResult;
  TResult myResult;
  TGetSelectionEvent() : myResult( 0 ) {}
  virtual void Execute() {
    myResult = SALOME_Selection::GetSelection( getApplication() );
  }
};
SALOME_Selection* SalomePyQt::getSelection()
{
  return ProcessEvent( new TGetSelectionEvent() );
}

/*!
  SalomePyQt::putInfo
  Puts an information message to the desktop's status bar
  (with optional delay parameter given in seconds)
*/
class TPutInfoEvent: public SALOME_Event {
  QString myMsg;
  int     mySecs;
public:
  TPutInfoEvent( const QString& msg, const int sec = 0 ) : myMsg( msg ), mySecs( sec ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      anApp->putInfo( myMsg, mySecs * 1000 );
    }
  }
};
void SalomePyQt::putInfo( const QString& msg, const int sec )
{
  ProcessVoidEvent( new TPutInfoEvent( msg, sec ) );
}

/*!
  SalomePyQt::getActiveComponent
  Returns an active component name or empty string if there is no active component
*/
class TGetActiveComponentEvent: public SALOME_Event {
public:
  typedef QString TResult;
  TResult myResult;
  TGetActiveComponentEvent() {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      if ( CAM_Module* mod = anApp->activeModule() ) {
        myResult = mod->name("");
      }
    }
  }
};
const QString SalomePyQt::getActiveComponent()
{
  return ProcessEvent( new TGetActiveComponentEvent() );
}

/*!
  SalomePyQt::updateObjBrowser
  Updates an Object Browser of a given study.
  If <studyId> <= 0 the active study's object browser is updated.
  <updateSelection> parameter is obsolete parameter and currently not used. To be removed lately.
*/
void SalomePyQt::updateObjBrowser( const int studyId, bool updateSelection )
{  
  class TEvent: public SALOME_Event {
    int  myStudyId;
    bool myUpdateSelection;
  public:
    TEvent( const int studyId, bool updateSelection ) 
      : myStudyId( studyId ), myUpdateSelection( updateSelection ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        if ( getActiveStudy() && myStudyId <= 0 )
          myStudyId = getActiveStudy()->id();
	if ( myStudyId > 0 ) {
          QPtrList<SUIT_Application> apps = SUIT_Session::session()->applications();
          QPtrListIterator<SUIT_Application> it( apps );
	  for( ; it.current(); ++it ) {
            SalomeApp_Application* anApp = dynamic_cast<SalomeApp_Application*>( it.current() );
            if ( anApp && anApp->activeStudy() && anApp->activeStudy()->id() == myStudyId )
	      anApp->updateObjectBrowser();
          }
        }
      }
    }
  };
  ProcessVoidEvent( new TEvent( studyId, updateSelection ) );
}

const char* DEFAULT_SECTION = "SalomePyQt";

/*!
  SalomePyQt::addStringSetting
  Adds a string setting to the application preferences
  <autoValue> parameter is obsolete parameter and currently not used. To be removed lately.
  This function is obsolete. Use addSetting() instead.
*/
void SalomePyQt::addStringSetting( const QString& name, const QString& value, bool autoValue )
{
  class TEvent: public SALOME_Event {
    QString myName;
    QString myValue;
    bool    myAutoValue;
  public:
    TEvent( const QString& name, const QString& value, bool autoValue ) 
      : myName( name ), myValue( value ), myAutoValue( autoValue ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	QStringList sl = QStringList::split( ":", myName );
	QString _sec = sl.count() > 1 ? sl[ 0 ].stripWhiteSpace() : QString( DEFAULT_SECTION );
	QString _nam = sl.count() > 1 ? sl[ 1 ].stripWhiteSpace() : sl.count() > 0 ? sl[ 0 ].stripWhiteSpace() : QString( "" );
	if ( !_sec.isEmpty() && !_nam.isEmpty() )
          resMgr->setValue( _sec, _nam, myValue );
      }
    }
  };
  ProcessVoidEvent( new TEvent( name, value, autoValue ) );
}

/*!
  SalomePyQt::addIntSetting
  Adds an integer setting to the application preferences
  <autoValue> parameter is obsolete parameter and currently not used. To be removed lately.
  This function is obsolete. Use addSetting() instead.
*/
void SalomePyQt::addIntSetting( const QString& name, const int value, bool autoValue)
{
  class TEvent: public SALOME_Event {
    QString myName;
    int     myValue;
    bool    myAutoValue;
  public:
    TEvent( const QString& name, const int value, bool autoValue ) 
      : myName( name ), myValue( value ), myAutoValue( autoValue ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	QStringList sl = QStringList::split( ":", myName );
	QString _sec = sl.count() > 1 ? sl[ 0 ].stripWhiteSpace() : QString( DEFAULT_SECTION );
	QString _nam = sl.count() > 1 ? sl[ 1 ].stripWhiteSpace() : sl.count() > 0 ? sl[ 0 ].stripWhiteSpace() : QString( "" );
	if ( !_sec.isEmpty() && !_nam.isEmpty() )
          resMgr->setValue( _sec, _nam, myValue );
      }
    }
  };
  ProcessVoidEvent( new TEvent( name, value, autoValue ) );
}

/*!
  SalomePyQt::addDoubleSetting
  Adds an double setting to the application preferences
  <autoValue> parameter is obsolete parameter and currently not used. To be removed lately.
  This function is obsolete. Use addSetting() instead.
*/
void SalomePyQt::addDoubleSetting( const QString& name, const double value, bool autoValue )
{
  class TEvent: public SALOME_Event {
    QString myName;
    double  myValue;
    bool    myAutoValue;
  public:
    TEvent( const QString& name, const double value, bool autoValue ) 
      : myName( name ), myValue( value ), myAutoValue( autoValue ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	QStringList sl = QStringList::split( ":", myName );
	QString _sec = sl.count() > 1 ? sl[ 0 ].stripWhiteSpace() : QString( DEFAULT_SECTION );
	QString _nam = sl.count() > 1 ? sl[ 1 ].stripWhiteSpace() : sl.count() > 0 ? sl[ 0 ].stripWhiteSpace() : QString( "" );
	if ( !_sec.isEmpty() && !_nam.isEmpty() )
          resMgr->setValue( _sec, _nam, myValue );
      }
    }
  };
  ProcessVoidEvent( new TEvent( name, value, autoValue ) );
}

/*!
  SalomePyQt::removeSettings
  Removes a setting from the application preferences
  This function is obsolete. Use removeSetting() instead.
*/
void SalomePyQt::removeSettings( const QString& name )
{
  class TEvent: public SALOME_Event {
    QString myName;
  public:
    TEvent( const QString& name ) : myName( name ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	QStringList sl = QStringList::split( ":", myName );
	QString _sec = sl.count() > 1 ? sl[ 0 ].stripWhiteSpace() : QString( DEFAULT_SECTION );
	QString _nam = sl.count() > 1 ? sl[ 1 ].stripWhiteSpace() : sl.count() > 0 ? sl[ 0 ].stripWhiteSpace() : QString( "" );
	if ( !_sec.isEmpty() && !_nam.isEmpty() )
          resMgr->remove( _sec, _nam );
      }
    }
  };
  ProcessVoidEvent( new TEvent( name ) );
}

/*!
  SalomePyQt::getSetting
  Gets a setting value (as string)
  This function is obsolete. Use stringSetting(), integerSetting(), 
  boolSetting(), stringSetting() or colorSetting() instead.
*/
class TGetSettingEvent: public SALOME_Event {
public:
  typedef QString TResult;
  TResult myResult;
  QString myName;
  TGetSettingEvent( const QString& name ) : myName( name ) {}
  virtual void Execute() {
    if ( SUIT_Session::session() ) {
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      QStringList sl = QStringList::split( ":", myName );
      QString _sec = sl.count() > 1 ? sl[ 0 ].stripWhiteSpace() : QString( DEFAULT_SECTION );
      QString _nam = sl.count() > 1 ? sl[ 1 ].stripWhiteSpace() : sl.count() > 0 ? sl[ 0 ].stripWhiteSpace() : QString( "" );
      myResult = ( !_sec.isEmpty() && !_nam.isEmpty() ) ? resMgr->stringValue( _sec, _nam, "" ) : QString( "" );
    }
  }
};
QString SalomePyQt::getSetting( const QString& name )
{
  return ProcessEvent( new TGetSettingEvent( name ) );
}

/*!
  SalomePyQt::addSetting
  Adds a double setting to the application preferences
*/
void SalomePyQt::addSetting( const QString& section, const QString& name, const double value )
{
  class TEvent: public SALOME_Event {
    QString mySection;
    QString myName;
    double  myValue;
  public:
    TEvent( const QString& section, const QString& name, double value ) 
      : mySection( section ), myName( name ), myValue( value ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	if ( !mySection.isEmpty() && !myName.isEmpty() )
          resMgr->setValue( mySection, myName, myValue );
      }
    }
  };
  ProcessVoidEvent( new TEvent( section, name, value ) );
}

/*!
  SalomePyQt::addSetting
  Adds an integer setting to the application preferences
*/
void SalomePyQt::addSetting( const QString& section, const QString& name, const int value )
{
  class TEvent: public SALOME_Event {
    QString mySection;
    QString myName;
    int     myValue;
  public:
    TEvent( const QString& section, const QString& name, int value ) 
      : mySection( section ), myName( name ), myValue( value ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	if ( !mySection.isEmpty() && !myName.isEmpty() )
          resMgr->setValue( mySection, myName, myValue );
      }
    }
  };
  ProcessVoidEvent( new TEvent( section, name, value ) );
}

/*!
  SalomePyQt::addSetting
  Adds a string setting to the application preferences
*/
void SalomePyQt::addSetting( const QString& section, const QString& name, const QString& value )
{
  class TEvent: public SALOME_Event {
    QString mySection;
    QString myName;
    QString myValue;
  public:
    TEvent( const QString& section, const QString& name, const QString& value ) 
      : mySection( section ), myName( name ), myValue( value ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	if ( !mySection.isEmpty() && !myName.isEmpty() )
          resMgr->setValue( mySection, myName, myValue );
      }
    }
  };
  ProcessVoidEvent( new TEvent( section, name, value ) );
}

/*!
  SalomePyQt::addSetting
  Adds a color setting to the application preferences
*/
void SalomePyQt::addSetting( const QString& section, const QString& name, const QColor& value )
{
  class TEvent: public SALOME_Event {
    QString mySection;
    QString myName;
    QColor  myValue;
  public:
    TEvent( const QString& section, const QString& name, const QColor& value ) 
      : mySection( section ), myName( name ), myValue( value ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	if ( !mySection.isEmpty() && !myName.isEmpty() )
          resMgr->setValue( mySection, myName, myValue );
      }
    }
  };
  ProcessVoidEvent( new TEvent( section, name, value ) );
}

/*!
  SalomePyQt::integerSetting
  Gets an integer setting from the application preferences
*/
class TGetIntSettingEvent: public SALOME_Event {
public:
  typedef int TResult;
  TResult myResult;
  QString mySection;
  QString myName;
  TResult myDefault;
  TGetIntSettingEvent( const QString& section, const QString& name, const int def ) 
    : mySection( section ), myName( name ), myDefault( def ) {}
  virtual void Execute() {
    if ( SUIT_Session::session() ) {
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      myResult = ( !mySection.isEmpty() && !myName.isEmpty() ) ? resMgr->integerValue( mySection, myName, myDefault ) : myDefault;
    }
  }
};
int SalomePyQt::integerSetting( const QString& section, const QString& name, const int def )
{
  return ProcessEvent( new TGetIntSettingEvent( section, name, def ) );
}

/*!
  SalomePyQt::doubleSetting
  Gets a double setting from the application preferences
*/
class TGetDblSettingEvent: public SALOME_Event {
public:
  typedef double TResult;
  TResult myResult;
  QString mySection;
  QString myName;
  TResult myDefault;
  TGetDblSettingEvent( const QString& section, const QString& name, const double def ) 
    : mySection( section ), myName( name ), myDefault( def ) {}
  virtual void Execute() {
    if ( SUIT_Session::session() ) {
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      myResult = ( !mySection.isEmpty() && !myName.isEmpty() ) ? resMgr->doubleValue( mySection, myName, myDefault ) : myDefault;
    }
  }
};
double SalomePyQt::doubleSetting( const QString& section, const QString& name, const int def )
{
  return ProcessEvent( new TGetDblSettingEvent( section, name, def ) );
}

/*!
  SalomePyQt::boolSetting
  Gets a boolean setting from the application preferences
*/
class TGetBoolSettingEvent: public SALOME_Event {
public:
  typedef bool TResult;
  TResult myResult;
  QString mySection;
  QString myName;
  TResult myDefault;
  TGetBoolSettingEvent( const QString& section, const QString& name, const bool def ) 
    : mySection( section ), myName( name ), myDefault( def ) {}
  virtual void Execute() {
    if ( SUIT_Session::session() ) {
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      myResult = ( !mySection.isEmpty() && !myName.isEmpty() ) ? resMgr->booleanValue( mySection, myName, myDefault ) : myDefault;
    }
  }
};
bool SalomePyQt::boolSetting( const QString& section, const QString& name, const bool def )
{
  return ProcessEvent( new TGetBoolSettingEvent( section, name, def ) );
}

/*!
  SalomePyQt::stringSetting
  Gets a string setting from the application preferences
*/
class TGetStrSettingEvent: public SALOME_Event {
public:
  typedef QString TResult;
  TResult myResult;
  QString mySection;
  QString myName;
  TResult myDefault;
  TGetStrSettingEvent( const QString& section, const QString& name, const QString& def ) 
    : mySection( section ), myName( name ), myDefault( def ) {}
  virtual void Execute() {
    if ( SUIT_Session::session() ) {
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      myResult = ( !mySection.isEmpty() && !myName.isEmpty() ) ? resMgr->stringValue( mySection, myName, myDefault ) : myDefault;
    }
  }
};
QString SalomePyQt::stringSetting( const QString& section, const QString& name, const QString& def )
{
  return ProcessEvent( new TGetStrSettingEvent( section, name, def ) );
}

/*!
  SalomePyQt::colorSetting
  Gets a color setting from the application preferences
*/
class TGetColorSettingEvent: public SALOME_Event {
public:
  typedef QColor TResult;
  TResult myResult;
  QString mySection;
  QString myName;
  TResult myDefault;
  TGetColorSettingEvent( const QString& section, const QString& name, const QColor& def ) 
    : mySection( section ), myName( name ), myDefault( def ) {}
  virtual void Execute() {
    if ( SUIT_Session::session() ) {
      SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
      myResult = ( !mySection.isEmpty() && !myName.isEmpty() ) ? resMgr->colorValue( mySection, myName, myDefault ) : myDefault;
    }
  }
};
QColor SalomePyQt::colorSetting ( const QString& section, const QString& name, const QColor& def )
{
  return ProcessEvent( new TGetColorSettingEvent( section, name, def ) );
}

/*!
  SalomePyQt::removeSetting
  Removes a setting from the application preferences
*/
void SalomePyQt::removeSetting( const QString& section, const QString& name )
{
  class TEvent: public SALOME_Event {
    QString mySection;
    QString myName;
  public:
    TEvent( const QString& section, const QString& name ) : mySection( section ), myName( name ) {}
    virtual void Execute() {
      if ( SUIT_Session::session() ) {
        SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();
	if ( !mySection.isEmpty() && !myName.isEmpty() )
          resMgr->remove( mySection, myName );
      }
    }
  };
  ProcessVoidEvent( new TEvent( section, name ) );
}

/*!
  SalomePyQt::getFileName
  Displays 'Open/Save file' dialog box and returns a user's choice (file name)
*/
class TGetFileNameEvent: public SALOME_Event {
public:
  typedef QString TResult;
  TResult     myResult;
  QWidget*    myParent;
  QString     myInitial;
  QStringList myFilters;
  QString     myCaption;
  bool        myOpen;
  TGetFileNameEvent( QWidget*           parent, 
		     const QString&     initial, 
		     const QStringList& filters, 
 		     const QString&     caption,
	 	     bool               open ) 
    : myParent ( parent ), 
      myInitial( initial ), 
      myFilters( filters ), 
      myCaption( caption ), 
      myOpen ( open ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      myResult = anApp->getFileName( myOpen, myInitial, myFilters.join(";;"), myCaption, myParent );
    }
  }
};
QString SalomePyQt::getFileName( QWidget*           parent, 
				 const QString&     initial, 
				 const QStringList& filters, 
				 const QString&     caption,
				 bool               open )
{
  return ProcessEvent( new TGetFileNameEvent( parent, initial, filters, caption, open ) );
}

/*!
  SalomePyQt::getOpenFileNames
  Displays 'Open files' dialog box and returns a user's choice (a list of file names)
*/
class TGetOpenFileNamesEvent: public SALOME_Event {
public:
  typedef QStringList TResult;
  TResult     myResult;
  QWidget*    myParent;
  QString     myInitial;
  QStringList myFilters;
  QString     myCaption;
  TGetOpenFileNamesEvent( QWidget*           parent, 
			  const QString&     initial, 
			  const QStringList& filters, 
			  const QString&     caption ) 
    : myParent ( parent ), 
      myInitial( initial ), 
      myFilters( filters ), 
      myCaption( caption ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      myResult = anApp->getOpenFileNames( myInitial, myFilters.join(";;"), myCaption, myParent );
    }
  }
};
QStringList SalomePyQt::getOpenFileNames( QWidget*           parent, 
					  const QString&     initial, 
					  const QStringList& filters, 
					  const QString&     caption )
{
  return ProcessEvent( new TGetOpenFileNamesEvent( parent, initial, filters, caption ) );
}

/*!
  SalomePyQt::getExistingDirectory
  Displays 'Get Directory' dialog box and returns a user's choice (a directory name)
*/
class TGetExistingDirectoryEvent: public SALOME_Event {
public:
  typedef QString TResult;
  TResult     myResult;
  QWidget*    myParent;
  QString     myInitial;
  QString     myCaption;
  TGetExistingDirectoryEvent( QWidget*           parent, 
			      const QString&     initial, 
			      const QString&     caption ) 
    : myParent ( parent ), 
      myInitial( initial ), 
      myCaption( caption ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      myResult = anApp->getDirectory( myInitial, myCaption, myParent );
    }
  }
};
QString SalomePyQt::getExistingDirectory( QWidget*       parent,
					  const QString& initial,
					  const QString& caption )
{
  return ProcessEvent( new TGetExistingDirectoryEvent( parent, initial, caption ) );
}

/*!
  SalomePyQt::helpContext
  Opens external browser to display 'context help' information
  current implementation does nothing.
*/
void SalomePyQt::helpContext( const QString& source, const QString& context ) {
  class TEvent: public SALOME_Event {
    QString mySource;
    QString myContext;
  public:
    TEvent( const QString& source, const QString& context ) 
      : mySource( source ), myContext( context ) {}
    virtual void Execute() {
      if ( /*SalomeApp_Application* anApp =*/ getApplication() ) {
	// VSR: TODO
        // anApp->helpContext( mySource, myContext );
      }
    }
  };
  ProcessVoidEvent( new TEvent( source, context ) );
}

/*!
  SalomePyQt::dumpView
  Dumps the contents of the currently active view to the image file 
  in the given format (JPEG, PNG, BMP are supported)
*/
class TDumpViewEvent: public SALOME_Event {
public:
  typedef bool TResult;
  TResult myResult;
  QString myFileName;
  TDumpViewEvent( const QString& filename ) 
    : myResult ( false ), myFileName( filename ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      SUIT_ViewManager* vm = anApp->activeViewManager();
      if ( vm ) { 
        SUIT_ViewWindow* vw = vm->getActiveView();
	if ( vw ) {
          QImage im = vw->dumpView();
	  if ( !im.isNull() && !myFileName.isEmpty() ) {
            QString fmt = SUIT_Tools::extension( myFileName ).upper();
	    if ( fmt.isEmpty() ) fmt = QString( "BMP" ); // default format
	    if ( fmt == "JPG" )  fmt = "JPEG";
	    myResult = im.save( myFileName, fmt.latin1() );
          }
	}
      }
    }
  }
};
bool SalomePyQt::dumpView( const QString& filename )
{
  return ProcessEvent( new TDumpViewEvent( filename ) );
}

/*!
  SalomePyQt::createTool
  These methods allow operating with the toolbars:
  - create a new toolbar or get the existing one (the toolbar name is passed as parameter);
    this method returns an id of the toolbar;
  - add action with given id (must be created previously) and optional index to the existing toolbar
    (toobar is identified either by its id or by its name)
    these methods return an id of the action.
  If error occurs, the -1 value is returned.
*/
class CrTool
{
public:
  CrTool( const QString& tBar ) 
    : myCase( 0 ), myTbName( tBar ) {}
  CrTool( const int id, const int tBar, const int idx ) 
    : myCase( 1 ), myId( id ), myTbId( tBar ), myIndex( idx ) {}
  CrTool( const int id, const QString& tBar, const int idx )
    : myCase( 2 ), myId( id ), myTbName( tBar ), myIndex( idx ) {}
  CrTool( QtxAction* action, const int tbId, const int id, const int idx )
    : myCase( 3 ), myAction( action ), myTbId( tbId ), myId( id ), myIndex( idx ) {}
  CrTool( QtxAction* action, const QString& tBar, const int id, const int idx )
    : myCase( 4 ), myAction( action ), myTbName( tBar ), myId( id ), myIndex( idx ) {}

  int execute( SALOME_PYQT_Module* module ) const
  {
    if ( module ) {
      switch ( myCase ) {
      case 0:
        return module->createTool( myTbName );
      case 1:
        return module->createTool( myId, myTbId, myIndex );
      case 2:
        return module->createTool( myId, myTbName, myIndex );
      case 3:
        return module->createTool( myAction, myTbId, myId, myIndex );
      case 4:
        return module->createTool( myAction, myTbName, myId, myIndex );
      }
    }
    return -1;
  }
private:
   int        myCase;
   QString    myTbName;
   int        myTbId;
   QtxAction* myAction;
   int        myId;
   int        myIndex;
};
class TCreateToolEvent: public SALOME_Event {
public:
  typedef int TResult;
  TResult myResult;
  const CrTool& myCrTool;
  TCreateToolEvent( const CrTool& crTool ) 
    : myResult( -1 ), myCrTool( crTool ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      SALOME_PYQT_Module* module = SALOME_PYQT_Module::getInitModule();
      if ( !module )
        module = dynamic_cast<SALOME_PYQT_Module*>( anApp->activeModule() );
      myResult = myCrTool.execute( module );
    }
  }
};
// create new toolbar or get existing by name 
int SalomePyQt::createTool( const QString& tBar )
{
  return ProcessEvent( new TCreateToolEvent( CrTool( tBar ) ) );
}
// add action with id and index to the existing tollbar
int SalomePyQt::createTool( const int id, const int tBar, const int idx )
{
  return ProcessEvent( new TCreateToolEvent( CrTool( id, tBar, idx ) ) );
}
// add action with id and index to the existing tollbar
int SalomePyQt::createTool( const int id, const QString& tBar, const int idx )
{
  return ProcessEvent( new TCreateToolEvent( CrTool( id, tBar, idx ) ) );
}
// add action with id and index to the existing tollbar
int SalomePyQt::createTool( QtxAction* a, const int tBar, const int id, const int idx )
{
  return ProcessEvent( new TCreateToolEvent( CrTool( a, tBar, id, idx ) ) );
}
// add action with id and index to the existing tollbar
int SalomePyQt::createTool( QtxAction* a, const QString& tBar, const int id, const int idx )
{
  return ProcessEvent( new TCreateToolEvent( CrTool( a, tBar, id, idx ) ) );
}

/*!
  SalomePyQt::createMenu
  These methods allow operating with the main menu:
  - create a new menu or submenu or get the existing one (the parent menu name or id is passed as parameter, 
    if it is empty or -1, it means that main menu is created, otherwise submenu is created);
    this method returns an id of the menu/submenu;
  - add action with given id (must be created previously) and optional index and group number to the existing menu
    or submenu (menu name or id us passed as parameter)
    these methods return an id of the action.
  If error occurs, the -1 value is returned.
*/
class CrMenu
{
public:
  CrMenu( const QString& subMenu, const int menu, const int group, const int idx ) 
    : myCase( 0 ), mySubMenuName( subMenu ), myMenuId( menu ), myGroup( group ), myIndex( idx ) {}
  CrMenu( const QString& subMenu, const QString& menu, const int group, const int idx ) 
    : myCase( 1 ), mySubMenuName( subMenu ), myMenuName( menu ), myGroup( group ), myIndex( idx ) {}
  CrMenu( const int id, const int menu, const int group, const int idx ) 
    : myCase( 2 ), myId( id ), myMenuId( menu ), myGroup( group ), myIndex( idx ) {}
  CrMenu( const int id, const QString& menu, const int group, const int idx ) 
    : myCase( 3 ), myId( id ), myMenuName( menu ), myGroup( group ), myIndex( idx ) {}
  CrMenu( QtxAction* action, const int menu, const int id, const int group, const int idx ) 
    : myCase( 4 ), myAction( action ), myMenuId( menu ), myId( id ), myGroup( group ), myIndex( idx ) {}
  CrMenu( QtxAction* action, const QString& menu, const int id, const int group, const int idx ) 
    : myCase( 5 ), myAction( action ), myMenuName( menu ), myId( id ), myGroup( group ), myIndex( idx ) {}

  int execute( SALOME_PYQT_Module* module ) const
  {
    if ( module ) {
      switch ( myCase ) {
      case 0:
        return module->createMenu( mySubMenuName, myMenuId, -1, myGroup, myIndex );
      case 1:
        return module->createMenu( mySubMenuName, myMenuName, -1, myGroup, myIndex );
      case 2:
        return module->createMenu( myId, myMenuId, myGroup, myIndex );
      case 3:
        return module->createMenu( myId, myMenuName, myGroup, myIndex );
      case 4:
        return module->createMenu( myAction, myMenuId, myId, myGroup, myIndex );
      case 5:
        return module->createMenu( myAction, myMenuName, myId, myGroup, myIndex );
      }
    }
    return -1;
  }
private:
   int        myCase;
   QString    myMenuName;
   int        myMenuId;
   QString    mySubMenuName;
   int        myGroup;
   QtxAction* myAction;
   int        myId;
   int        myIndex;
};
class TCreateMenuEvent: public SALOME_Event {
public:
  typedef int TResult;
  TResult myResult;
  const CrMenu& myCrMenu;
  TCreateMenuEvent( const CrMenu& crMenu ) 
    : myResult( -1 ), myCrMenu( crMenu ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      SALOME_PYQT_Module* module = SALOME_PYQT_Module::getInitModule();
      if ( !module )
        module = dynamic_cast<SALOME_PYQT_Module*>( anApp->activeModule() );
      myResult = myCrMenu.execute( module );
    }
  }
};
int SalomePyQt::createMenu( const QString& subMenu, const int menu, const int group, const int idx )
{
  return ProcessEvent( new TCreateMenuEvent( CrMenu( subMenu, menu, group, idx ) ) );
}

int SalomePyQt::createMenu( const QString& subMenu, const QString& menu, const int group, const int idx )
{
  return ProcessEvent( new TCreateMenuEvent( CrMenu( subMenu, menu, group, idx ) ) );
}

int SalomePyQt::createMenu( const int id, const int menu, const int group, const int idx )
{
  return ProcessEvent( new TCreateMenuEvent( CrMenu( id, menu, group, idx ) ) );
}

int SalomePyQt::createMenu( const int id, const QString& menu, const int group, const int idx )
{
  return ProcessEvent( new TCreateMenuEvent( CrMenu( id, menu, group, idx ) ) );
}

int SalomePyQt::createMenu( QtxAction* a, const int menu, const int id, const int group, const int idx )
{
  return ProcessEvent( new TCreateMenuEvent( CrMenu( a, menu, id, group, idx ) ) );
}

int SalomePyQt::createMenu( QtxAction* a, const QString& menu, const int id, const int group, const int idx )
{
  return ProcessEvent( new TCreateMenuEvent( CrMenu( a, menu, id, group, idx ) ) );
}

/*!
  SalomePyQt::createSeparator
  Create a separator action which can be then used in the menu or toolbar.
*/
class TCreateSepEvent: public SALOME_Event {
public:
  typedef QtxAction* TResult;
  TResult myResult;
  TCreateSepEvent() 
    : myResult( 0 ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      SALOME_PYQT_Module* module = SALOME_PYQT_Module::getInitModule();
      if ( !module )
        module = dynamic_cast<SALOME_PYQT_Module*>( anApp->activeModule() );
      if ( module )
        myResult = (QtxAction*)module->createSeparator();
    }
  }
};
QtxAction* SalomePyQt::createSeparator()
{
  return ProcessEvent( new TCreateSepEvent() );
}

/*!
  SalomePyQt::createAction
  Create an action which can be then used in the menu or toolbar:
  - id         : the unique id action to be registered to;
  - menuText   : action text which should appear in menu;
  - tipText    : text which should appear in the tooltip;
  - statusText : text which should appear in the status bar when action is activated;
  - icon       : the name of the icon file (the actual icon file name can be coded in the translation files);
  - key        : the key accelrator for the action
  - toggle     : if true the action is checkable
*/
class TCreateActionEvent: public SALOME_Event {
public:
  typedef QtxAction* TResult;
  TResult myResult;
  int     myId;
  QString myMenuText;
  QString myTipText;
  QString myStatusText;
  QString myIcon;
  int     myKey;
  bool    myToggle;
  TCreateActionEvent( const int id, const QString& menuText, const QString& tipText, 
		      const QString& statusText, const QString& icon, const int key, const bool toggle ) 
    : myResult( 0 ), myId( id ), myMenuText( menuText ), myTipText( tipText ),
      myStatusText( statusText ), myIcon( icon ), myKey( key ), myToggle( toggle ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      SALOME_PYQT_Module* module = SALOME_PYQT_Module::getInitModule();
      if ( !module )
        module = dynamic_cast<SALOME_PYQT_Module*>( anApp->activeModule() );
      if ( module )
        myResult = (QtxAction*)module->createAction( myId, myTipText, myIcon, myMenuText, myStatusText, myKey, myToggle );
    }
  }
};
QtxAction* SalomePyQt::createAction( const int id,           const QString& menuText, 
				     const QString& tipText, const QString& statusText, 
				     const QString& icon,    const int key, const bool toggle )
{
  return ProcessEvent( new TCreateActionEvent( id, menuText, tipText, statusText, icon, key, toggle ) );
}

/*!
  SalomePyQt::action
  Get an action by its id. Returns 0 if the action with such id was not registered.
*/
class TActionEvent: public SALOME_Event {
public:
  typedef QtxAction* TResult;
  TResult myResult;
  int     myId;
  TActionEvent( const int id )
    : myResult( 0 ), myId( id ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      SALOME_PYQT_Module* module = SALOME_PYQT_Module::getInitModule();
      if ( !module )
        module = dynamic_cast<SALOME_PYQT_Module*>( anApp->activeModule() );
      if ( module )
        myResult = (QtxAction*)module->action( myId );
    }
  }
};
QtxAction* SalomePyQt::action( const int id )
{
  return ProcessEvent( new TActionEvent( id ) );
}

/*!
  SalomePyQt::actionId
  Get an action id. Returns -1 if the action was not registered.
*/
class TActionIdEvent: public SALOME_Event {
public:
  typedef  int TResult;
  TResult  myResult;
  const QtxAction* myAction;
  TActionIdEvent( const QtxAction* action )
    : myResult( -1 ), myAction( action ) {}
  virtual void Execute() {
    if ( SalomeApp_Application* anApp = getApplication() ) {
      SALOME_PYQT_Module* module = SALOME_PYQT_Module::getInitModule();
      if ( !module )
        module = dynamic_cast<SALOME_PYQT_Module*>( anApp->activeModule() );
      if ( module )
        myResult = module->actionId( myAction );
    }
  }
};
int SalomePyQt::actionId( const QtxAction* a )
{
  return ProcessEvent( new TActionIdEvent( a ) );
}
