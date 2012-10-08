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

//  SALOME Session : implementation of Session.idl
//  File : SALOME_Session_Server.cxx
//  Author : Paul RASCLE, EDF
//  Module : SALOME

#include <SALOME_NamingService.hxx>
#include <SALOME_ModuleCatalog_impl.hxx>
#include <SALOME_LifeCycleCORBA.hxx>
#include <SALOME_Event.h>

#include <Basics_OCCTVersion.hxx>

#include <Container_init_python.hxx>
#include <ConnectionManager_i.hxx>
#include <RegistryService.hxx>

#ifdef ENABLE_TESTRECORDER
  #include <TestApplication.h>
#endif

#include <OpUtil.hxx>
#include <Utils_ORB_INIT.hxx>
#include <Utils_SINGLETON.hxx>
#include <Utils_SALOME_Exception.hxx>
#include <Utils_CorbaException.hxx>

#include <utilities.h>
#include "Session_ServerLauncher.hxx"
#include "Session_ServerCheck.hxx"
#include "Session_Session_i.hxx"

#include <Qtx.h>
#include <QtxSplash.h>

#include <Style_Salome.h>

#include "GUI_version.h"
#include <SUIT_Tools.h>
#include <SUIT_Session.h>
#include <SUIT_Application.h>
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_ExceptionHandler.h>

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SALOME_Session)
#include CORBA_SERVER_HEADER(SALOMEDS)

#include <QDir>
#include <QFile>
#include <QApplication>
#include <QMutex>
#include <QWaitCondition>
#include <QRegExp>
#include <QTextStream>

/*! - read arguments, define list of server to launch with their arguments.
 * - wait for naming service
 * - create and run a thread for launch of all servers
 *
*/

//! CORBA server for SALOME Session
/*!
 * SALOME_Session Server launches a SALOME session servant.
 * The servant registers to the Naming Service.
 * See SALOME_Session.idl for interface specification.
 *
 * Main services offered by the servant are:
 * - launch GUI
 * - stop Session ( must be idle )
 * - get session state
 */

PyObject* salome_shared_modules_module = 0;

void MessageOutput( QtMsgType type, const char* msg )
{
  switch ( type )
  {
  case QtDebugMsg:
    //MESSAGE( "Debug: " << msg );
    break;
  case QtWarningMsg:
    MESSAGE( "Warning: " << msg );
    break;
  case QtFatalMsg:
    MESSAGE( "Fatal: " << msg );
    break;
  }
}

/* XPM */
static const char* pixmap_not_found_xpm[] = {
"16 16 3 1",
"       c None",
".      c #000000",
"+      c #A80000",
"                ",
"                ",
"    .     .     ",
"   .+.   .+.    ",
"  .+++. .+++.   ",
"   .+++.+++.    ",
"    .+++++.     ",
"     .+++.      ",
"    .+++++.     ",
"   .+++.+++.    ",
"  .+++. .+++.   ",
"   .+.   .+.    ",
"    .     .     ",
"                ",
"                ",
"                "};

QString salomeVersion()
{
  return GUI_VERSION_STR;
}

class SALOME_ResourceMgr : public SUIT_ResourceMgr
{
public:
  SALOME_ResourceMgr( const QString& app, const QString& resVarTemplate ) : SUIT_ResourceMgr( app, resVarTemplate )
  {
    setCurrentFormat( "xml" );
    setOption( "translators", QString( "%P_msg_%L.qm|%P_icons.qm|%P_images.qm" ) );
    setDefaultPixmap( QPixmap( pixmap_not_found_xpm ) );
  }
  static void initResourceMgr()
  {
    if ( myExtAppName.isNull() || myExtAppVersion.isNull() ) {
      SALOME_ResourceMgr resMgr( "SalomeApp", QString( "%1Config" ) );
      resMgr.loadLanguage( "LightApp",  "en" );
      resMgr.loadLanguage( "SalomeApp", "en" );

      myExtAppName = QObject::tr( "APP_NAME" ).trimmed();
      if ( myExtAppName == "APP_NAME" || myExtAppName.toLower() == "salome" ) 
        myExtAppName = "SalomeApp";
      myExtAppVersion = QObject::tr( "APP_VERSION" );
      if ( myExtAppVersion == "APP_VERSION" ) {
        if ( myExtAppName != "SalomeApp" )
          myExtAppVersion = "";
        else myExtAppVersion = salomeVersion();
      }
    }
  }
  QString version() const { return myExtAppVersion; }

protected:
  QString userFileName( const QString& appName, const bool for_load ) const
  { 
    if ( version().isEmpty()  ) return ""; 
    return SUIT_ResourceMgr::userFileName( myExtAppName, for_load );
  }

