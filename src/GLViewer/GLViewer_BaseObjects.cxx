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
**  Class:   GLViewer_BaseObjects
**  Descr:   Internal OpenGL Objects
**  Module:  GLViewer
**  Created: UI team, 02.09.02
****************************************************************************/

//#include <GLViewerAfx.h>
#include "GLViewer_BaseObjects.h"
#include "GLViewer_BaseDrawers.h"
#include "GLViewer_AspectLine.h"
#include "GLViewer_CoordSystem.h"
#include "GLViewer_Text.h"
#include "GLViewer_Group.h"

#include "GLViewer_Drawer.h"

//#include <cmath>
//using namespace std;

/***************************************************************************
**  Class:   GLViewer_MarkerSet
**  Descr:   OpenGL MarkerSet
**  Module:  GLViewer
**  Created: UI team, 03.09.02
****************************************************************************/

GLViewer_MarkerSet::GLViewer_MarkerSet( int number, float size, const QString& toolTip ) :
  GLViewer_Object(),
  myNumber( 0 ),
  myXCoord( 0 ),
  myYCoord( 0 )       
{
    
    myMarkerSize = size;
    myHNumbers.clear();
    myUHNumbers.clear();
    mySelNumbers.clear();
    myUSelNumbers.clear();
    myCurSelNumbers.clear();
    myPrevHNumbers.clear();    

    myType = "GLViewer_MarkerSet";
    myToolTipText = toolTip;
    
    setNumMarkers( number );    
}

GLViewer_MarkerSet::~GLViewer_MarkerSet()
{
    if ( myXCoord )
        delete[] myXCoord;
    if ( myYCoord )
        delete[] myYCoord;
}

void AddCoordsToHPGL( QString& buffer, QString command, GLViewer_CoordSystem* aViewerCS, 
                      GLViewer_CoordSystem* aPaperCS, double x, double y, bool NewLine = true )
{
    if( aViewerCS && aPaperCS )
        aViewerCS->transform( *aPaperCS, x, y );

    QString temp = command + "%1, %2;";
    buffer += temp.arg( x ).arg( y );
    if( NewLine )
        buffer += ";\n";
}

void AddCoordsToPS( QString& buffer, QString command, GLViewer_CoordSystem* aViewerCS, 
                    GLViewer_CoordSystem* aPaperCS, double x, double y, bool NewLine = true )
{
    if( aViewerCS && aPaperCS )
        aViewerCS->transform( *aPaperCS, x, y );

    QString temp = "%1 %2 "+command;    
    buffer += temp.arg( x ).arg( y );
    if( NewLine )
        buffer += "\n";
}

void AddLineAspectToPS( QString& buffer, GLViewer_AspectLine* anAspect, 
                        GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aPaperCS )
{
    if( anAspect )
    {
        QColor col1, col2, col3;
        anAspect->getLineColors( col1, col2, col3 );

        float aWidth = anAspect->getLineWidth();
        int aLineType = anAspect->getLineType();

        QString temp = "%1 %2 %3 setrgbcolor\n";
        double rr = 1 - double( col1.red() ) / 255.0, //color inverting
               gg = 1 - double( col1.green() ) / 255.0,
               bb = 1 - double( col1.blue() ) / 255.0;

        buffer += temp.arg( rr ).arg( gg ).arg( bb );

        double x_stretch, y_stretch;
        aViewerCS->getStretching( *aPaperCS, x_stretch, y_stretch );
        buffer += temp.arg( x_stretch * aWidth )+" setlinewidth\n";

        if( aLineType==0 ) //solid
            buffer += "[] 0 setdash\n";
        else if( aLineType==1 ) //strip
            buffer += "[2] 0 setdash\n";
    }
}

#ifdef WIN32
HPEN AddLineAspectToEMF( HDC hDC, GLViewer_AspectLine* anAspect, 
                         GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aPaperCS )
{
    if( anAspect )
    {
        QColor col1, col2, col3;
        anAspect->getLineColors( col1, col2, col3 );

        double x_stretch, y_stretch;
        aViewerCS->getStretching( *aPaperCS, x_stretch, y_stretch );

        double aWidth = anAspect->getLineWidth()*x_stretch;
        int aLineType = anAspect->getLineType();

        return CreatePen( PS_SOLID, aWidth, RGB( 255-col1.red(), 255-col1.green(), 255-col1.blue() ) );
    }
    else
        return NULL;
}
#endif

bool GLViewer_MarkerSet::translateToPS( QFile& hFile, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aPSCS )
{   
    int noPoints = 20;

    QString aBuffer = "newpath\n";

    AddLineAspectToPS( aBuffer, getAspectLine(), aViewerCS, aPSCS );

    for( int i=0; i<myNumber; i++ )
    {       
        aBuffer += "\n";

        double x_stretch, y_stretch;
        aViewerCS->getStretching( *aPSCS, x_stretch, y_stretch );

        double x0 = myXCoord[i],
               y0 = myYCoord[i],
               r  = myMarkerSize,
               x, y;

        for( int j=0; j<=noPoints; j++ )
        {
            x = x0 + r*cos( double(j)*2*PI/double(noPoints) );
            y = y0 + r*sin( double(j)*2*PI/double(noPoints) );          
            if( j==0 )
                AddCoordsToPS( aBuffer, "moveto", aViewerCS, aPSCS, x, y, true );               
            else
                AddCoordsToPS( aBuffer, "lineto", aViewerCS, aPSCS, x, y, true );
        }
    }
    aBuffer+="closepath\nstroke\n";

    hFile.writeBlock( aBuffer.ascii(), aBuffer.length() );

    return true;
}

