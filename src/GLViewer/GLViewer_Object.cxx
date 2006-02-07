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

/***************************************************************************
**  Class:   GLViewer_Object
**  Descr:   OpenGL Object
**  Module:  GLViewer
**  Created: UI team, 03.09.02
****************************************************************************/

//#include <GLViewerAfx.h>
#include "GLViewer_Object.h"
#include "GLViewer_Drawer.h"
#include "GLViewer_AspectLine.h"
#include "GLViewer_Geom.h"
#include "GLViewer_Text.h"
#include "GLViewer_Group.h"

//#include <cmath>
//using namespace std;

GLViewer_Object::GLViewer_Object()
{
  myXScale = 1.0; 
  myYScale = 1.0;
  myXGap = 0;
  myYGap = 0;
  myZoom = 1.0;

  myIsHigh = GL_FALSE;
  myIsSel = GL_FALSE;
  
  myRect = new GLViewer_Rect();;  
  myUpdateRect = new GLViewer_Rect();;  
  myGLText = new GLViewer_Text( 0, 0, 0, QColor(0,0,0) );

  myAspectLine = new GLViewer_AspectLine();
  myType = "GLViewer_Object";

  myOwner = NULL;
  myDrawer = NULL;

  myIsVisible = true;

  isToolTipHTML = false;  

  myGroup = NULL;
}

GLViewer_Object::~GLViewer_Object()
{
  if( myRect )
    delete myRect;

  if( myUpdateRect )
    delete myUpdateRect;

  if( myGLText )
    delete myGLText;

  if( myAspectLine )
    delete myAspectLine;
}

int GLViewer_Object::getPriority() const
{
    return myDrawer ? myDrawer->getPriority() : 0;
}

GLboolean GLViewer_Object::isInside( GLViewer_Rect theRect )
{
    return theRect.toQRect().contains( myRect->toQRect() );
}

GLboolean GLViewer_Object::setZoom( GLfloat zoom, bool, bool )
{
    if( myZoom == zoom )
        return GL_FALSE;

    myZoom = zoom;
    return GL_TRUE;
}

GLboolean GLViewer_Object::updateZoom( bool zoomIn )
{
    float newZoom;
    float step = zoomIn ? 1 : -1;
    double epsilon = 0.001;

    if( myZoom - 1 > epsilon )
        newZoom = ( myZoom * 2 + step ) / 2;
    else if( 1 - myZoom > epsilon )
        newZoom = 2 / ( 2 / myZoom - step );
    else
        newZoom = zoomIn ? 3./2. : 2./3.;

    if( newZoom < 0.01 || newZoom > 100.0 )
        return GL_FALSE;

    return setZoom( newZoom, true );
}

QByteArray GLViewer_Object::getByteCopy()
{
    int i = 0;
    int anISize = sizeof( int );

    const char* aTypeStr = myType.data();
    const char* aToolTipStr = myToolTipText.data();

    int aTypeLength = myType.length();
    int aToolTipLength = myToolTipText.length();


    QByteArray aGLText = myGLText->getByteCopy();
    QByteArray aAspect = myAspectLine->getByteCopy();
    
    float aRectData[8];
    aRectData[ 0 ] = myRect->left();
    aRectData[ 1 ] = myRect->top();
    aRectData[ 2 ] = myRect->right();
    aRectData[ 3 ] = myRect->bottom();
    aRectData[ 4 ] = myXScale;
    aRectData[ 5 ] = myYScale;
    aRectData[ 6 ] = myXGap;
    aRectData[ 7 ] = myYGap;
    
    int sizeOf8Float = sizeof( aRectData );

    QByteArray aResult( 2*anISize + sizeOf8Float + 
                        aTypeLength + aToolTipLength +
                        aGLText.size() + aAspect.size() );
    // puts 8 float values into the byte array
    char* aPointer = (char*)&aRectData;
    for( i = 0; i < sizeOf8Float; i++, aPointer++ )
        aResult[i] = *aPointer;
    // puts length of type string
    aPointer = (char*)&aTypeLength;
    for( ; i < anISize + sizeOf8Float; i++, aPointer++ )
        aResult[i] = *aPointer;
    // puts type string
    for( ; i < anISize + sizeOf8Float + aTypeLength; i++ )
        aResult[i] = aTypeStr[i - anISize - sizeOf8Float ];
    // puts length of tooltiptext string
    aPointer = (char*)&aToolTipLength;
    for( ; i < 2*anISize + sizeOf8Float + aTypeLength; i++, aPointer++ )
        aResult[i] = *aPointer;
    // puts tooltiptext string
    for( ; i <  2*anISize + sizeOf8Float + aTypeLength + aToolTipLength; i++ )
        aResult[ i] = aToolTipStr[i - 2*anISize - sizeOf8Float - aTypeLength];

    int aCurPos = 2*anISize + sizeOf8Float + aTypeLength + aToolTipLength;
    // adds aspect byte array
    for( i = aCurPos; i < aCurPos + aAspect.size(); i++ )
        aResult[i] = aAspect[i - aCurPos];

    aCurPos = aCurPos + aAspect.size();
    // adds GL text byte array
    for( i = aCurPos; i < aCurPos + aGLText.size(); i++ )
        aResult[i] = aGLText[i - aCurPos];    

    aCurPos += aGLText.size();
    aPointer = (char*)&myOwner;
    for( i = 0; i < sizeof( GLViewer_Owner* ); i++, aPointer++ )
        aResult[ aCurPos + i ] = *aPointer;

    return aResult;
}

