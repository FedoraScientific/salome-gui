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

#include "OCCViewer_ViewSketcher.h"
#include "OCCViewer_ViewWindow.h"
#include "OCCViewer_ViewPort3d.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qpointarray.h>

/****************************************************************
**  Class: OCCViewer_ViewSketcher
**  Level: Public
*****************************************************************/

OCCViewer_ViewSketcher::OCCViewer_ViewSketcher( OCCViewer_ViewWindow* vw, int type )
: QObject( vw ),
mySketchButton( Qt::LeftButton ),
mypViewWindow( vw ),
myType( type ),
mypData( 0 ),
myResult( Neutral ),
myButtonState( 0 )
{
}

OCCViewer_ViewSketcher::~OCCViewer_ViewSketcher()
{
}

void OCCViewer_ViewSketcher::activate()
{
  OCCViewer_ViewPort3d* avp = mypViewWindow->getViewPort();

  mySavedCursor = avp->cursor();
  avp->setCursor( Qt::PointingHandCursor );
  avp->installEventFilter( this );
  qApp->installEventFilter( this );

  connect( avp, SIGNAL( vpDrawExternal( QPainter* ) ), this, SLOT( onDrawViewPort() ) );

  myStart = QPoint();
  myResult = Neutral;

  onActivate();
}

void OCCViewer_ViewSketcher::deactivate()
{
  OCCViewer_ViewPort3d* avp = mypViewWindow->getViewPort();

  disconnect( avp, SIGNAL( vpDrawExternal( QPainter* ) ), this, SLOT( onDrawViewPort() ) );

  qApp->removeEventFilter( this );
  avp->removeEventFilter( this );
  avp->setCursor( mySavedCursor );

  onDeactivate();
}

int OCCViewer_ViewSketcher::type() const
{
  return myType;
}

void* OCCViewer_ViewSketcher::data() const
{
  return mypData;
}

int OCCViewer_ViewSketcher::result() const
{
  return myResult;
}

int OCCViewer_ViewSketcher::buttonState() const
{
  return myButtonState;
}

void OCCViewer_ViewSketcher::onActivate()
{
}

void OCCViewer_ViewSketcher::onDeactivate()
{
}

bool OCCViewer_ViewSketcher::isDefault() const
{
  return true;
}

bool OCCViewer_ViewSketcher::eventFilter( QObject* o, QEvent* e )
{
  OCCViewer_ViewPort3d* avp = mypViewWindow->getViewPort();

  SketchState state = EnTrain;
  bool ignore = false;
  if ( o == avp )
  {
    switch ( e->type() )
    {
      case QEvent::MouseMove:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      {
        QMouseEvent* me = (QMouseEvent*)e;

        myButtonState = me->state();
        if ( e->type() == QEvent::MouseButtonPress )
          myButtonState |= me->button();

        if ( myStart.isNull() && ( myButtonState & sketchButton() ) )
        {
          state = Debut;
          myStart = me->pos();
        }

        myCurr = me->pos();

        onMouse( me );

        if ( myResult != Neutral )
          state = Fin;

        ignore = true;
        break;
      }
      case QEvent::Hide:
      case QEvent::HideToParent:
        myResult = Reject;
        onSketch( Fin );
        break;
      default:
        break;
    }
  }
  if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease )
  {
    ignore = onKey( (QKeyEvent*)e );
    if ( myResult != Neutral )
      state = Fin;
  }

  if ( ignore )
  {
    onSketch( state );
    return true;
  }
  return QObject::eventFilter( o, e );
}

void OCCViewer_ViewSketcher::onDrawViewPort()
{
  onSketch( Debut );
}

bool OCCViewer_ViewSketcher::onKey( QKeyEvent* )
{
  return false;
}

void OCCViewer_ViewSketcher::onMouse( QMouseEvent* )
{
}

int OCCViewer_ViewSketcher::sketchButton()
{
  return mySketchButton;
}

void OCCViewer_ViewSketcher::setSketchButton( int b )
{
  mySketchButton = b;
}

/****************************************************************
**  Class: OCCViewer_RectSketcher
**  Level: Public
*****************************************************************/

OCCViewer_RectSketcher::OCCViewer_RectSketcher( OCCViewer_ViewWindow* vw, int typ )
: OCCViewer_ViewSketcher( vw, typ )
{
}

OCCViewer_RectSketcher::~OCCViewer_RectSketcher()
{
  delete mypData;
}

void OCCViewer_RectSketcher::onActivate()
{
  mypData = new QRect();
}

void OCCViewer_RectSketcher::onDeactivate()
{
  delete mypData;
  mypData = 0;
}

bool OCCViewer_RectSketcher::onKey( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Escape )
    myResult = Reject;
  else if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return )
    myResult = Accept;

  return true;
}

