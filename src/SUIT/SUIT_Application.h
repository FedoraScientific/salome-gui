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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
#ifndef SUIT_APPLICATION_H
#define SUIT_APPLICATION_H

#include "SUIT.h"
#include "SUIT_Study.h"

#include <qobject.h>
#include <qwidget.h>

class QAction;
class SUIT_Desktop;
class SUIT_Convertor;
class SUIT_ViewModel;
class SUIT_ResourceMgr;
class QString;
class QIconSet;
class QLabel;
/*! \class QObject
 * \brief For more information see <a href="http://doc.trolltech.com">QT documentation</a>.
 */
/*!
  An <b>Application</b> is a class which defines application configuration and behaviour.
  For example Application object defines what Viewers are used in this application, what auxilliary windows
  are present, how user can dial with them. Also Application object defines an sertain type of data structure by 
  holding of pointer on an instance of SUIT_Study class (which represents Document data structure). In other words
  Application defines type of sata structure, type of used Viewers, type of main GUI widget (Desktop),
  and other auxilliary tools.
*/

class SUIT_EXPORT SUIT_Application : public QObject
{
  Q_OBJECT

public:
  SUIT_Application();
  virtual ~SUIT_Application();

  //! Returns main widget (Desktop) of the application (if it exists)
  virtual SUIT_Desktop* desktop();

  /*! Returns FALSE if application can not be closed (because of non saved data for example). 
      This method called by SUIT_Session whin closing of application was requested. */
  virtual bool          isPossibleToClose();

  /*! Performs some finalization of life cycle of this application.
      For instance, the application can force its documents(s) to close. */
  virtual void          closeApplication();

  //! Returns active Study. If Application supports wirking with several studies this method should be redefined
  virtual SUIT_Study*   activeStudy() const;

  //! Returns Name of application. Using is not defined.
  virtual QString       applicationName() const = 0;

  virtual QString       applicationVersion() const;

  //! Shows the application's main widget. For non GUI application must be redefined.
  virtual void          start();

  //! Opens document <theFileName> into active Study. If Study is empty - creates it.
  virtual bool          useFile( const QString& theFileName);

  //! Loads document <theName> into active Study. If Study is empty - creates it.
  virtual bool          useStudy( const QString& theName);

  //! Creates new empty Study if active Study = 0
  virtual void          createEmptyStudy();

  /*! Returns number of Studies. 
   *  Must be redefined in Applications which support several studies for one Application instance. */
  virtual int           getNbStudies() const;

  SUIT_ResourceMgr*     resourceMgr() const;

  /*! Returns instance of data object Convertor class according to given Viewer. 
      If convertation is not supported returns 0. */
  virtual SUIT_Convertor* getConvertor(const SUIT_ViewModel* theViewer) { return 0; }

  //! Puts the message to the status bar  
  void putInfo ( const QString&, const int = 0 );

  //! Invokes application-specific "Open/Save File" dialog and returns the selected file name.
  virtual QString getFileName( bool open, const QString& initial, const QString& filters, 
			       const QString& caption, QWidget* parent ) = 0;

  //! Invokes application-specific "Select Directory" dialog and returns the selected directory name.
  virtual QString getDirectory( const QString& initial, const QString& caption, QWidget* parent ) = 0;

signals:
  void                  applicationClosed( SUIT_Application* );
  void                  activated( SUIT_Application* );

protected:
  SUIT_Application*     startApplication( int, char** ) const;
  SUIT_Application*     startApplication( const QString&, int, char** ) const;

  virtual void          setDesktop( SUIT_Desktop* );

  //! Creates a new Study instance. Must be redefined in new application according to its Study type.
  virtual SUIT_Study*   createNewStudy();
  virtual void          setActiveStudy( SUIT_Study* );
  
  /** @name Create tool functions*/ //@{
  int                   createTool( const QString& );
  int                   createTool( const int, const int, const int = -1 );
  int                   createTool( const int, const QString&, const int = -1 );
  int                   createTool( QAction*, const int, const int = -1, const int = -1 );
  int                   createTool( QAction*, const QString&, const int = -1, const int = -1 );//@}

  /** @name Create menu functions*/ //@{
  int                   createMenu( const QString&, const int, const int = -1, const int = -1, const int = -1 );
  int                   createMenu( const QString&, const QString&, const int = -1, const int = -1, const int = -1 );
  int                   createMenu( const int, const int, const int = -1, const int = -1 );
  int                   createMenu( const int, const QString&, const int = -1, const int = -1 );
  int                   createMenu( QAction*, const int, const int = -1, const int = -1, const int = -1 );
  int                   createMenu( QAction*, const QString&, const int = -1, const int = -1, const int = -1 );//@}

  /** @name Set menu shown functions*/ //@{
  void                  setMenuShown( QAction*, const bool );
  void                  setMenuShown( const int, const bool );//@}
  /** @name Set tool shown functions*/ //@{
  void                  setToolShown( QAction*, const bool );
  void                  setToolShown( const int, const bool );//@}

  void                  setActionShown( QAction*, const bool );
  void                  setActionShown( const int, const bool );

  static QAction*       separator();
  QAction*              action( const int ) const;
  int                   actionId( const QAction* ) const;
  int                   registerAction( const int, QAction* );
  QAction*              createAction( const int, const QString&, const QIconSet&, const QString&,
                                      const QString&, const int, QObject* = 0,
                                      const bool = false, QObject* = 0, const char* = 0 );

protected slots:
  virtual void          onDesktopActivated();

private:
  SUIT_Study*           myStudy;
  SUIT_Desktop*         myDesktop;
  QMap<int, QAction*>   myActionMap;

  QLabel*               myStatusLabel;
};

//! This function must return a new application instance.
extern "C"
{
  //jfa 22.06.2005:typedef SUIT_Application* (*APP_CREATE_FUNC)( int, char** );
  typedef SUIT_Application* (*APP_CREATE_FUNC)();
}

#define APP_CREATE_NAME "createApplication"

#endif