bool GLViewer_MarkerSet::translateToHPGL( QFile& hFile, GLViewer_CoordSystem* aViewerCS,
                                       GLViewer_CoordSystem* aHPGLCS )
{
    int noPoints = 20;
    QString aBuffer;
    for( int i=0; i<myNumber; i++ )
    {
        aBuffer = "";

        double x_stretch, y_stretch;
        aViewerCS->getStretching( *aHPGLCS, x_stretch, y_stretch );

        double x0 = myXCoord[i],
               y0 = myYCoord[i],
               r  = myMarkerSize,
               x, y;

        AddCoordsToHPGL( aBuffer, "PA", aViewerCS, aHPGLCS, x0+r, y0 );
        aBuffer+="PD;\n";
        for( int j=1; j<=noPoints; j++ )
        {
            x = x0 + r*cos( double(j)*2*PI/double(noPoints) );
            y = y0 + r*sin( double(j)*2*PI/double(noPoints) );
            AddCoordsToHPGL( aBuffer, "PD", aViewerCS, aHPGLCS, x, y );
        }
        aBuffer+="PU;\n";

        hFile.writeBlock( aBuffer.ascii(), aBuffer.length() );
    }

    return true;
}

#ifdef WIN32
bool GLViewer_MarkerSet::translateToEMF( HDC dc, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aEMFCS )
{
    int noPoints = 20;
    if( !aViewerCS || !aEMFCS )
        return false;
    
    HPEN pen = AddLineAspectToEMF( dc, getAspectLine(), aViewerCS, aEMFCS );
    HGDIOBJ old = SelectObject( dc, pen );

    for( int i=0; i<myNumber; i++ )
    {
        double x0 = myXCoord[i],
               y0 = myYCoord[i],
               r  = myMarkerSize,
               x, y;

        for( int j=0; j<=noPoints; j++ )
        {
            x = x0 + r*cos( double(j)*2*PI/double(noPoints) );
            y = y0 + r*sin( double(j)*2*PI/double(noPoints) );
            aViewerCS->transform( *aEMFCS, x, y );
            if( j==0 )
                MoveToEx( dc, x, y, NULL );
            else
                LineTo( dc, x, y );
        }
    }

    SelectObject( dc, old );
    if( pen )
        DeleteObject( pen );
    return true;
}
#endif


void GLViewer_MarkerSet::compute()
{
//  cout << "GLViewer_MarkerSet::compute" << endl;
  GLfloat xa = myXCoord[0]; 
  GLfloat xb = myXCoord[0]; 
  GLfloat ya = myYCoord[0]; 
  GLfloat yb = myYCoord[0]; 

  for ( int i = 0; i < myNumber; i++ )  
  {
    xa = QMIN( xa, myXCoord[i] );
    xb = QMAX( xb, myXCoord[i] );
    ya = QMIN( ya, myYCoord[i] );
    yb = QMAX( yb, myYCoord[i] );
  }
  
  myXGap = ( xb - xa ) / 10;
  myYGap = ( yb - ya ) / 10;

  myRect->setLeft( xa - myXGap );
  myRect->setTop( yb + myYGap ); 
  myRect->setRight( xb + myXGap );
  myRect->setBottom( ya - myYGap );
}

GLViewer_Drawer* GLViewer_MarkerSet::createDrawer()
{
//  cout << "GLViewer_MarkerSet::createDrawer" << endl;
  return myDrawer = new GLViewer_MarkerDrawer();
}


GLboolean GLViewer_MarkerSet::highlight( GLfloat x, GLfloat y, GLfloat tol, GLboolean isCircle )
{
    if( !myIsVisible )
        return false;
//  cout << "GLViewer_MarkerSet::highlight " << x <<" " << y << " " << tol << endl;
  int count = 0;
  GLfloat xdist, ydist, radius;
  QValueList<int>::Iterator it;
  QValueList<int> curHNumbers;
  bool isFound;
  GLboolean update;
  int cnt = 0;

  radius = tol - myMarkerSize / 2.;
  
  myUHNumbers += myHNumbers;
  myHNumbers.clear();

  for ( int i = 0; i < myNumber; i++ ) 
  {
    xdist = ( myXCoord[i] - x ) * myXScale;
    ydist = ( myYCoord[i] - y ) * myYScale;

//    if ( isCircle && ( xdist * xdist + ydist * ydist <= radius * radius ) ||
    if ( isCircle && ( xdist * xdist + ydist * ydist <= myMarkerSize * myMarkerSize ) ||
    !isCircle && ( fabs( xdist ) <= radius && fabs( ydist ) <= radius ) )
    {
      isFound = FALSE;
      count++;
      for ( it = myCurSelNumbers.begin(); it != myCurSelNumbers.end(); ++it )
        if( i == *it )
        {
          isFound = TRUE;
          curHNumbers.append( i );
        }
      
      if( !isFound )
          myHNumbers.append( i );
      else
        cnt++;
    }
  }
  myCurSelNumbers = curHNumbers;

  myIsHigh = ( GLboolean )count;
  update = ( GLboolean )( myHNumbers != myPrevHNumbers );

  myPrevHNumbers = myHNumbers;

  //cout << "GLViewer_MarkerSet::highlight complete with " << (int)myIsHigh << endl;
  return update;
}

GLboolean GLViewer_MarkerSet::unhighlight()
{
  if( !myHNumbers.isEmpty() )
  {
    myUHNumbers += myHNumbers;
    myPrevHNumbers.clear();
    myHNumbers.clear();
    //??? myCurSelNumbers.clear();
    return GL_TRUE;
  }
  
  return GL_FALSE;
}

