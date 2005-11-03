//  Copyright (C) 2005 OPEN CASCADE
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org
//
//  Author : OPEN CASCADE
//

// File:      GLViewer_Grid.cxx
// Created:   November, 2004

//#include <GLViewerAfx.h>
#include "GLViewer_Grid.h"
#include "GLViewer_Defs.h"

#include <Precision.hxx>
#include <qglobal.h>

GLViewer_Grid::GLViewer_Grid() :
       myGridList( 0 ), myGridHeight( (GLfloat)0.0 ), myGridWidth( (GLfloat)0.0 ),
       myWinW( (GLfloat)0.0 ), myWinH( (GLfloat)0.0 ), myXSize( (GLfloat)0.0 ), myYSize( (GLfloat)0.0 ),
       myXPan( (GLfloat)0.0 ), myYPan( (GLfloat)0.0 ), myXScale( (GLfloat)1.0 ), myYScale( (GLfloat)1.0 ),
       myLineWidth( (GLfloat)0.05 ), myCenterWidth( (GLfloat)1.5 ), myCenterRadius( (GLfloat)5.0 ), 
       myScaleFactor( 10 ), myIsUpdate( GL_FALSE )
{
  myGridColor[0] = 0.5;
  myGridColor[1] = 0.5;
  myGridColor[2] = 0.5;
  myAxisColor[0] = 0.75;
  myAxisColor[1] = 0.75;
  myAxisColor[2] = 0.75;
}

GLViewer_Grid::GLViewer_Grid( GLfloat width, GLfloat height,
                              GLfloat winW, GLfloat winH,
                              GLfloat xSize, GLfloat ySize,
                              GLfloat xPan, GLfloat yPan,
                              GLfloat xScale, GLfloat yScale ) :
       myGridList( 0 ), myGridHeight( (GLfloat)0.0 ), myGridWidth( (GLfloat)0.0 ),
       myWinW( (GLfloat)0.0 ), myWinH( (GLfloat)0.0 ), myXSize( (GLfloat)0.0 ), myYSize( (GLfloat)0.0 ),
       myXPan( (GLfloat)0.0 ), myYPan( (GLfloat)0.0 ), myXScale( (GLfloat)1.0 ), myYScale( (GLfloat)1.0 ),
       myLineWidth( (GLfloat)0.05 ), myCenterWidth( (GLfloat)1.5 ), myCenterRadius( (GLfloat)5.0 ), 
       myScaleFactor( 10 ), myIsUpdate( GL_FALSE )
{
  myGridColor[0] = 0.5;
  myGridColor[1] = 0.5;
  myGridColor[2] = 0.5;
  myAxisColor[0] = 0.75;
  myAxisColor[1] = 0.75;
  myAxisColor[2] = 0.75;
}

GLViewer_Grid::~GLViewer_Grid()
{
}

void GLViewer_Grid::draw()
{
  if ( myGridList == 0 || myIsUpdate )
    initList();

  glCallList( myGridList );
}

void GLViewer_Grid::setGridColor( GLfloat r, GLfloat g, GLfloat b )
{
  if( myGridColor[0] == r && myGridColor[1] == g && myGridColor[2] == b )
    return;

  myGridColor[0] = r;
  myGridColor[1] = g;
  myGridColor[2] = b;
  myIsUpdate = GL_TRUE;
}

void GLViewer_Grid::setAxisColor( GLfloat r, GLfloat g, GLfloat b )
{
  if( myAxisColor[0] == r && myAxisColor[1] == g && myAxisColor[2] == b )
    return;

  myAxisColor[0] = r;
  myAxisColor[1] = g;
  myAxisColor[2] = b;
  myIsUpdate = GL_TRUE;
}

void GLViewer_Grid::setGridWidth( float w )
{
  if( myGridWidth == w )
    return;

  myGridWidth = w;
  myIsUpdate = GL_TRUE;
}

void GLViewer_Grid::setCenterRadius( int r )
{
  if( myCenterRadius == r )
    return;

  myCenterRadius = r;
  myIsUpdate = GL_TRUE;
}

void GLViewer_Grid::setSize( float xSize, float ySize )
{
  if( myXSize == xSize && myYSize == ySize )
    return;
  
  myXSize = xSize;
  myYSize = ySize;
  myIsUpdate = GL_TRUE;
}

void GLViewer_Grid::setPan( float xPan, float yPan )
{
  if( myXPan == xPan && myYPan == yPan )
    return;
 
  myXPan = xPan;
  myYPan = yPan;
  myIsUpdate = GL_TRUE; 
}

bool GLViewer_Grid::setZoom( float zoom )
{
  if( zoom == 1.0 )
    return true;
  
  //backup values
  float bXScale = myXScale;
  float bYScale = myYScale;

  myXScale /= zoom; 
  myYScale /= zoom;

  if( fabs(myXScale) < Precision::Confusion() || fabs(myYScale) < Precision::Confusion() )
  { //undo
    myXScale = bXScale;
    myYScale = bYScale;
    return false;
  }
  
  myGridWidth /= zoom; 
  myGridHeight /= zoom;  
  myIsUpdate = GL_TRUE;
  return true;
}