  virtual long userFileId( const QString& _fname ) const
  {
    //////////////////////////////////////////////////////////////////////////////////////////////
    // In SALOME and SALOME-based applications the user preferences file is named as
    // - <AppName>.xml.<AppVersion> on Windows
    // - <AppName>rc.<AppVersion> on Linux
    // where
    //   * AppName is application name, default SalomeApp (can be customized in SALOME-based
    //     applications
    //   * AppVersion is application version
    //
    // Since version 6.5.0 of SALOME, user file is situated in the ~/.config/salome
    // directory. For backward compatibility, when user preferences from nearest
    // version of application is searched, user home directory is also looked through,
    // with lower priority.
    // 
    // Since version 6.6.0 of SALOME, user file name on Linux is no more prefixed by dot
    // symbol since it is situated in hidden ~/.config/salome directory. Files with dot
    // prefix also though taken into account (with lower priority) for backward compatibility.
    //
    // Notes:
    // - Currently the following format of version number is supported:
    //   <major>[.<minor>[.<release>[<type><dev>]]]
    //   Parts in square brackets are considered optional. Here:
    //   * major   - major version id
    //   * minor   - minor version id
    //   * release - maintenance version id
    //   * type    - dev or patch marker; it can be either one alphabetical symbol (from 'a' to 'z')
    //               or 'rc' to point release candidate (case-insensitive)
    //   * dev     - dev version or patch number
    //   All numerical values must be of range [1-99].
    //   Examples: 1.0, 6.5.0, 1.2.0a1, 3.3.3rc3 (release candidate 3), 11.0.0p1 (patch 1)
    //
    // - Versioning approach can be customized by implementing and using own resource manager class,
    //   see QtxResurceMgr, SUIT_ResourceMgr classes.
    //////////////////////////////////////////////////////////////////////////////////////////////
    long id = -1;
    if ( !myExtAppName.isEmpty() ) {
#ifdef WIN32
      // On Windows, user file name is something like SalomeApp.xml.6.5.0 where
      // - SalomeApp is an application name (can be customized)
      // - xml is a file format (xml or ini)
      // - 6.5.0 is an application version, can include alfa/beta/rc marks, e.g. 6.5.0a3, 6.5.0rc1
      QRegExp exp( QString( "%1\\.%2\\.([a-zA-Z0-9.]+)" ).arg( myExtAppName ).arg( currentFormat() ) );
#else
      // On Linux, user file name is something like SalomeApprc.6.5.0 where
      // - SalomeApp is an application name (can be customized)
      // - 6.5.0 is an application version, can include alfa/beta/rc marks, e.g. 6.5.0a3, 6.5.0rc1

      // VSR 24/09/2012: issue 0021781: since version 6.6.0 user filename is not prepended with "."
      // when it is stored in the ~/.config/<appname> directory;
      // for backward compatibility we also check files prepended with "." with lower priority
      QRegExp exp( QString( "\\.?%1rc\\.([a-zA-Z0-9.]+)" ).arg( myExtAppName ) );
#endif
      QRegExp vers_exp( "^([0-9]+)([A-Z]|RC)?([0-9]*)", Qt::CaseInsensitive );
      
      QString fname = QFileInfo( _fname ).fileName();
      if( exp.exactMatch( fname ) ) {
        QStringList vers = exp.cap( 1 ).split( ".", QString::SkipEmptyParts );
        int major=0, minor=0;
        int release = 0, dev1 = 0, dev2 = 0;
	if ( vers.count() > 0 ) major = vers[0].toInt();
	if ( vers.count() > 1 ) minor = vers[1].toInt();
	if ( vers.count() > 2 ) {
	  if ( vers_exp.indexIn( vers[2] ) != -1 ) {
	    release = vers_exp.cap( 1 ).toInt();
	    QString tag = vers_exp.cap( 2 ).toLower();
	    if ( !tag.isEmpty() ) {
	      if ( tag == "rc" ) // release candidate
		dev1 = 49;       // 'rc'=49
	      else               // a, b, c, ... 
		dev1 = (int)( tag[ 0 ].toLatin1() ) - (int)( QChar('a').toLatin1() ) + 1; // 'a'=1, 'b'=2, ..., 'z'=26
	    }
	    if ( !vers_exp.cap( 3 ).isEmpty() )
	      dev2 = vers_exp.cap( 3 ).toInt();
	  }
	}
        
        int dev = dev1*100+dev2;
	id = major;
        id*=100; id+=minor;
        id*=100; id+=release;
        id*=10000;
        if ( dev > 0 ) id-=dev;
      }
    }
    return id;
  }

public:
  static QString myExtAppName;
  static QString myExtAppVersion;
};