GLboolean GLViewer_MarkerSet::select( GLfloat x, GLfloat y, GLfloat tol, GLViewer_Rect rect, GLboolean isFull,
                                      GLboolean isCircle, GLboolean isShift )
{
    if( !myIsVisible )
        return false;
//  cout << "GLViewer_MarkerSet::select " << x << " " << y << endl;
  int count = 0;
  GLfloat xdist, ydist, radius;
  QValueList<int>::Iterator it;
  QValueList<int>::Iterator it1;
  QValueList<int>::Iterator remIt;
  QValueList<int>::Iterator curIt;

  radius = tol - myMarkerSize / 2.;

  if( radius < myMarkerSize / 2.)
    radius = myMarkerSize / 2.;

  count = isShift ? mySelNumbers.count() : 0;

  myUSelNumbers = mySelNumbers;

  if ( !isShift )
  {
    mySelNumbers.clear();
    myCurSelNumbers.clear();
  }

  for ( int i = 0; i < myNumber; i++ ) 
  {
    xdist = ( myXCoord[i] - x ) * myXScale;
    ydist = ( myYCoord[i] - y ) * myYScale;

    //if ( isCircle && ( xdist * xdist + ydist * ydist <= radius * radius ) ||
    if ( isCircle && ( xdist * xdist + ydist * ydist <= myMarkerSize * myMarkerSize ) ||
          !isCircle && ( fabs( xdist ) <= radius && fabs( ydist ) <= radius ) )
    {
      count++;
      if ( isShift )
      {
        bool isFound = FALSE;
          for( it = mySelNumbers.begin(); it != mySelNumbers.end(); ++it )
            if ( *it == i )
            {
              myUSelNumbers.append( *it );
            remIt = it;
              isFound = TRUE;
              break;
            }

          if ( !isFound )
        {
          mySelNumbers.append( i );
            myCurSelNumbers.append( i );
            for ( it1 = myHNumbers.begin(); it1 != myHNumbers.end(); ++it1 )
              if( i == *it1 )
              {
                myHNumbers.remove( it1 );
                break;
              }
      for ( it1 = myUHNumbers.begin(); it1 != myUHNumbers.end(); ++it1 )
        if( i == *it1 )
        {
          myUHNumbers.remove( it1 );
          break;
        }
        }
    else
        {
      mySelNumbers.remove( remIt );
      for ( curIt = myCurSelNumbers.begin(); curIt != myCurSelNumbers.end(); ++curIt )
        if( *curIt == *remIt)
        {
          myCurSelNumbers.remove( curIt );
          break;
        }
      for ( it1 = myHNumbers.begin(); it1 != myHNumbers.end(); ++it1 )
        if( i == *it1 )
        {
          myHNumbers.remove( it1 );
          break;
        }
      for ( it1 = myUHNumbers.begin(); it1 != myUHNumbers.end(); ++it1 )
        if( i == *it1 )
        {
          myUHNumbers.remove( it1 );
          break;
        }
        }
      }
      else
      {
    mySelNumbers.append( i );
    myCurSelNumbers.append( i );
    for ( it1 = myHNumbers.begin(); it1 != myHNumbers.end(); ++it1 )
      if( i == *it1 )
      {
        myHNumbers.remove( it1 );
        break;
      }
    for ( it1 = myUHNumbers.begin(); it1 != myUHNumbers.end(); ++it1 )
      if( i == *it1 )
          {
        myUHNumbers.remove( it1 );
        break;
      }        
      }     
    }
  }

  for( it = mySelNumbers.begin(); it != mySelNumbers.end(); ++it )
    for( it1 = myUSelNumbers.begin(); it1 != myUSelNumbers.end(); ++it1 )
      if( *it == *it1 )
      {
        it1 = myUSelNumbers.remove( it1 );
        it1--;
      }
  
  myIsSel = (GLboolean)count;

//  cout << "GLViewer_MarkerSet::select complete with " << (int)myIsSel << endl;
  return myIsSel;
}

GLboolean GLViewer_MarkerSet::unselect()
{
  if( !mySelNumbers.isEmpty() )
  {
    myUSelNumbers = mySelNumbers;
    mySelNumbers.clear();
    myCurSelNumbers.clear();
    return GL_TRUE;
  }

  return GL_FALSE;
}

GLViewer_Rect* GLViewer_MarkerSet::getUpdateRect()
{
  GLViewer_Rect* rect = new GLViewer_Rect();
  
  rect->setLeft( myRect->left() + myXGap - myMarkerSize / myXScale );
  rect->setTop( myRect->top() + myYGap + myMarkerSize / myYScale ); 
  rect->setRight( myRect->right() - myXGap + myMarkerSize / myXScale );
  rect->setBottom( myRect->bottom() - myYGap - myMarkerSize / myYScale );
  //cout << " Additional tolerance " << myMarkerSize / myYScale << endl;
  //rect->setLeft( myRect->left() - myMarkerSize / myXScale );
  //rect->setTop( myRect->top() - myMarkerSize / myYScale ); 
  //rect->setRight( myRect->right() + myMarkerSize / myXScale );
  //rect->setBottom( myRect->bottom() + myMarkerSize / myYScale );
  
  return rect;
}


void GLViewer_MarkerSet::setXCoord( GLfloat* xCoord, int size )
{
  myXCoord = new GLfloat[ size ];
  for( int i = 0; i < size; i++ )
     myXCoord[i] = xCoord[i];
}

void GLViewer_MarkerSet::setYCoord( GLfloat* yCoord, int size )
{
  myYCoord = new GLfloat[ size ];
  for( int i = 0; i < size; i++ )
     myYCoord[i] = yCoord[i];
}

void GLViewer_MarkerSet::setNumMarkers( GLint number )
{
  if ( myNumber == number )
    return;
    
  if ( myXCoord && myYCoord )
  {
    delete[] myXCoord;
    delete[] myYCoord;
  }

  myNumber = number;
  myXCoord = new GLfloat[ myNumber ];
  myYCoord = new GLfloat[ myNumber ];
}
/*
void GLViewer_MarkerSet::onSelectionDone( bool append)
{
  mySelectedIndexes.Clear();
  QValueList<int>::Iterator it;
  //for( it = myMarkers->mySelNumbers.begin(); it != myMarkers->mySelNumbers.end(); ++it )
  //  mySelectedIndexes.Append( *it / 2 ); //!!!

  emit dvMarkersSelected( mySelectedIndexes );
}

void GLViewer_MarkerSet::onSelectionCancel()
{
  mySelectedIndexes.Clear();
  emit dvMarkersSelected( mySelectedIndexes );
}
*/
void GLViewer_MarkerSet::exportNumbers( QValueList<int>& highlight,
                     QValueList<int>& unhighlight,
                     QValueList<int>& select,
                     QValueList<int>& unselect )
{
    highlight = myHNumbers;
    unhighlight = myUHNumbers;
    select = mySelNumbers;
    unselect = myUSelNumbers;

    myUHNumbers = myHNumbers;
}

