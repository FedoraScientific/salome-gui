// File:      GLViewer_BaseDrawers.cxx
// Created:   November, 2004
// Author:    OCC team
// Copyright (C) CEA 2004

//#include <GLViewerAfx.h>
#include "GLViewer_BaseDrawers.h"
#include "GLViewer_Object.h"
#include "GLViewer_Text.h"
#include "GLViewer_AspectLine.h"
#include "GLViewer_BaseObjects.h"

#ifndef WIN32
#include <GL/glx.h>
#endif

/***************************************************************************
**  Class:   GLViewer_MarkerDrawer
**  Descr:   Drawer for GLViewer_MarkerSet
**  Module:  GLViewer
**  Created: UI team, 03.10.01
****************************************************************************/

GLfloat sin_table[SEGMENTS];
GLfloat cos_table[SEGMENTS];

GLViewer_MarkerDrawer::GLViewer_MarkerDrawer()
: GLViewer_Drawer()
{
    GLfloat angle = 0.0;
    for ( int i = 0; i < SEGMENTS; i++ )
    {
        sin_table[i] = sin( angle );
        cos_table[i] = cos( angle );
        angle += float( STEP );
    }
    myObjectType = "GLViewer_MarkerSet";
}

GLViewer_MarkerDrawer::~GLViewer_MarkerDrawer()
{
}

void GLViewer_MarkerDrawer::create( float xScale, float yScale, bool onlyUpdate )
{
    QValueList<int>::Iterator it;
    QValueList<int>::Iterator EndIt;
    QValueList<GLViewer_Object*>::Iterator anObjectIt = myObjects.begin();
    QValueList<GLViewer_Object*>::Iterator anEndObjectIt = myObjects.end();

    myXScale = xScale;
    myYScale = yScale;

    QColor colorN, colorH, colorS;

    GLViewer_MarkerSet* aMarkerSet = NULL;
    GLViewer_AspectLine* anAspectLine = NULL;

    for( ; anObjectIt != anEndObjectIt; anObjectIt++ )
    {
        aMarkerSet = ( GLViewer_MarkerSet* )(*anObjectIt);
        anAspectLine = aMarkerSet->getAspectLine();
        anAspectLine->getLineColors( colorN, colorH, colorS );

        float* aXCoord = aMarkerSet->getXCoord();
        float* anYCoord = aMarkerSet->getYCoord();
        float aRadius = aMarkerSet->getMarkerSize();

        QValueList<int> aHNumbers, anUHNumbers, aSelNumbers, anUSelNumbers;
        aMarkerSet->exportNumbers( aHNumbers, anUHNumbers, aSelNumbers, anUSelNumbers );

        if( onlyUpdate )
        {
            EndIt = anUHNumbers.end();
            for( it = anUHNumbers.begin(); it != EndIt; ++it )
            {
                drawMarker( aXCoord[*it], anYCoord[*it], aRadius, colorN, anAspectLine );
            }

            EndIt = anUSelNumbers.end();
            for( it = anUSelNumbers.begin(); it != EndIt; ++it )
                drawMarker( aXCoord[*it], anYCoord[*it], aRadius, colorN, anAspectLine );

            EndIt = aSelNumbers.end();
            for( it = aSelNumbers.begin(); it != EndIt; ++it )
                drawMarker( aXCoord[*it], anYCoord[*it], aRadius, colorS, anAspectLine );

            EndIt = aHNumbers.end();
            for( it = aHNumbers.begin(); it != EndIt; ++it )
            {
                drawMarker( aXCoord[*it], anYCoord[*it], aRadius, colorH, anAspectLine );
            }
        }
        else
        {
            int aNumber = aMarkerSet->getNumMarkers();
            for( int i = 0; i < aNumber; i++ )
                drawMarker( aXCoord[i], anYCoord[i], aRadius, colorN, anAspectLine );

            EndIt = anUSelNumbers.end();
            for( it = anUSelNumbers.begin(); it != EndIt; ++it )
                drawMarker( aXCoord[*it], anYCoord[*it], aRadius, colorN, anAspectLine );

            EndIt = aSelNumbers.end();
            for( it = aSelNumbers.begin(); it != EndIt; ++it )
                drawMarker( aXCoord[*it], anYCoord[*it], aRadius, colorS, anAspectLine );
        }
        if( aMarkerSet->getGLText()->getText() != "" )
        {
            //float aXPos = 0, anYPos = 0;
            //aMarkerSet->getGLText()->getPosition( aXPos, anYPos );
            //drawText( aMarkerSet->getGLText()->getText(), aXPos, anYPos, colorN, &aMarkerSet->getGLText()->getFont(), aMarkerSet->getGLText()->getSeparator() );
            drawText( aMarkerSet );
        }
    }
}

void GLViewer_MarkerDrawer::drawMarker( float& theXCoord, float& theYCoord,
                                     float& theRadius, QColor& theColor, GLViewer_AspectLine* theAspectLine )
{
    glColor3f( ( GLfloat )theColor.red() / 255, 
               ( GLfloat )theColor.green() / 255, 
               ( GLfloat )theColor.blue() / 255 );

    glLineWidth( theAspectLine->getLineWidth() );

    if ( theAspectLine->getLineType() == 0 )
        glBegin( GL_LINE_LOOP );
    else
        glBegin( GL_LINE_STRIP);

    for ( int i = 0; i < SEGMENTS; i++ )
        glVertex2f( theXCoord + cos_table[i] * theRadius / myXScale,
                    theYCoord + sin_table[i] * theRadius / myYScale );
    glEnd();
}

