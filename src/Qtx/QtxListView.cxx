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

#include "QtxListView.h"

#include <qheader.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>

static const char* list_xpm[] = {
"16 16 6 1",
". c None",
"a c #E3E9EB",
"b c #798391",
"c c #EBEBEB",
"d c #ABB4BE",
"e c #030E1F",
"................",
"................",
"................",
"...aaaaaaaaaa...",
"..abbbbbbbbbbe..",
"..abecbecbecbe..",
"..abbbbbbbbbbe..",
"..abecbecbecbe..",
"..abecaaaaaaaa..",
"..abeccdbbbbbb..",
"..abecccdbbbbe..",
"..abbbbe.dbbe...",
"...eeeee..de....",
"................",
"................",
"................" };

QtxListView::QtxListView( const int state, QWidget* parent, const char* name, WFlags f )
: QListView( parent, name, f ),
myButton( 0 ),
myHeaderState( state )
{
  initialize();
}

QtxListView::QtxListView( QWidget* parent, const char* name, WFlags f )
: QListView( parent, name, f ),
myButton( 0 ),
myHeaderState( HeaderAuto )
{
  initialize();
}

void QtxListView::initialize()
{
  if ( myHeaderState == HeaderButton )
  {
    QPixmap p( list_xpm );

    QPushButton* but = new QPushButton( this );
    but->setDefault( false );
    but->setFlat( true );
    but->setIconSet( p );
    but->setBackgroundPixmap( p );
    if ( p.mask() )
	    but->setMask( *p.mask() );
    myButton = but;

    connect( myButton, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
  }
  else
  {
    header()->installEventFilter( this );
  }

  myPopup = new QPopupMenu( this );
  connect( myPopup, SIGNAL( activated( int ) ), this, SLOT( onShowHide( int ) ) );
  connect( header(), SIGNAL( sizeChange( int, int, int ) ), this, SLOT( onHeaderResized() ) );
}

QtxListView::~QtxListView()
{
}

int QtxListView::addColumn( const QString& label, int width )
{
  int res = QListView::addColumn( label, width );
  for ( int i = myAppropriate.count(); i <= res; i++ )
    myAppropriate.append( 1 );
  onHeaderResized();
  return res;
}

int QtxListView::addColumn( const QIconSet& iconset, const QString& label, int width ) 
{
  int res = QListView::addColumn( iconset, label, width );
  for ( int i = myAppropriate.count(); i <= res; i++ )
    myAppropriate.append( 1 );
  onHeaderResized();
  return res;
}

void QtxListView::removeColumn( int index ) 
{
  QListView::removeColumn( index );
  if ( index >= 0 && index < (int)myAppropriate.count() )
    myAppropriate.remove( myAppropriate.at( index ) );
  onHeaderResized();
}

bool QtxListView::appropriate( const int index ) const
{
  return index >= 0 && index < (int)myAppropriate.count() && myAppropriate[index];
}

void QtxListView::setAppropriate( const int index, const bool on )
{
  if ( index < 0 || index >= (int)myAppropriate.count() )
    return;

  myAppropriate[index] = on ? 1 : 0;
}

void QtxListView::resize( int w, int h )
{
  QListView::resize( w, h );
  onHeaderResized();
}

void QtxListView::show()
{
  QListView::show();
  onHeaderResized();
}

void QtxListView::resizeContents( int w, int h )
{
/*
  if ( myButton && myButton->isVisibleTo( myButton->parentWidget() ) )
  {
    if ( header()->orientation() == Qt::Horizontal )
      w += myButton->width();
    else
      h += myButton->width();
  }
*/
  QListView::resizeContents( w, h );

  onHeaderResized();
}

void QtxListView::show( int ind )
{
  setShown( ind, true );
}

void QtxListView::hide( int ind )
{
  setShown( ind, false );
}

bool QtxListView::isShown( int ind ) const
{
  if ( ind>=0 && ind<header()->count() )
    return columnWidth( ind ) > 0 || header()->isResizeEnabled( ind );
  else
    return false;
}

void QtxListView::setShown( int ind, bool sh )
{
  if( ind<0 || ind>=header()->count() || isShown( ind )==sh )
    return;

  ColumnData& data = myColumns[ind];
  if ( sh )
  {
    int w = data.width;
    bool resizeable = data.resizeable;
    myColumns.remove( ind );

    setColumnWidth( ind, w );
    header()->setResizeEnabled( resizeable, ind );
  }
  else
  {
    int w = columnWidth( ind );
    bool r = header()->isResizeEnabled( ind );
    setColumnWidth( ind, 0 );
    header()->setResizeEnabled( false, ind );
    data.width = w;
    data.resizeable = r;
  }
  updateContents();
}

void QtxListView::setColumnWidth( int c, int w )
{
  if ( myColumns.contains( c ) )
    myColumns[c].width = w;

  QListView::setColumnWidth( c, !myColumns.contains( c ) ? w : 0 );
}

QSize QtxListView::sizeHint() const
{
  QSize sz = QListView::sizeHint();

  if ( myButton && myButton->isVisibleTo( myButton->parentWidget() ) )
    sz.setWidth( sz.width() + 2 + myButton->width() );

  return sz;
}

QSize QtxListView::minimumSizeHint() const
{
  QSize sz = QListView::minimumSizeHint();

  if ( myButton && myButton->isVisibleTo( myButton->parentWidget() ) )
    sz.setWidth( sz.width() + 2 + myButton->width() );

  return sz;
}

void QtxListView::onHeaderResized()
{
  if ( myHeaderState == HeaderAuto )
  {
    int c = 0;
    for ( int i = 0; i < columns(); i++ )
    {
      if ( !header()->label( i ).isEmpty() ||
           ( header()->iconSet( i ) && !header()->iconSet( i )->isNull() ) )
        c++;
    }

    if ( c > 1 )
      header()->show();
    else
      header()->hide();
  }

  if ( !myButton || !header()->isVisibleTo( this ) )
    return;

  int lw = lineWidth();
  int h = header()->size().height() - 1;
  myButton->setFixedSize( h, h );

  int x = header()->headerWidth() - header()->offset() + 2;
  if ( x < header()->width() - h )
    x = header()->width() - h;

  if ( myHeaderState == HeaderButton )
  {
    if ( header()->orientation() == Qt::Horizontal )
      myButton->move( lw+x, lw );
    else
      myButton->move( lw, lw+x );
  }
}

void QtxListView::showPopup( const int x, const int y )
{
  myPopup->clear();
  for ( int i = 0; i < columns(); i++ )
  {
    if ( appropriate( i ) )
    {
      int id = myPopup->insertItem( header()->label( i ), i );
      myPopup->setItemChecked( id, isShown( i ) );
    }
  }

  if( myPopup->count() )
    myPopup->exec( mapToGlobal( QPoint( x, y ) ) );
}

void QtxListView::onButtonClicked()
{
  if ( myHeaderState != HeaderButton )
    return;

  int x = myButton->x(),
      y = myButton->y() + myButton->height();

  showPopup( x, y );
}

void QtxListView::onShowHide( int id )
{
  //if ( myHeaderState != HeaderButton )
  //  return;

  setShown( id, !isShown( id ) );
}

void QtxListView::viewportResizeEvent( QResizeEvent* e )
{
  QListView::viewportResizeEvent( e );
  onHeaderResized();
}

bool QtxListView::eventFilter( QObject* o, QEvent* e )
{
  if( o==header() && e->type()==QEvent::MouseButtonPress )
  {
    QMouseEvent* me = ( QMouseEvent* )e;
    if( me->button()==Qt::RightButton )
    {
      showPopup( me->x()+2, me->y()+2 );
      return true;
    }
  }
  
  return QListView::eventFilter( o, e );
}
