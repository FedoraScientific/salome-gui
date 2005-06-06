// File:      QtxDockWindow.cxx
// Author:    Sergey TELKOV

#include "QtxDockWindow.h"

#include <qlayout.h>
#include <qpixmap.h>
#include <qdockarea.h>
#include <qmainwindow.h>
#include <qapplication.h>

/*!
    Class: QtxDockWindow::Watcher [Internal]
    Descr: Internal object with event filter.
*/

class QtxDockWindow::Watcher : public QObject
{
public:
  Watcher( QtxDockWindow* );

  void           shown( QtxDockWindow* );
  void           hided( QtxDockWindow* );

  virtual bool   eventFilter( QObject*, QEvent* );

protected:
  virtual void   customEvent( QCustomEvent* );

private:
  void           installFilters();

  void           showContainer();
  void           hideContainer();

  void           updateIcon();
  void           updateCaption();
  void           updateVisibility();

private:
  QtxDockWindow* myCont;
  bool           myState;
  bool           myEmpty;
  bool           myVisible;
};

QtxDockWindow::Watcher::Watcher( QtxDockWindow* cont )
: QObject( cont ), myCont( cont ),
myState( true ),
myEmpty( true )
{
  if ( myCont->mainWindow() )
    myState = myCont->mainWindow()->appropriate( myCont );

  myCont->installEventFilter( this );
  myVisible = myCont->isVisibleTo( myCont->parentWidget() );

  installFilters();
}

bool QtxDockWindow::Watcher::eventFilter( QObject* o, QEvent* e )
{
  if ( o == myCont &&
       ( e->type() == QEvent::Show || e->type() == QEvent::ShowToParent ||
         e->type() == QEvent::Hide || e->type() == QEvent::HideToParent ||
         e->type() == QEvent::ChildInserted ) )
    QApplication::postEvent( this, new QCustomEvent( QEvent::User ) );

  if ( o != myCont && e->type() == QEvent::IconChange )
    updateIcon();

  if ( o != myCont && e->type() == QEvent::CaptionChange )
    updateCaption();

  if ( o != myCont &&
       ( e->type() == QEvent::Show || e->type() == QEvent::ShowToParent ||
         e->type() == QEvent::Hide || e->type() == QEvent::HideToParent ||
         e->type() == QEvent::ChildRemoved ) )
    updateVisibility();

  return false;
}

void QtxDockWindow::Watcher::shown( QtxDockWindow* dw )
{
  if ( dw != myCont )
    return;

  myVisible = true;
}

void QtxDockWindow::Watcher::hided( QtxDockWindow* dw )
{
  if ( dw != myCont )
    return;

  myVisible = false;
}

void QtxDockWindow::Watcher::showContainer()
{
  if ( !myCont )
    return;

  QtxDockWindow* cont = myCont;
  myCont = 0;
  cont->show();
  myCont = cont;
}

void QtxDockWindow::Watcher::hideContainer()
{
  if ( !myCont )
    return;

  QtxDockWindow* cont = myCont;
  myCont = 0;
  cont->hide();
  myCont = cont;
}

void QtxDockWindow::Watcher::customEvent( QCustomEvent* e )
{
  installFilters();

  updateIcon();
  updateCaption();
  updateVisibility();
}

void QtxDockWindow::Watcher::installFilters()
{
  if ( !myCont )
    return;

  QBoxLayout* bl = myCont->boxLayout();
  if ( !bl )
    return;

  for ( QLayoutIterator it = bl->iterator(); it.current(); ++it )
  {
    if ( it.current()->widget() )
      it.current()->widget()->installEventFilter( this );
  }
}