/***************************************************************************
**  Class:   GLViewer_PolylineDrawer
**  Descr:   Drawer for GLViewer_Polyline
**  Module:  GLViewer
**  Created: UI team, 03.10.01
****************************************************************************/

GLViewer_PolylineDrawer::GLViewer_PolylineDrawer()
:GLViewer_Drawer()
{
    myObjectType = "GLViewer_Polyline";
}

GLViewer_PolylineDrawer::~GLViewer_PolylineDrawer()
{
}

void GLViewer_PolylineDrawer::create( float xScale, float yScale, bool onlyUpdate )
{
    QValueList<GLViewer_Object*>::Iterator aObjectIt = myObjects.begin();
    QValueList<GLViewer_Object*>::Iterator aObjectEndIt = myObjects.end();
    
    myXScale = xScale;
    myYScale = yScale;

    QColor color, colorN, colorH, colorS;
    GLViewer_AspectLine* anAspect = NULL;
    GLViewer_Polyline* aPolyline = NULL;
    for( ; aObjectIt != aObjectEndIt; aObjectIt++ )
    {
        anAspect = (*aObjectIt)->getAspectLine();
        aPolyline = (GLViewer_Polyline*)(*aObjectIt);


        anAspect->getLineColors( colorN, colorH, colorS );
        if( onlyUpdate )
        {
            if( aPolyline->isHighlighted() )
                color = colorH;
            else if( aPolyline->isSelected() )
                color = colorS;
            else
                color = colorN;
        }
        else
        {
            if( aPolyline->isSelected() )
                color = colorS;
            else
                color = colorN;
        }

        float* aXCoord = aPolyline->getXCoord();
        float* anYCoord = aPolyline->getYCoord();
        int aSize = aPolyline->getNumber();        

        glColor3f( ( GLfloat )color.red() / 255, 
                   ( GLfloat )color.green() / 255, 
                   ( GLfloat )color.blue() / 255 );

        glLineWidth( anAspect->getLineWidth() );

        if ( anAspect->getLineType() == 0 )
            glBegin( GL_LINE_LOOP );
        else
            glBegin( GL_LINE_STRIP);

        for( int i = 0; i < aSize ; i++ )
             glVertex2f( aXCoord[ i ], anYCoord[ i ] );        
 
        if( aPolyline->isClosed() )
            glVertex2f( aXCoord[ 0 ], anYCoord[ 0 ] );

        glEnd();       

        if( aPolyline->getGLText()->getText() != "" )
        {
            //float aXPos = 0, anYPos = 0;
            //aPolyline->getGLText()->getPosition( aXPos, anYPos );
            //drawText( aPolyline->getGLText()->getText(), aXPos, anYPos, color, &aPolyline->getGLText()->getFont(), aPolyline->getGLText()->getSeparator() );
          drawText( aPolyline );
        }
    }
}

/***************************************************************************
**  Class:   GLViewer_TextDrawer
**  Descr:   
**  Module:  GLViewer
**  Created: UI team, 27.02.04
****************************************************************************/

GLViewer_TextDrawer::GLViewer_TextDrawer()
: GLViewer_Drawer()
{
    myObjectType = "GLViewer_TextObject";
}

GLViewer_TextDrawer::~GLViewer_TextDrawer()
{
}

void GLViewer_TextDrawer::create( float xScale, float yScale, bool onlyUpdate )
{
    QValueList<GLViewer_Object*>::Iterator aObjectIt = myObjects.begin();
    QValueList<GLViewer_Object*>::Iterator aObjectEndIt = myObjects.end();
    
    myXScale = xScale;
    myYScale = yScale;

    QColor color, colorN, colorH, colorS;
    GLViewer_AspectLine* anAspect = NULL;    
    GLViewer_TextObject* anObject = NULL;
    //float aXPos = 0, anYPos = 0;
    for( ; aObjectIt != aObjectEndIt; aObjectIt++ )
    {
        anObject = (GLViewer_TextObject*)(*aObjectIt);
        anAspect = anObject->getAspectLine();    

        anAspect->getLineColors( colorN, colorH, colorS );
        if( onlyUpdate )
        {
            if( anObject->isHighlighted() )
                color = colorH;
            else if( anObject->isSelected() )
                color = colorS;
            else
                color = colorN;
        }
        else
        {
            if( anObject->isSelected() )
                color = colorS;
            else
                color = colorN;
        }        
        
        //anObject->getGLText()->getPosition( aXPos, anYPos );
        //drawText( anObject->getGLText()->getText(), aXPos, anYPos, color, &(anObject->getGLText()->getFont()), anObject->getGLText()->getSeparator() );
        drawText( anObject );
    }
}

void GLViewer_TextDrawer::updateObjects()
{
    QValueList<GLViewer_Object*>::Iterator aObjectIt = myObjects.begin();
    QValueList<GLViewer_Object*>::Iterator aObjectEndIt = myObjects.end();
    for( ; aObjectIt != aObjectEndIt; aObjectIt++ )
        (*aObjectIt)->compute();
}