void OCCViewer_RectSketcher::onMouse( QMouseEvent* e )
{
  OCCViewer_ViewPort3d* avp = mypViewWindow->getViewPort();

  if ( avp->rect().contains( myCurr ) )
    avp->setCursor( Qt::PointingHandCursor );
  else
    avp->setCursor( Qt::ForbiddenCursor );

  if ( e->type() == QEvent::MouseButtonRelease && e->button() == sketchButton() )
  {
    myResult = Accept;
    QApplication::postEvent( avp, new QMouseEvent( e->type(), e->pos(),
                                                   e->globalPos(), e->state(), e->button() ) );
  }
}

void OCCViewer_RectSketcher::onSketch( SketchState state )
{
  OCCViewer_ViewPort3d* avp = mypViewWindow->getViewPort();

  QRect* sketchRect = (QRect*)data();
  if ( myButtonState & sketchButton() )
  {
    QRect rect( QMIN( myStart.x(), myCurr.x() ), QMIN( myStart.y(), myCurr.y() ),
                QABS( myStart.x() - myCurr.x() ), QABS( myStart.y() - myCurr.y() ) );
    QPainter p( avp );
    p.setPen( Qt::white );
    p.setRasterOp( Qt::XorROP );
    if ( state != Debut && !sketchRect->isEmpty() )
      p.drawRect( *sketchRect );
    *sketchRect = rect;
    if ( !rect.isEmpty() && state != Fin )
      p.drawRect( *sketchRect );
  }

  if ( state == Fin )
  {
    QApplication::syncX();  /* force rectangle redrawing */
    mypViewWindow->activateSketching( OCCViewer_ViewWindow::NoSketching );
  }
}

/****************************************************************
**  Class: OCCViewer_PolygonSketcher
**  Level: Public
*****************************************************************/

OCCViewer_PolygonSketcher::OCCViewer_PolygonSketcher( OCCViewer_ViewWindow* vw, int typ )
: OCCViewer_ViewSketcher( vw, typ ),
  myDbl           ( false ),
  myToler         ( 5, 5 ),
  mypPoints        ( 0L ),
  myAddButton     ( 0 ),
  myDelButton     ( 0 )
{
  mySketchButton = Qt::RightButton;
}

OCCViewer_PolygonSketcher::~OCCViewer_PolygonSketcher()
{
  delete mypPoints;
  delete mypData;
}

void OCCViewer_PolygonSketcher::onActivate()
{
  myDbl = false;
  mypData = new QPointArray( 0 );
  mypPoints = new QPointArray( 0 );

  switch ( sketchButton() )
  {
  case Qt::LeftButton:
    myAddButton = Qt::RightButton;
    myDelButton = Qt::MidButton;
    break;
  case Qt::MidButton:
    myAddButton = Qt::LeftButton;
    myDelButton = Qt::RightButton;
    break;
  case Qt::RightButton:
  default:
    myAddButton = Qt::LeftButton;
    myDelButton = Qt::MidButton;
    break;
  };
}

void OCCViewer_PolygonSketcher::onDeactivate()
{
  delete mypPoints;
  mypPoints = 0;
  delete mypData;
  mypData = 0;
}

bool OCCViewer_PolygonSketcher::onKey( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    myResult = Reject;
    return true;
  }
  else if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return )
  {
    QPointArray* points = (QPointArray*)data();
    if ( points->count() )
    {
      QPoint last = points->point( points->count() - 1 );
      if ( last != myCurr )
      {
        points->resize( points->count() + 1 );
        points->setPoint( points->count() - 1, myCurr );
      }
    }
    myResult = Accept;
    return true;
  }
  else if ( e->key() == Qt::Key_Backspace && e->type() == QEvent::KeyRelease )
  {
    QPointArray* points = (QPointArray*)data();
    if ( points->count() > 1 )
      points->resize( points->count() - 1 );
    onMouse( 0 );
    return true;
  }

  return true;
}

void OCCViewer_PolygonSketcher::onMouse( QMouseEvent* e )
{
  OCCViewer_ViewPort3d* avp = mypViewWindow->getViewPort();

  QPointArray* points = (QPointArray*)data();
  if ( !points->count() && !myStart.isNull() )
  {
    points->resize( points->count() + 1 );
    points->setPoint( points->count() - 1, myStart );
  }

  bool closed = false;
  bool valid = avp->rect().contains( myCurr );
  if ( !myStart.isNull() )
  {
    QRect aRect( myStart.x() - myToler.width(), myStart.y() - myToler.height(),
                 2 * myToler.width(), 2 * myToler.height() );
    closed = aRect.contains( myCurr );
  }
  valid = valid && isValid( points, myCurr );
  if ( closed && !valid )
    closed = false;

  if ( closed )
    avp->setCursor( Qt::CrossCursor );
  else if ( valid )
    avp->setCursor( Qt::PointingHandCursor );
  else
    avp->setCursor( Qt::ForbiddenCursor );

  if ( !e )
    return;

  if ( e->type() == QEvent::MouseButtonRelease && ( e->button() & sketchButton() ) )
  {
    myResult = Reject;
    QApplication::postEvent( avp, new QMouseEvent( e->type(), e->pos(),
                                                   e->globalPos(), e->state(), e->button() ) );
  }
  else if ( e->type() == QEvent::MouseButtonRelease && ( e->button() & myAddButton ) )
  {
    if ( closed )
      myResult = Accept;
    else
    {
      if ( myStart.isNull() )
        myStart = myCurr;
      else
      {
        QPoint last = points->point( points->count() - 1 );
        if ( last != myCurr && valid )
        {
          points->resize( points->count() + 1 );
          points->setPoint( points->count() - 1, myCurr );
        }
        if ( valid && myDbl )
          myResult = Accept;
      }
    }
  }
  else if ( ( e->type() == QEvent::MouseButtonRelease && ( e->button() & myDelButton ) ) ||
            ( e->type() == QEvent::MouseButtonDblClick && ( e->button() & myDelButton ) ) )
  {
    if ( points->count() > 1 )
      points->resize( points->count() - 1 );
    onMouse( 0 );
  }
  myDbl = e->type() == QEvent::MouseButtonDblClick && ( e->button() & myAddButton );
}