bool GLViewer_Object::initializeFromByteCopy( QByteArray theArray )
{
    int i = 0;
    int anISize = sizeof( int );
    int aFSize = sizeof( GLfloat );
    
    float aLeft = 0, aTop = 0, aRight = 0, aBottom = 0;    

    //QString aTypeStr, aToolTipStr;
    int aTypeLength = 0, aToolTipLength = 0;

    int aSize = theArray.size();

    GLViewer_Text* aGLText = new GLViewer_Text( 0, 0, 0, QColor(255,255,255));
    int aGLTextMinSize = (aGLText->getByteCopy()).size();
    GLViewer_AspectLine* aAspectLine = new GLViewer_AspectLine();
    int aGLAspLineSize = (aAspectLine->getByteCopy()).size();

    QByteArray aGLTextArray, aAspect( aGLAspLineSize );

    if( aSize < 2*anISize + 8*aFSize + aGLTextMinSize + aGLAspLineSize )
        return false;

    char* aPointer = (char*)&aLeft;
    for( i = 0; i < aFSize; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)&aTop;
    for( ; i < 2*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)&aRight;
    for( ; i < 3*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)&aBottom;
    for( ; i < 4*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];

    //myRect = new QRect( aLeft, aTop, aRight - aLeft, aBottom - aTop );
    myRect = new GLViewer_Rect( aLeft, aRight, aTop, aBottom );

    aPointer = (char*)&myXScale;
    for( ; i < 5*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)&myYScale;
    for( ; i < 6*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)&myXGap;
    for( ; i < 7*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)&myYGap;
    for( ; i < 8*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];

    myIsHigh = false;
    myIsSel = false;
    myIsVisible = true;

    aPointer = (char*)&aTypeLength;
    for( ; i < anISize + 8*aFSize; i++, aPointer++ )
        *aPointer = theArray[i];
    myType = "";
    for( ; i < anISize + 8*aFSize + aTypeLength; i++ )
    {
        QChar aChar( theArray[i] );
        myType += aChar;
    }

    aPointer = (char*)&aToolTipLength;
    for( ; i < 2*anISize + 8*aFSize + aTypeLength; i++, aPointer++ )
        *aPointer = theArray[i];
    myToolTipText= "";
    for( ; i < 2*anISize + 8*aFSize + aTypeLength + aToolTipLength; i++ )
    {
        QChar aChar( theArray[i] );
        myToolTipText += aChar;
    }
    
    int aCurPos = 2*anISize + 8*aFSize + aTypeLength + aToolTipLength;
    if( aSize - aCurPos < aGLTextMinSize + aGLAspLineSize )
        return false;

    for( i = 0; i < aGLAspLineSize; i++ )
        aAspect[i] = theArray[ aCurPos + i ];
    myAspectLine = GLViewer_AspectLine::fromByteCopy( aAspect );

    aCurPos = aCurPos + aGLAspLineSize;
    aGLTextArray.resize( aSize - aCurPos );
    for( i = 0; i + aCurPos < aSize; i++ )
        aGLTextArray[i] = theArray[ aCurPos + i ];
    // replace gl_text pointer by other
    if ( myGLText )
      delete myGLText;
    myGLText = GLViewer_Text::fromByteCopy( aGLTextArray );
    
    return true;        
}

void GLViewer_Object::setGroup( GLViewer_Group* theGroup )
{
  if ( myGroup == theGroup )
    return;

  if( myGroup )
    myGroup->removeObject( this );
  
  myGroup = theGroup;
  if( theGroup )
    myGroup->addObject( this );
}

GLViewer_Group* GLViewer_Object::getGroup() const
{
  return myGroup;
}