void GLViewer_Grid::setResize( float WinW, float WinH, float zoom )
{
  if( myWinW == WinW && myWinH == WinH && zoom == 1.0 )
    return;

  myGridWidth = myGridWidth + ( WinW - myWinW ) * myXScale; 
  myGridHeight = myGridHeight + ( WinH - myWinH ) * myYScale;
  myWinW = WinW;
  myWinH = WinH;
  setZoom( zoom );
  myIsUpdate = GL_TRUE;
}

void GLViewer_Grid::getSize( float& xSize, float& ySize ) const
{
  xSize = myXSize;
  ySize = myYSize;
}

void GLViewer_Grid::getPan( float& xPan, float& yPan ) const
{
  xPan = myXPan;
  yPan = myYPan;
}

void GLViewer_Grid::getScale( float& xScale, float& yScale ) const
{
  xScale = myXScale;
  yScale = myYScale;
}

bool GLViewer_Grid::initList()
{
  myIsUpdate = GL_FALSE;
   
    if( myXSize == (GLfloat)0.0 )
        myXSize = (GLfloat)0.1;
    if( myYSize == (GLfloat)0.0 )
        myYSize = (GLfloat)0.1;

label:
  if( ( myXSize >= myGridWidth/5 ) && ( myYSize >= myGridHeight/5 ) )
  { //zoom in
    myXSize /= myScaleFactor;
    myYSize /= myScaleFactor;
    goto label;
  }
  else if( ( myXSize * myScaleFactor < myGridWidth/5 ) 
        || ( myYSize * myScaleFactor < myGridHeight/5 ) )
  { //zoom out
    myXSize *= myScaleFactor;
    myYSize *= myScaleFactor;
    goto label;
  }

  //int n = myGridWidth / myXSize;
  //int m = myGridHeight / myYSize;
  // do not initialise integer by float
  //if( ( n != 0 ) || ( m != 0 ) ) 
  if( ( myGridWidth > 0.5 * myXSize ) || ( myGridHeight > 0.5 * myYSize ) )
  { 
    if ( myGridList != 0 )  
    { 
      glDeleteLists( myGridList, 1 ); 
      if ( glGetError() != GL_NO_ERROR ) 
    return FALSE;
    } 
         
    float xLoc = (int)(myXPan / myXSize) * myXSize; 
    float yLoc = (int)(myYPan / myYSize) * myYSize; 
 
    myGridList = glGenLists( 1 ); 
    glNewList( myGridList, GL_COMPILE ); 

    glColor3f( myGridColor[0], myGridColor[1], myGridColor[2] );  
    glLineWidth( myLineWidth ); 
    
    glBegin( GL_LINES ); 
    for( int j = 0; ( j-1 ) * myXSize <= myGridWidth / 2 ; j++ )
    { 
      glVertex2d( -myXSize * j - xLoc, -myGridHeight / 2 - myYSize - yLoc );
      glVertex2d( -myXSize * j - xLoc,  myGridHeight / 2 + myYSize - yLoc ); 
      glVertex2d(  myXSize * j - xLoc, -myGridHeight / 2 - myYSize - yLoc );
      glVertex2d(  myXSize * j - xLoc,  myGridHeight / 2 + myYSize - yLoc );
    }
    for( int i = 0; ( i-1 ) * myYSize <= myGridHeight / 2 ; i++)  
    {
      glVertex2d( -myGridWidth / 2 - myXSize - xLoc, -myYSize * i - yLoc ); 
      glVertex2d(  myGridWidth / 2 + myXSize - xLoc, -myYSize * i - yLoc ); 
      glVertex2d( -myGridWidth / 2 - myXSize - xLoc,  myYSize * i - yLoc ); 
      glVertex2d(  myGridWidth / 2 + myXSize - xLoc,  myYSize * i - yLoc ); 
    } 
    glEnd();

    glColor3f( myAxisColor[0], myAxisColor[1], myAxisColor[2] );
    glLineWidth( myCenterWidth );

    glBegin( GL_LINES );
    glVertex2d(  myGridWidth / 2 + myXSize - xLoc, 0); 
    glVertex2d( -myGridWidth / 2 - myXSize - xLoc, 0); 
    glVertex2d( 0,  myGridHeight / 2 + myYSize - yLoc );
    glVertex2d( 0, -myGridHeight / 2 - myYSize - yLoc );    
    glEnd();

    glBegin( GL_LINE_LOOP ); 
    double angle = 0.0;
    for ( int k = 0; k < SEGMENTS; k++ )     
    { 
      glVertex2f( cos(angle) * myCenterRadius * myXScale,
          sin(angle) * myCenterRadius * myYScale ); 
      angle += STEP; 
    } 
    glEnd();

    glEndList();
  }
  return TRUE;
}
