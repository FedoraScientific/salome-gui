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
#include "SOCC_ViewWindow.h"

#include "OCCViewer_ViewPort3d.h"

#include "SUIT_Accel.h"

/*!
  Constructor
*/
SOCC_ViewWindow
::SOCC_ViewWindow( SUIT_Desktop* theDesktop, 
		   OCCViewer_Viewer* theModel)
  : OCCViewer_ViewWindow( theDesktop, theModel )
{
}

/*!
  Destructor
*/
SOCC_ViewWindow
::~SOCC_ViewWindow()
{
}

/*!
  Performs action
  \param theAction - type of action
*/
bool 
SOCC_ViewWindow
::action( const int theAction  )
{
  const int inc = 10;
  int cx, cy;
  if ( theAction == SUIT_Accel::ZoomIn || theAction == SUIT_Accel::ZoomOut ||
       theAction == SUIT_Accel::RotateLeft || theAction == SUIT_Accel::RotateRight ||
       theAction == SUIT_Accel::RotateUp || theAction == SUIT_Accel::RotateDown ) {
    cx = myViewPort->width() / 2;
    cy = myViewPort->height() / 2;
  }
  switch ( theAction ) {
  case SUIT_Accel::PanLeft     : 
    myViewPort->pan( -inc, 0 );   
    break;
  case SUIT_Accel::PanRight    : 
    myViewPort->pan(  inc, 0 );   
    break;
  case SUIT_Accel::PanUp       : 
    myViewPort->pan( 0, inc );   
    break;
  case SUIT_Accel::PanDown     : 
    myViewPort->pan( 0, -inc );   
    break;
  case SUIT_Accel::ZoomIn      : 
    myViewPort->zoom( cx, cy, cx + inc, cy + inc );
    break;
  case SUIT_Accel::ZoomOut     : 
    myViewPort->zoom( cx, cy, cx - inc, cy - inc );
    break;
  case SUIT_Accel::ZoomFit     :
    myViewPort->fitAll();
    break;
  case SUIT_Accel::RotateLeft  : 
    myViewPort->startRotation( cx, cy );
    myViewPort->rotate( cx - inc, cy );
    myViewPort->endRotation();
    break;
  case SUIT_Accel::RotateRight :  
    myViewPort->startRotation( cx, cy );
    myViewPort->rotate( cx + inc, cy );
    myViewPort->endRotation();
    break;
  case SUIT_Accel::RotateUp    :  
    myViewPort->startRotation( cx, cy );
    myViewPort->rotate( cx, cy - inc );
    myViewPort->endRotation();
    break;
  case SUIT_Accel::RotateDown  :  
    myViewPort->startRotation( cx, cy );
    myViewPort->rotate( cx, cy + inc );
    myViewPort->endRotation();
    break;
  }
  return true;
}
