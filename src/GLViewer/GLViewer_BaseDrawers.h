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

// File:      GLViewer_BaseDrawers.h
// Created:   November, 2004

#ifndef GLVIEWER_BASEDRAWERS_H
#define GLVIEWER_BASEDRAWERS_H

#include <qcolor.h>
#include <qobject.h>
#include <qfile.h>
#include <qfont.h>
#include <qgl.h>

#include "GLViewer.h"
#include "GLViewer_Drawer.h"

class GLViewer_AspectLine;

#ifdef WNT
#pragma warning( disable:4251 )
#endif

/*! 
  \class GLViewer_MarkerDrawer
  Drawer for GLViewer_MarkerSet
*/

class GLVIEWER_API GLViewer_MarkerDrawer : public GLViewer_Drawer  
{
public:
  GLViewer_MarkerDrawer();
  ~GLViewer_MarkerDrawer();
  
  //! Redefined method
  virtual void       create( float, float, bool );
  
private:
  //! Draws marker in point (x,y) of \param radius with \param color and \param aspect
  void               drawMarker( float& x, float& y, float& radius, QColor& color, GLViewer_AspectLine* aspect );
};

/*!
  \class  GLViewer_PolylineDrawer
  Drawer for GLViewer_Polyline
*/

class GLVIEWER_API GLViewer_PolylineDrawer : public GLViewer_Drawer  
{
public:
  GLViewer_PolylineDrawer();
  ~GLViewer_PolylineDrawer();
  //! Redefined method
  virtual void       create( float, float, bool );    
};

/*!
   \class GLViewer_TextDrawer
   Drawer for GLViewer_Text
*/

class GLVIEWER_API GLViewer_TextDrawer: public GLViewer_Drawer
{
  
public:
  GLViewer_TextDrawer();
  ~GLViewer_TextDrawer();
  
  //! Redefined method
  virtual void              create( float, float, bool );
  //! Updates objects after updating font
  void                      updateObjects();
};

#ifdef WNT
#pragma warning ( default:4251 )
#endif

#endif