bool GLViewer_MarkerSet::addOrRemoveSelected( int index )
{
  if( index < 0 || index > myNumber )
    return FALSE;

  int n = mySelNumbers.findIndex( index );
  if( n == -1 )
    mySelNumbers.append( index );
  else
  {
    QValueList<int>::Iterator it;
    it = mySelNumbers.at( n );
    mySelNumbers.remove( it );
    myUSelNumbers.append( index );
  }
  return TRUE;
}

void GLViewer_MarkerSet::addSelected( const TColStd_SequenceOfInteger& seq )
{
  for ( int i = 1; i <= seq.Length(); i++ )
    if( mySelNumbers.findIndex( seq.Value( i ) ) == -1 )
      mySelNumbers.append( seq.Value( i ) - 1 );
}

void GLViewer_MarkerSet::setSelected( const TColStd_SequenceOfInteger& seq )
{
//   for( QValueList<int>::Iterator it = mySelNumbers.begin(); it != mySelNumbers.end(); ++it )
//     if( myUSelNumbers.findIndex( *it ) == -1 )
//       myUSelNumbers.append( *it );

  myUSelNumbers = mySelNumbers;
  mySelNumbers.clear();
    
  for ( int i = 1; i <= seq.Length(); i++ )
    mySelNumbers.append( seq.Value( i ) - 1 );
}

void GLViewer_MarkerSet::moveObject( float theX, float theY, bool fromGroup )
{
    if( !fromGroup && myGroup)
    {
      myGroup->dragingObjects( theX, theY );
      return;
    }
    for( int i = 0; i < myNumber;  i++ )
    {
        myXCoord[i] = myXCoord[i] + theX;
        myYCoord[i] = myYCoord[i] + theY;
    }
    compute();    
}

QByteArray GLViewer_MarkerSet::getByteCopy()
{
    int i = 0;
    int anISize = sizeof( GLint );
    int aFSize = sizeof( GLfloat );
    
    QByteArray aObject = GLViewer_Object::getByteCopy();

    QByteArray aResult( anISize + 2*aFSize*myNumber + aFSize + aObject.size());

    char* aPointer = (char*)&myNumber;
    for( i = 0; i < anISize; i++, aPointer++ )
        aResult[i] = *aPointer;

    aPointer = (char*)myXCoord;
    for( ; i < anISize + aFSize*myNumber; i++, aPointer++ )
        aResult[i] = *aPointer;
    aPointer = (char*)myYCoord;
    for( ; i < anISize + 2*aFSize*myNumber; i++, aPointer++ )
        aResult[i] = *aPointer;
    
    aPointer = (char*)&myMarkerSize;
    for( ; i < anISize + 2*aFSize*myNumber + aFSize; i++, aPointer++ )
        aResult[i] = *aPointer;
        
    
    for( ; i < aResult.size(); i++ )
        aResult[i] = aObject[i - anISize - 2*aFSize*myNumber - aFSize];

    return aResult;
}

bool GLViewer_MarkerSet::initializeFromByteCopy( QByteArray theArray )
{
    int i = 0;
    int anISize = sizeof( GLint );
    int aFSize = sizeof( GLfloat );

    char* aPointer = (char*)&myNumber;
    for( i = 0; i < anISize; i++, aPointer++ )
        *aPointer = theArray[i];

    int aSize = theArray.size();
    if( aSize < anISize + 2*aFSize*myNumber + aFSize)
        return false;

    myXCoord = new GLfloat[myNumber];
    myYCoord = new GLfloat[myNumber];
    aPointer = (char*)myXCoord;
    for( ; i < anISize + aFSize*myNumber; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)myYCoord;
    for( ; i < anISize + 2*aFSize*myNumber; i++, aPointer++ )
        *aPointer = theArray[i];

    aPointer = (char*)&myMarkerSize;
    for( ; i < anISize + 2*aFSize*myNumber + aFSize; i++, aPointer++ )
         *aPointer = theArray[i];
         
    int aCurIndex = anISize + 2*aFSize*myNumber + aFSize;
    QByteArray aObject( aSize - aCurIndex );
    for( ; i < aSize; i++ )
        aObject[i - aCurIndex] = theArray[i];
        

    if( !GLViewer_Object::initializeFromByteCopy( aObject ) || myType != "GLViewer_MarkerSet" )
        return false;

    myHNumbers.clear();
    myUHNumbers.clear();
    mySelNumbers.clear();
    myUSelNumbers.clear();
    myCurSelNumbers.clear();
    myPrevHNumbers.clear();

    return true;        
}

/***************************************************************************
**  Class:   GLViewer_Polyline
**  Descr:   OpenGL Polyline
**  Module:  GLViewer
**  Created: UI team, 03.09.02
****************************************************************************/

#define SECTIONS 100
#define DISTANTION 5

GLViewer_Polyline::GLViewer_Polyline( int number, float size, const QString& toolTip ):
  GLViewer_Object(),
  myNumber( 0 ),
  myXCoord( 0 ),
  myYCoord( 0 )       
{
  myHighFlag = GL_TRUE;

  myHNumbers.clear();
  myUHNumbers.clear();
  mySelNumbers.clear();
  myUSelNumbers.clear();
  myCurSelNumbers.clear();
  myPrevHNumbers.clear();

  setNumber( number );

  myType = "GLViewer_Polyline";
  myToolTipText = toolTip;
}

GLViewer_Polyline::~GLViewer_Polyline()
{
  if ( myXCoord )
    delete[] myXCoord;
  if ( myYCoord )
    delete[] myYCoord;
}