QString SALOME_ResourceMgr::myExtAppName    = QString();
QString SALOME_ResourceMgr::myExtAppVersion = QString();

class SALOME_Session : public SUIT_Session
{
public:
  SALOME_Session() : SUIT_Session() {}
  virtual ~SALOME_Session() {}

protected:
  virtual SUIT_ResourceMgr* createResourceMgr( const QString& appName ) const
  {
    SALOME_ResourceMgr::initResourceMgr();
    SALOME_ResourceMgr* resMgr = new SALOME_ResourceMgr( appName, QString( "%1Config" ) );
    return resMgr;
  }
};

#ifdef ENABLE_TESTRECORDER
  class SALOME_QApplication : public TestApplication
#else
  class SALOME_QApplication : public QApplication
#endif
{
public:
#ifdef ENABLE_TESTRECORDER
  SALOME_QApplication( int& argc, char** argv ) : TestApplication( argc, argv ), myHandler ( 0 ) {}
#else
  SALOME_QApplication( int& argc, char** argv )
#ifndef WIN32
  // san: Opening an X display and choosing a visual most suitable for 3D visualization
  // in order to make SALOME viewers work with non-native X servers
  : QApplication( (Display*)Qtx::getDisplay(), argc, argv, Qtx::getVisual() ),
#else
  : QApplication( argc, argv ), 
#endif
    myHandler ( 0 ) {}
#endif

  virtual bool notify( QObject* receiver, QEvent* e )
  {
#if OCC_VERSION_LARGE < 0x06010100
    // Disable GUI user actions while python command is executed
    if (SUIT_Session::IsPythonExecuted()) {
      // Disable mouse and keyboard events
      QEvent::Type aType = e->type();
      if (aType == QEvent::MouseButtonPress || aType == QEvent::MouseButtonRelease ||
          aType == QEvent::MouseButtonDblClick || aType == QEvent::MouseMove ||
          aType == QEvent::Wheel || aType == QEvent::ContextMenu ||
          aType == QEvent::KeyPress || aType == QEvent::KeyRelease ||
          aType == QEvent::Accel || aType == QEvent::AccelOverride)
        return false;
    }
#endif

#ifdef ENABLE_TESTRECORDER
    return myHandler ? myHandler->handle( receiver, e ) :
      TestApplication::notify( receiver, e );
#else
    try {
      return myHandler ? myHandler->handle( receiver, e ) : QApplication::notify( receiver, e );
    }
    catch (std::exception& e) {
      std::cerr << e.what()  << std::endl;
    }
    catch (CORBA::Exception& e) {
      std::cerr << "Caught CORBA::Exception"  << std::endl;
      CORBA::Any tmp;
      tmp<<= e;
      CORBA::TypeCode_var tc = tmp.type();
      const char *p = tc->name();
      std::cerr << "notify(): CORBA exception of the kind : " << p << " is caught" << std::endl;
    }
    catch (...) {
      std::cerr << "Unknown exception caught in Qt handler: it's probably a bug in SALOME platform" << std::endl;
    }
    return false;  // return false when exception is caught
#endif
  }
  SUIT_ExceptionHandler* handler() const { return myHandler; }
  void setHandler( SUIT_ExceptionHandler* h ) { myHandler = h; }

private:
  SUIT_ExceptionHandler* myHandler;
};

