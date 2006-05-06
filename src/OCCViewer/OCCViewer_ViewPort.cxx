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

#if !defined WNT
#define QT_CLEAN_NAMESPACE         /* avoid definition of INT32 and INT8 */
#endif

#include "OCCViewer_ViewPort.h"

#include "SUIT_Session.h"

#include <qrect.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qintdict.h>
#include <qpopupmenu.h>
#include <qcolordialog.h>

#include <stdlib.h>

#if !defined WNT
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>
#undef QT_CLEAN_NAMESPACE
#include <Xw_Window.hxx>
#include <Graphic3d_GraphicDevice.hxx>

struct CMapEntry
{
	CMapEntry();
	~CMapEntry();
	Colormap          cmap;
	bool              alloc;
	XStandardColormap scmap;
};

/*!
  Constructor
*/
CMapEntry::CMapEntry()
{
	cmap = 0;
	alloc = false;
	scmap.colormap = 0;
}

/*!
  Destructor
*/
CMapEntry::~CMapEntry()
{
	if ( alloc )
        XFreeColormap( QPaintDevice::x11AppDisplay(), cmap );
}

static QIntDict<CMapEntry> *cmap_dict = 0;
static bool mesa_gl = false;

static void cleanup_cmaps()
{
	if ( !cmap_dict )
		return;
	cmap_dict->setAutoDelete( true );
	delete cmap_dict;
	cmap_dict = 0;
}

static Colormap choose_cmap( Display *dpy, XVisualInfo *vi )
{
	if ( !cmap_dict )
	{
		cmap_dict = new QIntDict<CMapEntry>;
		const char *v = glXQueryServerString( dpy, vi->screen, GLX_VERSION );
		mesa_gl = strstr( v,"Mesa" ) != 0;
		qAddPostRoutine( cleanup_cmaps );
	}

	CMapEntry *x = cmap_dict->find( (long)vi->visualid );
    if ( x )					// found colormap for visual
        return x->cmap;

    x = new CMapEntry();

    XStandardColormap *c;
    int n, i;

#ifdef DEBUG
    cout << "Choosing cmap for vID = " << vi->visualid << endl;
#endif

    if ( vi->visualid == XVisualIDFromVisual( (Visual*)QPaintDevice::x11AppVisual() ) )
	{
#ifdef DEBUG
		cout << "Using x11AppColormap" << endl;
#endif
		return QPaintDevice::x11AppColormap();
    }

    if ( mesa_gl )
	{
		Atom hp_cmaps = XInternAtom( dpy, "_HP_RGB_SMOOTH_MAP_LIST", true );
		if ( hp_cmaps && vi->visual->c_class == TrueColor && vi->depth == 8 )
		{
	        if ( XGetRGBColormaps( dpy, RootWindow( dpy, vi->screen ), &c, &n, hp_cmaps ) )
            {
		        i = 0;
			    while ( i < n && x->cmap == 0 )
                {
			        if ( c[i].visualid == vi->visual->visualid )
				    {
				        x->cmap = c[i].colormap;
					    x->scmap = c[i];
			        }
				    i++;
                }
			    XFree( (char*)c );
	        }
        }
	}
#if !defined( _OS_SOLARIS_ )
    if ( !x->cmap )
    {
	    if ( XmuLookupStandardColormap( dpy, vi->screen, vi->visualid, vi->depth, XA_RGB_DEFAULT_MAP, false, true ) )
        {
		    if ( XGetRGBColormaps( dpy, RootWindow( dpy, vi->screen ), &c, &n, XA_RGB_DEFAULT_MAP ) )
            {
			    i = 0;
				while ( i < n && x->cmap == 0 )
				{
				    if ( c[i].visualid == vi->visualid )
                    {
					    x->cmap = c[i].colormap;
						x->scmap = c[i];
				    }
					i++;
                }
				XFree( (char *)c );
            }
        }
    }
#endif
	if ( !x->cmap )
	{
		// no shared cmap found
		x->cmap = XCreateColormap( dpy, RootWindow( dpy, vi->screen ), vi->visual, AllocNone );
		x->alloc = true;
	}

	cmap_dict->insert( (long)vi->visualid, x ); // associate cmap with visualid
	return x->cmap;
}
#endif