bool GLViewer_Polyline::translateToPS( QFile& hFile, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aPSCS )
{
    QString aBuffer = "newpath\n";

    AddLineAspectToPS( aBuffer, getAspectLine(), aViewerCS, aPSCS );

    for( int i=0; i<myNumber; i++ )
        if( i==0 )
            AddCoordsToPS( aBuffer, "moveto", aViewerCS, aPSCS, myXCoord[i], myYCoord[i] );
        else
            AddCoordsToPS( aBuffer, "lineto", aViewerCS, aPSCS, myXCoord[i], myYCoord[i] );

    if( myIsClosed )
        AddCoordsToPS( aBuffer, "lineto", aViewerCS, aPSCS, myXCoord[0], myYCoord[0] );

    aBuffer+="closepath\nstroke\n";
    
    hFile.writeBlock( aBuffer.ascii(), aBuffer.length() );

    return true;
}

bool GLViewer_Polyline::translateToHPGL( QFile& hFile, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aHPGLCS )
{
    QString aBuffer = "";
    for( int i=0; i<myNumber; i++ )
    {
        AddCoordsToHPGL( aBuffer, "PA", aViewerCS, aHPGLCS, myXCoord[i], myYCoord[i] );
        if( i==0 )
            aBuffer+="PD;\n";
    }

    if( myIsClosed )
        AddCoordsToHPGL( aBuffer, "PA", aViewerCS, aHPGLCS, myXCoord[0], myYCoord[0] );

    aBuffer+="PU;\n";
    
    hFile.writeBlock( aBuffer.ascii(), aBuffer.length() );

    return true;
}

#ifdef WIN32
bool GLViewer_Polyline::translateToEMF( HDC dc, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aEMFCS )
{
    if( !aViewerCS || !aEMFCS )
        return false;
    
    HPEN pen = AddLineAspectToEMF( dc, getAspectLine(), aViewerCS, aEMFCS );
    HGDIOBJ old = SelectObject( dc, pen );

    double x, y;
    for( int i=0; i<myNumber; i++ )
    {
        x = myXCoord[i];
        y = myYCoord[i];
        aViewerCS->transform( *aEMFCS, x, y );
        if( i==0 )
            MoveToEx( dc, x, y, NULL );
        else
            LineTo( dc, x, y );
    }

    if( myIsClosed )
    {
        x = myXCoord[0];
        y = myYCoord[0];
        aViewerCS->transform( *aEMFCS, x, y );
        LineTo( dc, x, y );
    }

    SelectObject( dc, old );
    if( pen )
        DeleteObject( pen );

    return true;
}
#endif

void GLViewer_Polyline::compute()
{
//  cout << "GLViewer_MarkerSet::compute" << endl;
  GLfloat xa = myXCoord[0]; 
  GLfloat xb = myXCoord[0]; 
  GLfloat ya = myYCoord[0]; 
  GLfloat yb = myYCoord[0]; 

  for ( int i = 0; i < myNumber; i++ )  
  {
    xa = QMIN( xa, myXCoord[i] );
    xb = QMAX( xb, myXCoord[i] );
    ya = QMIN( ya, myYCoord[i] );
    yb = QMAX( yb, myYCoord[i] );
  }

  GLfloat xGap = ( xb - xa ) / 10;
  GLfloat yGap = ( yb - ya ) / 10;

  myRect->setLeft( xa - xGap );
  myRect->setTop( yb + yGap ); 
  myRect->setRight( xb + xGap );
  myRect->setBottom( ya - yGap );
}

GLViewer_Rect* GLViewer_Polyline::getUpdateRect()
{
    GLViewer_Rect* rect = new GLViewer_Rect();

    rect->setLeft( myRect->left() - myXGap );
    rect->setTop( myRect->top() + myYGap ); 
    rect->setRight( myRect->right() + myXGap );
    rect->setBottom( myRect->bottom() - myYGap );

    return rect;
}

GLViewer_Drawer* GLViewer_Polyline::createDrawer()
{
//  cout << "GLViewer_MarkerSet::createDrawer" << endl;
    return myDrawer = new GLViewer_PolylineDrawer();
}

GLboolean GLViewer_Polyline::highlight( GLfloat x, GLfloat y, GLfloat tol, GLboolean isCircle )
{
    if( !myIsVisible )
        return false;
    GLfloat xa, xb, ya, yb, l;
    GLfloat rsin, rcos, r, ra, rb;
    GLboolean update;
    GLboolean highlighted = myIsHigh;

    myIsHigh = GL_FALSE;

    int c = 0;
    if( myIsClosed )
        c = 1;

    for( int i = 0; i < myNumber-1+c; i++ ) 
    {
        xa = myXCoord[i];
        ya = myYCoord[i];
        if( i != myNumber-1 )
        {
              xb = myXCoord[i+1];
              yb = myYCoord[i+1];
        }
        else
        {    
              xb = myXCoord[0];      
              yb = myYCoord[0];
        }

        l = sqrt( (xb-xa)*(xb-xa) + (yb-ya)*(yb-ya) );
        rsin = (yb-ya) / l;
        rcos = (xb-xa) / l;
        r = ( (x-xa)*(y-yb) - (x-xb)*(y-ya) ) / ( rsin*(ya-yb) + rcos*(xa-xb) );
        ra = sqrt( (x-xa)*(x-xa) + (y-ya)*(y-ya) );
        rb = sqrt( (x-xb)*(x-xb) + (y-yb)*(y-yb) );
        if( fabs( r ) * myXScale <= DISTANTION && ra <= l + DISTANTION && rb <= l + DISTANTION )
        {
            myIsHigh = GL_TRUE;
            break;
        }
    }

    if( !myHighFlag && myIsHigh )
        myIsHigh = GL_FALSE;
    else
        myHighFlag = GL_TRUE;

    update = ( GLboolean )( myIsHigh != highlighted );

//  cout << "GLViewer_Polyline::highlight complete with " << (int)myIsHigh << endl;
    return update;
}