// class which calls SALOME::Session::GetInterface() from another thread
// to avoid mutual lock ( if called from the same thread as main()
class GetInterfaceThread : public QThread
{
public:
  GetInterfaceThread( SALOME::Session_var s ) : session ( s )
  {
    start();
  }
protected:
  virtual void run()
  {
    if ( !CORBA::is_nil( session ) )
      session->GetInterface();
    else
      printf( "\nFATAL ERROR: SALOME::Session object is nil! Can not display GUI\n\n" );
  }
private:
  SALOME::Session_var session;
};

// returns true if 'str' is found in argv
bool isFound( const char* str, int argc, char** argv )
{
  for ( int i = 1; i <= ( argc-1 ); i++ )
    if ( !strcmp( argv[i], str ) )
      return true;
  return false;
}

void killOmniNames()
{
  SALOME_LifeCycleCORBA::killOmniNames();
}

// shutdown standalone servers
void shutdownServers( SALOME_NamingService* theNS )
{
  SALOME_LifeCycleCORBA lcc(theNS);
  lcc.shutdownServers();
}

// ---------------------------- MAIN -----------------------
int main( int argc, char **argv )
{
  // Install Qt debug messages handler
  qInstallMsgHandler( MessageOutput );
  
  // add $QTDIR/plugins to the pluins search path for image plugins
  QString qtdir( ::getenv( "QTDIR" ) );
  if ( !qtdir.isEmpty() )
    QApplication::addLibraryPath( QDir( qtdir ).absoluteFilePath( "plugins" ) );

  // Create Qt application instance;
  // this should be done the very first!
  SALOME_QApplication _qappl( argc, argv );
  _qappl.setOrganizationName( "salome" );
  _qappl.setApplicationName( "salome" );
  _qappl.setApplicationVersion( salomeVersion() );

  // Add application library path (to search style plugin etc...)
  QString path = QDir::convertSeparators( SUIT_Tools::addSlash( QString( ::getenv( "GUI_ROOT_DIR" ) ) ) + QString( "bin/salome" ) );
  _qappl.addLibraryPath( path );

  bool isGUI    = isFound( "GUI",    argc, argv );
  bool isSplash = isFound( "SPLASH", argc, argv );
  // Show splash screen (only if both the "GUI" and "SPLASH" parameters are set)
  // Note, that user preferences are not taken into account for splash settings -
  // it is a property of the application!
  QtxSplash* splash = 0;
  if ( isGUI && isSplash ) {
    // ...create resource manager
    SUIT_ResourceMgr resMgr( "SalomeApp", QString( "%1Config" ) );
    resMgr.setCurrentFormat( "xml" );
    resMgr.setWorkingMode( QtxResourceMgr::IgnoreUserValues );
    resMgr.loadLanguage( "LightApp" );
    //
    splash = QtxSplash::splash( QPixmap() );
    splash->readSettings( &resMgr );
    if ( splash->pixmap().isNull() )
      splash->setPixmap( resMgr.loadPixmap( "LightApp", QObject::tr( "ABOUT_SPLASH" ) ) );
    if ( splash->pixmap().isNull() ) {
      delete splash;
      splash = 0;
    }
    else {
      splash->setOption( "%A", QObject::tr( "APP_NAME" ) );
      splash->setOption( "%V", QObject::tr( "ABOUT_VERSION" ).arg( salomeVersion() ) );
      splash->setOption( "%L", QObject::tr( "ABOUT_LICENSE" ) );
      splash->setOption( "%C", QObject::tr( "ABOUT_COPYRIGHT" ) );
      splash->show();
      QApplication::instance()->processEvents();
    }
  }

  
  // Initialization
  int result = -1;

  CORBA::ORB_var orb;
  PortableServer::POA_var poa;

  SUIT_Session* aGUISession = 0;
  SALOME_NamingService* _NS = 0;
  GetInterfaceThread* guiThread = 0;
  Session_ServerLauncher* myServerLauncher = 0;

  try {
    // ...initialize Python (only once)
    int   _argc   = 1;
    char* _argv[] = {(char*)""};
    KERNEL_PYTHON::init_python( _argc,_argv );
    PyEval_RestoreThread( KERNEL_PYTHON::_gtstate );
    if ( !KERNEL_PYTHON::salome_shared_modules_module ) // import only once
      KERNEL_PYTHON::salome_shared_modules_module = PyImport_ImportModule( "salome_shared_modules" );
    if ( !KERNEL_PYTHON::salome_shared_modules_module ) {
      INFOS( "salome_shared_modules_module == NULL" );
      PyErr_Print();
    }
    PyEval_ReleaseThread( KERNEL_PYTHON::_gtstate );

    // ...create ORB, get RootPOA object, NamingService, etc.
    ORB_INIT &init = *SINGLETON_<ORB_INIT>::Instance();
    ASSERT( SINGLETON_<ORB_INIT>::IsAlreadyExisting() );
    int orbArgc = 1;
    orb = init( orbArgc, argv );

    CORBA::Object_var obj = orb->resolve_initial_references( "RootPOA" );
    poa = PortableServer::POA::_narrow( obj );

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate() ;
    MESSAGE( "pman->activate()" );

    _NS = new SALOME_NamingService( orb );

    result = 0;
  }
  catch ( SALOME_Exception& e ) {
    INFOS( "run(): SALOME::SALOME_Exception is caught: "<<e.what() );
  }
  catch ( CORBA::SystemException& e ) {
    INFOS( "Caught CORBA::SystemException." );
  }
  catch ( CORBA::Exception& e ) {
    INFOS( "Caught CORBA::Exception." );
    CORBA::Any tmp;
    tmp<<= e;
    CORBA::TypeCode_var tc = tmp.type();
    const char *p = tc->name();
    INFOS ( "run(): CORBA exception of the kind : "<<p<< " is caught" );
  }
  catch ( std::exception& e ) {
    INFOS( "run(): An exception has been caught: " <<e.what() );
  }
  catch (...) {
    INFOS( "Caught unknown exception." );
  }

  QMutex _GUIMutex, _SessionMutex, _SplashMutex;
  QWaitCondition _ServerLaunch, _SessionStarted, _SplashStarted;

  // lock session mutex to ensure that GetInterface is not called
  // until all initialization is done
  _SessionMutex.lock();

  if ( !result ) {
    // Start embedded servers launcher (Registry, SALOMEDS, etc.)
    // ...lock mutex to block embedded servers launching thread until wait( mutex )
    _GUIMutex.lock();  
    // ...create launcher
    myServerLauncher = new Session_ServerLauncher( argc, argv, orb, poa, &_GUIMutex, &_ServerLaunch, &_SessionMutex, &_SessionStarted );
    // ...block this thread until launcher is ready
    _ServerLaunch.wait( &_GUIMutex );
    
    // Start servers check thread (splash)
    if ( splash ) {
      // ...lock mutex to block splash thread until wait( mutex )
      _SplashMutex.lock();
      // ...create servers checking thread
      Session_ServerCheck sc( &_SplashMutex, &_SplashStarted );
      // ... set initial progress
      splash->setProgress( 0, sc.totalSteps() );
      // start check loop 
      while ( true ) {
        int step    = sc.currentStep();
        int total   = sc.totalSteps();
        QString msg = sc.currentMessage();
        QString err = sc.error();
        if ( !err.isEmpty() ) {
          QtxSplash::setError( err );
          QApplication::instance()->processEvents();
          result = -1;
          break;
        }
        QtxSplash::setStatus( msg, step );
        QApplication::instance()->processEvents();
        if ( step >= total )
          break;
        // ...block this thread until servers checking is finished
        _SplashStarted.wait( &_SplashMutex );
      }
      // ...unlock mutex 'cause it is no more needed
      _SplashMutex.unlock();
    }

    // Finalize embedded servers launcher 
    // ...block this thread until launcher is finished
    _ServerLaunch.wait( &_GUIMutex );
    // ...unlock mutex 'cause it is no more needed
    _GUIMutex.unlock();
  }

  // Obtain Session interface reference
  CORBA::Object_var obj = _NS->Resolve( "/Kernel/Session" );
  SALOME::Session_var session = SALOME::Session::_narrow( obj ) ;

  bool shutdownAll = false;
  bool shutdownSession = false;
  if ( !result ) {
    // Launch GUI activator
    if ( isGUI ) {
      if ( splash )
        splash->setStatus( QApplication::translate( "", "Activating desktop..." ) );
      // ...create GUI launcher
      MESSAGE( "Session activated, Launch IAPP..." );
      guiThread = new GetInterfaceThread( session );
    }

    // GUI activation
    // Allow multiple activation/deactivation of GUI
    while ( true ) {
      MESSAGE( "waiting wakeAll()" );
      _SessionStarted.wait( &_SessionMutex ); // to be reseased by Launch server thread when ready:
      // atomic operation lock - unlock on mutex
      // unlock mutex: serverThread runs, calls _ServerLaunch->wakeAll()
      // this thread wakes up, and lock mutex

      _SessionMutex.unlock();

      // Session might be shutdowning here, check status
      SALOME::StatSession stat = session->GetStatSession();
      shutdownSession = stat.state == SALOME::shutdown;
      if ( shutdownSession ) {
	_SessionMutex.lock(); // lock mutex before leaving loop - it will be unlocked later
	break;
      }

      // SUIT_Session creation
      aGUISession = new SALOME_Session();

      // Load SalomeApp dynamic library
      MESSAGE( "creation SUIT_Application" );
      SUIT_Application* aGUIApp = aGUISession->startApplication( "SalomeApp", 0, 0 );
      if ( aGUIApp )
      {
        Style_Salome::initialize( aGUIApp->resourceMgr() );
        if ( aGUIApp->resourceMgr()->booleanValue( "Style", "use_salome_style", true ) )
          Style_Salome::apply();

        if ( !isFound( "noexcepthandler", argc, argv ) )
          _qappl.setHandler( aGUISession->handler() ); // after loading SalomeApp application
                                                       // aGUISession contains SalomeApp_ExceptionHandler
        // Run GUI loop
        MESSAGE( "run(): starting the main event loop" );

        if ( splash )
          splash->finish( aGUIApp->desktop() );
	
        result = _qappl.exec();
        
        splash = 0;
	
        if ( result == SUIT_Session::NORMAL ) {
	  // desktop is explicitly closed by user from GUI
	  // exit flags says if it's necessary to shutdown all servers
	  // all session server only
          shutdownAll = aGUISession->exitFlags();
	}
	else {
	  // desktop might be closed from:
	  // - StopSesion() /temporarily/ or
	  // - Shutdown() /permanently/
	  stat = session->GetStatSession();
	  shutdownSession = stat.state == SALOME::shutdown;
	}
	if ( shutdownAll || shutdownSession ) {
	  _SessionMutex.lock(); // lock mutex before leaving loop - it will be unlocked later
	  break;
	}
      }

      delete aGUISession;
      aGUISession = 0;
      
      // Prepare _GUIMutex for a new GUI activation
      _SessionMutex.lock();
    }
  }

  // unlock Session mutex
  _SessionMutex.unlock();
  
  if ( shutdownAll )
    shutdownServers( _NS );

  if ( myServerLauncher )
    myServerLauncher->KillAll(); // kill embedded servers

  // Unregister session server
  SALOME_Session_i* sessionServant = dynamic_cast<SALOME_Session_i*>( poa->reference_to_servant( session.in() ) );
  if ( sessionServant )
    sessionServant->NSunregister();

  delete aGUISession;
  delete guiThread;
  delete myServerLauncher;
  delete _NS;

  try  {
    orb->shutdown(0);
  }
  catch (...) {
    //////////////////////////////////////////////////////////////
    // VSR: silently skip exception:
    // CORBA.BAD_INV_ORDER.BAD_INV_ORDER_ORBHasShutdown 
    // exception is raised when orb->destroy() is called and
    // cpp continer is launched in the embedded mode
    //////////////////////////////////////////////////////////////
    // std::cerr << "Caught unexpected exception on shutdown : ignored !!" << std::endl;
    if ( shutdownAll )
      killOmniNames();
    abort(); //abort program to avoid deadlock in destructors or atexit when shutdown has been interrupted
  }

  PyGILState_Ensure();
  //Destroy orb from python (for chasing memory leaks)
  //PyRun_SimpleString("from omniORB import CORBA");
  //PyRun_SimpleString("orb=CORBA.ORB_init([''], CORBA.ORB_ID)");
  //PyRun_SimpleString("orb.destroy()");
  Py_Finalize();

  if ( shutdownAll )
    killOmniNames();

  MESSAGE( "Salome_Session_Server:endofserver" );
  return result;
}