/*!
    Constructor
*/
OCCViewer_ViewPort::OCCViewer_ViewPort( QWidget* parent )
: QWidget( parent, 0, WRepaintNoErase | WResizeNoErase )
{
	initialize();
}

/*!
    Destructor
*/
OCCViewer_ViewPort::~OCCViewer_ViewPort()
{
	cleanup();
}

/*!
    Initializes viewport. [ private ]
*/
void OCCViewer_ViewPort::initialize()
{
  myPopupActions.setAutoDelete( true );
  myPaintersRedrawing = false;
  myEnableSketching = false;
  myEnableTransform = true;
  
  setMouseTracking( true );
  setBackgroundMode( NoBackground );
  // set focus policy to threat QContextMenuEvent from keyboard  
  setFocusPolicy( StrongFocus );
}

/*!
    Cleans up the viewport. [ private ]
*/
void OCCViewer_ViewPort::cleanup()
{
}

/*!
    Selects visual ID for OpenGL window ( X11 specific ). [ protected ]
*/
void OCCViewer_ViewPort::selectVisualId()
{
#if !defined WNT
	XVisualInfo* pVisualInfo;
	if ( x11Display() )
	{
		/* Initialization with the default VisualID */
		Visual *v = DefaultVisual( x11Display(), DefaultScreen( x11Display() ) );
		int visualID = XVisualIDFromVisual( v );

		/*  Here we use the settings from Optimizer_ViewInfo::TxglCreateWindow() */
		int visualAttr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 1, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1,
							 GLX_BLUE_SIZE, 1, GLX_DOUBLEBUFFER, None };

		pVisualInfo = ::glXChooseVisual( x11Display(), DefaultScreen( x11Display() ), visualAttr );

		if ( isVisible() )
            hide();

		XSetWindowAttributes a;

		a.colormap = choose_cmap( x11Display(), pVisualInfo );	    /* find best colormap */
		a.background_pixel = backgroundColor().pixel();
		a.border_pixel = black.pixel();
		Window p = RootWindow( x11Display(), DefaultScreen( x11Display() ) );
		if ( parentWidget() )
            p = parentWidget()->winId();

        Window w;
	/*        if ( type == Type2D )  // creating simple X window for 2d
        {
            unsigned long xbackground =
                BlackPixel( x11Display(), DefaultScreen( x11Display() ) );
            unsigned long xforeground =
                WhitePixel( x11Display(), DefaultScreen( x11Display() ) );

            w = XCreateSimpleWindow ( x11Display(), p, x(), y(), width(),
                                      height(), 0, xforeground, xbackground );
        }
        else if ( type == Type3D )
        {
		    w = XCreateWindow( x11Display(), p,  x(), y(), width(), height(),
                               0, pVisualInfo->depth, InputOutput, pVisualInfo->visual,
                               CWBackPixel | CWBorderPixel | CWColormap, &a );
		}
		else
		return;*/
	w = XCreateWindow( x11Display(), p,  x(), y(), width(), height(),
			   0, pVisualInfo->depth, InputOutput, pVisualInfo->visual,
			   CWBackPixel | CWBorderPixel | CWColormap, &a );
	
		Window *cmw;
		Window *cmwret;
		int count;
		if ( XGetWMColormapWindows( x11Display(), topLevelWidget()->winId(), &cmwret, &count ) )
		{
			cmw = new Window[count+1];
			memcpy( (char*)cmw, (char*)cmwret, sizeof(Window) * count );
			XFree( (char*)cmwret );
            int i;

			for ( i = 0; i < count; i++ )
			{
				if ( cmw[i] == winId() ) /* replace old window */
				{
					cmw[i] = w;
					break;
				}
			}

			if ( i >= count )			 /* append new window */
				cmw[count++] = w;
		}
		else
		{
			count = 1;
			cmw = new Window[count];
			cmw[0] = w;
		}

		/* Creating new window (with good VisualID) for this widget */
		create(w);
		XSetWMColormapWindows( x11Display(), topLevelWidget()->winId(), cmw, count );
		delete[] cmw;

		if ( isVisible() )
            show();

		if ( pVisualInfo )
        {
			XFree( (char *)pVisualInfo );
		}
  		XFlush( x11Display() );
	}
#endif
}