GLboolean GLViewer_Polyline::unhighlight()
{
//   if( !myHNumbers.isEmpty() )
//   {
//     myUHNumbers = myHNumbers;
//     myHNumbers.clear();
//     return GL_TRUE;
//   }

  if( myIsHigh )
  {
    myIsHigh = GL_FALSE;
    return GL_TRUE;
  }

  return GL_FALSE;
}

GLboolean GLViewer_Polyline::select( GLfloat x, GLfloat y, GLfloat tol, GLViewer_Rect rect, GLboolean isFull,
                                     GLboolean isCircle, GLboolean isShift )
{
    if( !myIsVisible )
        return false;
    GLfloat xa, xb, ya, yb, l;
    GLfloat rsin, rcos, r, ra, rb;
    GLboolean update;
    GLboolean selected = myIsSel;

    myIsSel = GL_FALSE;

    int c = 0;
    if( myIsClosed )
        c = 1;

    for( int i = 0; i < myNumber-1+c; i++ ) 
    {
        xa = myXCoord[i];
        ya = myYCoord[i];
        if( i != myNumber-1 )
        {
            xb = myXCoord[i+1];
            yb = myYCoord[i+1];
        }
        else
        {
            xb = myXCoord[0];
            yb = myYCoord[0];
        }

        l = sqrt( (xb-xa)*(xb-xa) + (yb-ya)*(yb-ya) );
        rsin = (yb-ya) / l;
        rcos = (xb-xa) / l;
        r = ( (x-xa)*(y-yb) - (x-xb)*(y-ya) ) / ( rsin*(ya-yb) + rcos*(xa-xb) );
        ra = sqrt( (x-xa)*(x-xa) + (y-ya)*(y-ya) );
        rb = sqrt( (x-xb)*(x-xb) + (y-yb)*(y-yb) );
        if( fabs( r ) * myXScale <= DISTANTION && ra <= l + DISTANTION && rb <= l + DISTANTION )
        {
            myIsSel = GL_TRUE;
            break;
        }
    }

    if ( myIsSel )
    {
        myHighFlag = GL_FALSE;
        myIsHigh = GL_FALSE;
    }
    else
        myHighFlag = GL_TRUE;

    update = ( GLboolean )( myIsSel != selected );

    //  cout << "GLViewer_Polyline::select complete with " << (int)myIsSel << endl;

    //  return update;  !!!!!!!!!!!!!!!!!!!!!!!!!!! no here
    return myIsSel;
}

GLboolean GLViewer_Polyline::unselect()
{
//   if( !mySelNumbers.isEmpty() )
//   {
//     myUSelNumbers = mySelNumbers;
//     mySelNumbers.clear();
//     myCurSelNumbers.clear();
//     return GL_TRUE;
//   }

  if( myIsSel )
  {
    myIsSel = GL_FALSE;
    return GL_TRUE;
  }

  return GL_FALSE;
}

void GLViewer_Polyline::setXCoord( GLfloat* xCoord, int size )
{
  myXCoord = new GLfloat[ size ];
  for( int i = 0; i < size; i++ )
     myXCoord[i] = xCoord[i];
}

void GLViewer_Polyline::setYCoord( GLfloat* yCoord, int size )
{
  myYCoord = new GLfloat[ size ];
  for( int i = 0; i < size; i++ )
     myYCoord[i] = yCoord[i];
}

void GLViewer_Polyline::setNumber( GLint number )
{
  if ( myNumber == number )
    return;
    
  if ( myXCoord && myYCoord )
  {
    delete[] myXCoord;
    delete[] myYCoord;
  }

  myNumber = number;
  myXCoord = new GLfloat[ myNumber ];
  myYCoord = new GLfloat[ myNumber ];
}
/*
void GLViewer_Polyline::onSelectionDone( bool append)
{
  mySelectedIndexes.Clear();
  QValueList<int>::Iterator it;
  //for( it = myMarkers->mySelNumbers.begin(); it != myMarkers->mySelNumbers.end(); ++it )
  //  mySelectedIndexes.Append( *it / 2 ); //!!!
}

void GLViewer_Polyline::onSelectionCancel()
{
  mySelectedIndexes.Clear();
}
*/
void GLViewer_Polyline::exportNumbers( QValueList<int>& highlight,
                     QValueList<int>& unhighlight,
                     QValueList<int>& select,
                     QValueList<int>& unselect )
{
  highlight = myHNumbers;
  unhighlight = myUHNumbers;
  select = mySelNumbers;
  unselect = myUSelNumbers;
}

void GLViewer_Polyline::moveObject( float theX, float theY, bool fromGroup )
{
  if( !fromGroup && myGroup)
  {
    myGroup->dragingObjects( theX, theY );
    return;
  }
  for( int i = 0; i < myNumber;  i++ )
  {
      myXCoord[i] = myXCoord[i] + theX;
      myYCoord[i] = myYCoord[i] + theY;
  }
  compute();    
}

QByteArray GLViewer_Polyline::getByteCopy()
{
    int i = 0;
    int anISize = sizeof( GLint );
    int aFSize = sizeof( GLfloat );
    int aBSize = sizeof( GLboolean );

    QByteArray aObject = GLViewer_Object::getByteCopy();

    QByteArray aResult( aFSize*myNumber*2 + anISize + 2*aBSize + aObject.size());

    char* aPointer = (char*)&myNumber;
    for( i = 0; i < anISize; i++, aPointer++ )
        aResult[i] = *aPointer;

    aPointer = (char*)myXCoord;
    for( ; i < anISize + aFSize*myNumber; i++, aPointer++ )
        aResult[i] = *aPointer;
    aPointer = (char*)myYCoord;
    for( ; i < anISize + 2*aFSize*myNumber; i++, aPointer++ )
        aResult[i] = *aPointer;
    
    aPointer = (char*)&myIsClosed;
    for( ; i < anISize + 2*aFSize*myNumber + aBSize; i++, aPointer++ )
        aResult[i] = *aPointer;
    aPointer = (char*)&myHighSelAll;
    for( ; i < anISize + 2*aFSize*myNumber + 2*aBSize; i++, aPointer++ )
        aResult[i] = *aPointer;

    for( ; i < aResult.size(); i++ )
        aResult[i] = aObject[i - anISize - 2*aFSize*myNumber - 2*aBSize];

    return aResult;
}


