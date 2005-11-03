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
**  Class:   GLViewer_Text
**  Descr:   Substitution of Prs3d_Text for OpenGL
**  Module:  GLViewer
**  Created: UI team, 10.07.03
****************************************************************************/

//#include <GLViewerAfx.h>
#include "GLViewer_Text.h"

GLViewer_Text::GLViewer_Text( const QString& text, float xPos, float yPos, const QColor& color )
{
  myText = text;
  myXPos = xPos;
  myYPos = yPos;
  myColor = color;
  myQFont = QFont::defaultFont();
  mySeparator = 2;
  myDTF = DTF_BITMAP;
}

GLViewer_Text::GLViewer_Text( const QString& text, float xPos, float yPos, const QColor& color, QFont theFont, int theSeparator )
{
  myText = text;
  myXPos = xPos;
  myYPos = yPos;
  myColor = color;
  myQFont = theFont;
  mySeparator = theSeparator;
  myDTF = DTF_BITMAP;
}

GLViewer_Text::~GLViewer_Text()
{
}

int GLViewer_Text::getWidth()
{
    int aResult = 0;
    QFontMetrics aFM( myQFont );
    for( uint i = 0; i < myText.length(); i++ )
        aResult += aFM.width( myText.at(i) ) + mySeparator;
    return aResult;
}

int GLViewer_Text::getHeight()
{
    QFontMetrics aFM( myQFont );
    return aFM.height();
}

QByteArray GLViewer_Text::getByteCopy() const
{
    int i;
    int aSize = 5*sizeof( int ) + myText.length();

    int aR = myColor.red();
    int aG = myColor.green();
    int aB = myColor.blue();
    const char* aStr = myText.data();

    int anISize = sizeof( int );    
    QByteArray aResult( aSize );

    char* aPointer = (char*)&myXPos;
    for( i = 0; i < anISize; i++, aPointer++ )
        aResult[i] = *aPointer;
    aPointer = (char*)&myYPos;
    for( ; i < 2*anISize; i++, aPointer++ )
        aResult[i] = *aPointer;

    aPointer = (char*)&aR;
    for( ; i < 3*anISize; i++, aPointer++ )
        aResult[i] = *aPointer;
    aPointer = (char*)&aG;
    for( ; i < 4*anISize; i++, aPointer++ )
        aResult[i] = *aPointer;
    aPointer = (char*)&aB;
    for( ; i < 5*anISize; i++, aPointer++ )
        aResult[i] = *aPointer;

    int aTextSize = myText.length();
    aPointer = (char*)&aTextSize;
    for( ; i < 6*anISize; i++, aPointer++ )
        aResult[i] = *aPointer;

    for( i = 0; i < aTextSize; i++ )
        aResult[6*anISize + i] = aStr[i];

    aPointer = (char*)&mySeparator;
    for( ; i < 7*anISize + aTextSize; i++, aPointer++ )
        aResult[i] = *aPointer;

    const char* aFontStr = myQFont.toString().data();
    int aFontSize = myQFont.toString().length();

    for( i = 0; i < aFontSize; i++ )
        aResult[7*anISize + aTextSize + i] = aFontStr[i];

    return aResult;
}

GLViewer_Text* GLViewer_Text::fromByteCopy( QByteArray theBuf )
{
    int i = 0;
    int aSize = (int)theBuf.size();
    int aR = 0, aG = 0, aB = 0;

    int xPos = 0, yPos = 0;

    int anISize = sizeof( int );
    char* aPointer = (char*)&xPos;
    for ( i = 0; i < anISize; i++, aPointer++ )
        *aPointer = theBuf[i];

    aPointer = (char*)&yPos;
    for ( ; i < 2*anISize; i++, aPointer++ )
        *aPointer = theBuf[i];

    aPointer = (char*)&aR;
    for( ; i < 3*anISize; i++, aPointer++ )
        *aPointer = theBuf[i];
    aPointer = (char*)&aG;
    for( ; i < 4*anISize; i++, aPointer++ )
        *aPointer = theBuf[i];
    aPointer = (char*)&aB;
    for( ; i < 5*anISize; i++, aPointer++ )
        *aPointer = theBuf[i];

    int aTextSize = 0;
    aPointer = (char*)&aTextSize;
    for( ; i < 6*anISize; i++, aPointer++ )
        *aPointer = theBuf[i];

    QString aText;
    for( ; i < 6*anISize + aTextSize; i++ )
    {
        QChar aChar( theBuf[i] );
        aText += aChar;
    }

    int aSeparator = 0;
    aPointer = (char*)&aSeparator;
    for( ; i < 7*anISize + aTextSize; i++, aPointer++ )
        *aPointer = theBuf[i];

    QString aFontStr;
    for( ; i < aSize; i++ )
    {
        QChar aChar( theBuf[i] );
        aFontStr += aChar;
    }
    QFont aFont;

    if( !aFont.fromString( aFontStr ) )
        return NULL;    

    GLViewer_Text* aGlText = new GLViewer_Text( aText, xPos, yPos, QColor( aR,aG,aB ), aFont, aSeparator  );

    return aGlText;    
}