/*!
    Sets the background 'color'. [ virtual ]
*/
void OCCViewer_ViewPort::setBackgroundColor( const QColor& color )
{
    QPalette pal = palette();
    pal.setColor( QColorGroup::Background, color );
    setPalette( pal );
    repaint();
    emit vpChangeBGColor( color );
}

/*!
    Returns the background color. [ virtual ]
*/
QColor OCCViewer_ViewPort::backgroundColor() const
{
    return palette().active().background();
}

/*!
    Returns 'true' if sketching is enabled in  this viewport. [ public ]
*/
bool OCCViewer_ViewPort::isSketchingEnabled() const
{
    return myEnableSketching;
}

/*!
    Enables / disables sketching  [ public ]
*/
void OCCViewer_ViewPort::setSketchingEnabled( bool enable )
{
	myEnableSketching = enable;
}

/*!
    Returns 'true' if transformations ( rotation, zoom etc. )
    are enabled in this viewport. [ public ]
*/
bool OCCViewer_ViewPort::isTransformEnabled() const
{
    return myEnableTransform;
}

/*!
    Enables / disables transformations. [ public ]
*/
void OCCViewer_ViewPort::setTransformEnabled( bool enable )
{
	myEnableTransform = enable;
}

/*!
    Emits 'mouseEvent' signal. [ virtual protected ]
*/
void OCCViewer_ViewPort::mousePressEvent( QMouseEvent *e )
{
    emit vpMouseEvent( e );
}

/*!
    Emits 'mouseEvent' signal. [ virtual protected ]
*/
void OCCViewer_ViewPort::mouseMoveEvent( QMouseEvent* e )
{
    emit vpMouseEvent( e );
}

/*!
    Emits 'mouseEvent' signal. [ virtual protected ]
*/
void OCCViewer_ViewPort::mouseReleaseEvent( QMouseEvent *e )
{
    emit vpMouseEvent( e );
}

/*!
    Emits 'mouseEvent' signal. [ virtual protected ]
*/
void OCCViewer_ViewPort::mouseDoubleClickEvent( QMouseEvent *e )
{
    emit vpMouseEvent( e );
}

/*!
    Emits 'keyEvent' signal. [ virtual protected ]
*/
void OCCViewer_ViewPort::keyPressEvent( QKeyEvent *e )
{
    emit vpKeyEvent( e );
}

/*!
    Emits 'keyEvent' signal. [ virtual protected ]
*/
void OCCViewer_ViewPort::keyReleaseEvent( QKeyEvent *e )
{
    emit vpKeyEvent( e );
}

/*!
    Repaints the viewport. [ virtual protected ]
*/
void OCCViewer_ViewPort::paintEvent( QPaintEvent* )
{
	if ( myPaintersRedrawing )
	{
		QPainter p( this );
		emit vpDrawExternal( &p );
		myPaintersRedrawing = false;
	}
}

/*!
    Forces to redraw the viewport by an external painter. [ public ]
*/
void OCCViewer_ViewPort::redrawPainters()
{
	myPaintersRedrawing = true;
	repaint();
}

/*!
    Updates this view. Does nothing by default. [ virtual public ]
*/
void OCCViewer_ViewPort::onUpdate()
{
}

/*!
    Creates the popup. [ virtual protected ]
*/
/*void OCCViewer_ViewPort::onCreatePopup( QPopupMenu* popup )
{
	if ( popup )
	{
		QtxAction* a = new QtxAction( "", tr( "MEN_VP_CHANGEBGR" ), 0, this );
        a->setStatusTip( tr( "PRP_VP_CHANGEBGR" ) );
		connect( a, SIGNAL( activated() ), SLOT( onChangeBgColor()));
		myPopupActions.append( a );
		a->addTo( popup );
	}
}*/

/*!
    Destroys the popup. [ virtual protected ]
*/
/*void OCCViewer_ViewPort::onDestroyPopup( QPopupMenu* popup )
{
	if ( popup )
	{
		for ( QtxAction* a = myPopupActions.first(); a; a = myPopupActions.next() )
			a->removeFrom( popup );
		myPopupActions.clear();
	}
}*/

/*!
    Sets the background color with color selection dialog. [ virtual protected slot ]
*/
void OCCViewer_ViewPort::onChangeBgColor()
{
    QColor selColor = QColorDialog::getColor ( backgroundColor(), this );
    if ( selColor.isValid() )
        setBackgroundColor( selColor );
}