bool GLViewer_Polyline::initializeFromByteCopy( QByteArray theArray )
{
    int i = 0;
    int anISize = sizeof( GLint );
    int aFSize = sizeof( GLfloat );
    int aBSize = sizeof( GLboolean );

    char* aPointer = (char*)&myNumber;
    for( i = 0; i < anISize; i++, aPointer++ )
        *aPointer = theArray[i];

    int aSize = theArray.size();
    if( aSize < aFSize*myNumber*2 + anISize + 2*aBSize )
        return false;

    myXCoord = new GLfloat[myNumber];
    myYCoord = new GLfloat[myNumber];
    aPointer = (char*)myXCoord;
    for( ; i < anISize + aFSize*myNumber; i++, aPointer++ )
        *aPointer = theArray[i];
    aPointer = (char*)myYCoord;
    for( ; i < anISize + 2*aFSize*myNumber; i++, aPointer++ )
        *aPointer = theArray[i];

    aPointer = (char*)&myIsClosed;
    for( ; i < anISize + 2*aFSize*myNumber + aBSize; i++, aPointer++ )
         *aPointer = theArray[i];
    aPointer = (char*)&myHighSelAll;
    for( ; i < anISize + 2*aFSize*myNumber + 2*aBSize; i++, aPointer++ )
         *aPointer = theArray[i];

    int aCurIndex = anISize + 2*aFSize*myNumber + 2*aBSize;
    QByteArray aObject( aSize - aCurIndex );
    for( ; i < aSize; i++ )
        aObject[i - aCurIndex] = theArray[i];

    if( !GLViewer_Object::initializeFromByteCopy( aObject ) || myType != "GLViewer_Polyline" )
        return false;

    myHNumbers.clear();
    myUHNumbers.clear();
    mySelNumbers.clear();
    myUSelNumbers.clear();
    myCurSelNumbers.clear();
    myPrevHNumbers.clear();

    return true;        
}

/***************************************************************************
**  Class:   GLViewer_TextObject
**  Descr:   Text as Object for OpenGL
**  Module:  GLViewer
**  Created: UI team, 12.02.04
****************************************************************************/

GLViewer_TextObject::GLViewer_TextObject( const QString& theStr, float xPos, float yPos, 
                                    const QColor& color, const QString& toolTip )
                                    : GLViewer_Object()
{
    myGLText = new GLViewer_Text( theStr, xPos, yPos, color );
    myWidth = 0;
    myHeight = 0;

    myHighFlag = GL_TRUE;

    myToolTipText = toolTip;
}
GLViewer_TextObject::~GLViewer_TextObject()
{
  if ( myGLText )
    delete myGLText;
}

bool GLViewer_TextObject::translateToPS( QFile& hFile, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aPSCS )
{
    QString aText = myGLText->getText();    
    float xPos, yPos;
    myGLText->getPosition( xPos, yPos );

    QString aBuffer = "/Times-Roman findfont\n";
    aBuffer += "12 scalefont setfont\n";

    AddCoordsToPS( aBuffer, "moveto", aViewerCS, aPSCS, double(xPos), double(yPos) );
    aBuffer += "(" + aText + ") show\n";

    hFile.writeBlock( aBuffer.ascii(), aBuffer.length() );

    return true;
}

bool GLViewer_TextObject::translateToHPGL( QFile& hFile, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aHPGLCS )
{
    QString aText = myGLText->getText();    
    float xPos, yPos;
    myGLText->getPosition( xPos, yPos );

    QString aBuffer = "";
    AddCoordsToHPGL( aBuffer, "PA", aViewerCS, aHPGLCS, double(xPos), double(yPos) );
    
    aBuffer = "LB" + aText + "#;";
    
    hFile.writeBlock( aBuffer.ascii(), aBuffer.length() );

    return true;
}

#ifdef WIN32
bool GLViewer_TextObject::translateToEMF( HDC dc, GLViewer_CoordSystem* aViewerCS, GLViewer_CoordSystem* aEMFCS )
{
    QString aText = myGLText->getText();    
    float xPos, yPos;
    myGLText->getPosition( xPos, yPos );

    double x = double( xPos ), 
           y = double( yPos );

    aViewerCS->transform( *aEMFCS, x, y );
    const char* str = aText.ascii();

    int nHeight = 35*14;       // height of font
    int nWidth = 35*12;        // average character width
    int nEscapement = 0;       // angle of escapement
    int nOrientation = 0;      // base-line orientation angle
    int fnWeight = FW_NORMAL;  // font weight
    DWORD fdwItalic = FALSE;    // italic attribute option
    DWORD fdwUnderline = FALSE; // underline attribute option
    DWORD fdwStrikeOut = FALSE; // strikeout attribute option
    DWORD fdwCharSet = ANSI_CHARSET; // character set identifier
    DWORD fdwOutputPrecision = OUT_DEFAULT_PRECIS;  // output precision
    DWORD fdwClipPrecision = CLIP_DEFAULT_PRECIS;    // clipping precision
    DWORD fdwQuality = PROOF_QUALITY;          // output quality
    DWORD fdwPitchAndFamily = FIXED_PITCH | FF_DONTCARE;   // pitch and family
    LPCTSTR lpszFace = NULL;         // typeface name


    HFONT aFont = CreateFont( nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic,
                              fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, 
                              fdwClipPrecision, fdwQuality, fdwPitchAndFamily, lpszFace );
    LOGBRUSH aBrushData;
    aBrushData.lbStyle = BS_HOLLOW;

    HBRUSH aBrush = CreateBrushIndirect( &aBrushData );

    HGDIOBJ old1 = SelectObject( dc, aFont );
    HGDIOBJ old2 = SelectObject( dc, aBrush );

    TextOut( dc, x, y, str, aText.length() );

    SelectObject ( dc, old1 );
    SelectObject ( dc, old2 );

    DeleteObject( aFont );

    return true;
}
#endif