void QtxDockWindow::Watcher::updateVisibility()
{
  if ( !myCont )
    return;

  QBoxLayout* bl = myCont->boxLayout();
  if ( !bl )
    return;

  bool vis = false;
  for ( QLayoutIterator it = bl->iterator(); it.current() && !vis; ++it )
    vis = it.current()->widget() && it.current()->widget()->isVisibleTo( myCont );

  QMainWindow* mw = myCont->mainWindow();
  if ( mw && myEmpty == vis )
  {
    myEmpty = !vis;
    if ( !myEmpty )
      mw->setAppropriate( myCont, myState );
    else
    {
      myState = mw->appropriate( myCont );
      mw->setAppropriate( myCont, false );
    }
  }

  vis = !myEmpty && myVisible;
  if ( vis != myCont->isVisibleTo( myCont->parentWidget() ) )
    vis ? showContainer() : hideContainer();
}

void QtxDockWindow::Watcher::updateIcon()
{
  if ( !myCont || !myCont->widget() )
    return;
  
  const QPixmap* ico = myCont->widget()->icon();
  myCont->setIcon( ico ? *ico : QPixmap() );
}

void QtxDockWindow::Watcher::updateCaption()
{
  if ( myCont && myCont->widget() && !myCont->widget()->caption().isNull() )
    myCont->setCaption( myCont->widget()->caption() );
}

/*!
    Class: QtxDockWindow [Public]
    Descr: 
*/

QtxDockWindow::QtxDockWindow( Place p, QWidget* parent, const char* name, WFlags f )
: QDockWindow( p, parent, name, f ),
myWatcher( 0 ),
myStretch( false )
{
}

QtxDockWindow::QtxDockWindow( const bool watch, QWidget* parent, const char* name, WFlags f )
: QDockWindow( InDock, parent, name, f ),
myWatcher( 0 ),
myStretch( false )
{
  if ( watch )
    myWatcher = new Watcher( this );
}

QtxDockWindow::QtxDockWindow( QWidget* parent, const char* name, WFlags f )
: QDockWindow( InDock, parent, name, f ),
myWatcher( 0 ),
myStretch( false )
{
}

QtxDockWindow::~QtxDockWindow()
{
}

void QtxDockWindow::setWidget( QWidget* wid )
{
  if ( wid )
    wid->reparent( this, QPoint( 0, 0 ), wid->isVisibleTo( wid->parentWidget() ) );

  QDockWindow::setWidget( wid );
}

bool QtxDockWindow::isStretchable() const
{
  return myStretch;
}

void QtxDockWindow::setStretchable( const bool on )
{
  if ( myStretch == on )
    return;

  myStretch = on;

  boxLayout()->setStretchFactor( widget(), myStretch ? 1 : 0 );

  if ( myStretch != isHorizontalStretchable() ||
       myStretch != isVerticalStretchable() )
  {
	  if ( orientation() == Horizontal )
	    setHorizontalStretchable( myStretch );
	  else
	    setVerticalStretchable( myStretch );
  }
}

QSize QtxDockWindow::sizeHint() const
{
  QSize sz = QDockWindow::sizeHint();

  if ( place() == InDock && isStretchable() && area() )
  {
    if ( orientation() == Horizontal )
      sz.setWidth( area()->width() );
    else
      sz.setHeight( area()->height() );
  }

  return sz;
}

QSize QtxDockWindow::minimumSizeHint() const
{
  QSize sz = QDockWindow::minimumSizeHint();

  if ( orientation() == Horizontal )
	  sz = QSize( 0, QDockWindow::minimumSizeHint().height() );
  else
    sz = QSize( QDockWindow::minimumSizeHint().width(), 0 );

  if ( place() == InDock && isStretchable() && area() )
  {
    if ( orientation() == Horizontal )
      sz.setWidth( area()->width() );
    else
      sz.setHeight( area()->height() );
  }

  return sz;
}

QMainWindow* QtxDockWindow::mainWindow() const
{
  QMainWindow* mw = 0;

  QWidget* wid = parentWidget();
  while ( !mw && wid )
  {
    if ( wid->inherits( "QMainWindow" ) )
      mw = (QMainWindow*)wid;
    wid = wid->parentWidget();
  }

  return mw;
}

void QtxDockWindow::show()
{
  if ( myWatcher )
    myWatcher->shown( this );

  QDockWindow::show();
}

void QtxDockWindow::hide()
{
  if ( myWatcher )
    myWatcher->hided( this );

  QDockWindow::hide();
}