void OCCViewer_PolygonSketcher::onSketch( SketchState state )
{
  OCCViewer_ViewPort3d* avp = mypViewWindow->getViewPort();

  QPointArray* points = (QPointArray*)data();
  QPainter p( avp );
  p.setPen( Qt::white );
  p.setRasterOp( Qt::XorROP );
  if ( state != Debut )
    p.drawPolyline( *mypPoints );

  if ( points->count() )
  {
    mypPoints->resize( points->count() + 1 );
    for ( uint i = 0; i < points->count(); i++ )
      mypPoints->setPoint( i, points->point( i ) );
    mypPoints->setPoint( points->count(), myCurr );
    if ( state != Fin )
      p.drawPolyline( *mypPoints );
  }

  if ( state == Fin )
  {
    QApplication::syncX();
    mypViewWindow->activateSketching( OCCViewer_ViewWindow::NoSketching );
  }
}

bool OCCViewer_PolygonSketcher::isValid( const QPointArray* aPoints, const QPoint& aCur ) const
{
  if ( !aPoints->count() )
    return true;

  if ( aPoints->count() == 1 && aPoints->point( 0 ) == aCur )
    return false;

  const QPoint& aLast = aPoints->point( aPoints->count() - 1 );

  if ( aLast == aCur )
    return true;

  bool res = true;
  for ( uint i = 0; i < aPoints->count() - 1 && res; i++ )
  {
    const QPoint& aStart = aPoints->point( i );
    const QPoint& anEnd  = aPoints->point( i + 1 );
    res = !isIntersect( aStart, anEnd, aCur, aLast );
  }

  return res;
}

bool OCCViewer_PolygonSketcher::isIntersect( const QPoint& aStart1, const QPoint& anEnd1,
					     const QPoint& aStart2, const QPoint& anEnd2 ) const
{
  if ( ( aStart1 == aStart2 && anEnd1 == anEnd2 ) ||
       ( aStart1 == anEnd2 && anEnd1 == aStart2 ) )
    return true;

  if ( aStart1 == aStart2 || aStart2 == anEnd1 ||
       aStart1 == anEnd2 || anEnd1 == anEnd2 )
    return false;

  double x11 = aStart1.x() * 1.0;
  double x12 = anEnd1.x() * 1.0;
  double y11 = aStart1.y() * 1.0;
  double y12 = anEnd1.y() * 1.0;

  double x21 = aStart2.x() * 1.0;
  double x22 = anEnd2.x() * 1.0;
  double y21 = aStart2.y() * 1.0;
  double y22 = anEnd2.y() * 1.0;

  double k1 = x12 == x11 ? 0 : ( y12 - y11 ) / ( x12 - x11 );
  double k2 = x22 == x21 ? 0 : ( y22 - y21 ) / ( x22 - x21 );

  double b1 = y11 - k1 * x11;
  double b2 = y21 - k2 * x21;

  if ( k1 == k2 )
  {
    if ( b1 != b2 )
      return false;
    else
      return !( ( QMAX( x11, x12 ) <= QMIN( x21, x22 ) ||
                  QMIN( x11, x12 ) >= QMAX( x21, x22 ) ) &&
                ( QMAX( y11, y12 ) <= QMIN( y21, y22 ) ||
                  QMIN( y11, y12 ) >= QMAX( y21, y22 ) ) );
  }
  else
  {
    double x0 = ( b2 - b1 ) / ( k1 - k2 );
    double y0 = ( k1 * b2 - k2 * b1 ) / ( k1 - k2 );

    if ( QMIN( x11, x12 ) < x0 && x0 < QMAX( x11, x12 ) &&
         QMIN( y11, y12 ) < y0 && y0 < QMAX( y11, y12 ) &&
         QMIN( x21, x22 ) < x0 && x0 < QMAX( x21, x22 ) &&
         QMIN( y21, y22 ) < y0 && y0 < QMAX( y21, y22 ) )
      return true;
  }
  return false;
}