GLViewer_Drawer* GLViewer_TextObject::createDrawer()
{
    myDrawer = new GLViewer_TextDrawer();
    compute();
    return myDrawer;
}

void GLViewer_TextObject::compute()
{
    float xPos, yPos;
    QString aStr = myGLText->getText();
    myGLText->getPosition( xPos, yPos );

    myWidth = myGLText->getWidth();
    myHeight = myGLText->getHeight();
    myRect->setLeft( xPos );
    myRect->setTop( yPos + myHeight  ); 
    myRect->setRight( xPos + myWidth );
    myRect->setBottom( yPos );
}

void GLViewer_TextObject::setDrawer( GLViewer_Drawer* theDrawer )
{
    myDrawer = theDrawer;
    //compute();
}

GLViewer_Rect* GLViewer_TextObject::getUpdateRect()
{    
    GLViewer_Rect* rect = new GLViewer_Rect();

    float xPos, yPos;
    QString aStr = myGLText->getText();
    myGLText->getPosition( xPos, yPos );

    rect->setLeft( myRect->left() + myXGap - myWidth / myXScale );
    rect->setTop( myRect->top() + myYGap + myHeight / myYScale );
    rect->setRight( myRect->right() - myXGap + myWidth / myXScale );
    rect->setBottom( myRect->bottom() - myYGap - myHeight / myYScale );

    return rect;
}

GLboolean GLViewer_TextObject::highlight( GLfloat theX, GLfloat theY, GLfloat theTol, GLboolean isCircle )
{
    if( !myIsVisible )
        return false;

    float xPos, yPos;
    myGLText->getPosition( xPos, yPos );

    QRect aRect;
    aRect.setLeft( (int)xPos );
    aRect.setRight( (int)(xPos + myWidth / myXScale) );
    aRect.setTop( (int)yPos );// - myHeight / myYScale );
    aRect.setBottom( (int)(yPos + myHeight / myYScale) );

    //cout << "theX: " << theX << "  theY: " << theY << endl;
    //cout << "aRect.left(): " << aRect.left() << "  aRect.right(): " << aRect.right() << endl;
    //cout << "aRect.top(): " << aRect.top() << "  aRect.bottom(): " << aRect.bottom() << endl;

    QRegion obj( aRect );
    QRegion intersection;
    QRect region;

    region.setLeft( (int)(theX - theTol) );
    region.setRight( (int)(theX + theTol) );
    region.setTop( (int)(theY - theTol) );
    region.setBottom( (int)(theY + theTol) );

    QRegion circle( (int)(theX - theTol), (int)(theY - theTol),
                      (int)(2 * theTol), (int)(2 * theTol), QRegion::Ellipse );
    if( isCircle )
        intersection = obj.intersect( circle );
    else
        intersection = obj.intersect( region );
    
    if( intersection.isEmpty() )
        myIsHigh = false;
    else
        myIsHigh = true;
    
    if( !myHighFlag && myIsHigh )
        myIsHigh = GL_FALSE;
    else
        myHighFlag = GL_TRUE;

    return myIsHigh;
}

GLboolean GLViewer_TextObject::unhighlight()
{
    if( myIsHigh )
    {
        myIsHigh = GL_FALSE;
        return GL_TRUE;
    }

    return GL_FALSE;
}

GLboolean GLViewer_TextObject::select( GLfloat theX, GLfloat theY, GLfloat theTol, GLViewer_Rect rect,
                                       GLboolean isFull, GLboolean isCircle, GLboolean isShift )
{ 
    if( !myIsVisible )
        return false;

    QRegion obj( myRect->toQRect() );
    QRegion intersection;
    QRect region;

    region.setLeft( (int)(theX - theTol) );
    region.setRight( (int)(theX + theTol) );
    region.setTop( (int)(theY - theTol) );
    region.setBottom( (int)(theY + theTol) );

    QRegion circle( (int)(theX - theTol), (int)(theY - theTol),
                      (int)(2 * theTol), (int)(2 * theTol), QRegion::Ellipse );
    if( isCircle )
        intersection = obj.intersect( circle );
    else
        intersection = obj.intersect( region );
    
    if( intersection.isEmpty() )
        myIsSel = false;
    else
        myIsSel = true;

    if ( myIsSel )
    {
        myHighFlag = GL_FALSE;
        myIsHigh = GL_FALSE;
    }
    else
        myHighFlag = GL_TRUE;

    return myIsSel;
}

GLboolean GLViewer_TextObject::unselect()
{
    if( myIsSel )
    {
        myIsSel = GL_FALSE;
        return GL_TRUE;
    }

    return GL_FALSE;
}

void GLViewer_TextObject::moveObject( float theX, float theY, bool fromGroup )
{
  if( !fromGroup && myGroup)
  {
    myGroup->dragingObjects( theX, theY );
    return;
  }
  float aX, anY;
  myGLText->getPosition( aX, anY );
  aX += theX;
  anY += theY;
  myGLText->setPosition( aX, anY );
  compute();
}

QByteArray GLViewer_TextObject::getByteCopy()
{
    QByteArray aObject = GLViewer_Object::getByteCopy();

    return aObject;
}

bool GLViewer_TextObject::initializeFromByteCopy( QByteArray theArray )
{
    if( !GLViewer_Object::initializeFromByteCopy( theArray ) || myType != "GLViewer_TextObject" )
        return false;

    myHighFlag = true;
    return true;        
}
