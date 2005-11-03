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

// File:      GLViewer_Selector.h
// Created:   November, 2004

/****************************************************************************
**  Class:   GLViewer_Selector
**  Descr:   Base class for object selection in QAD-based application
**  Module:  GLViewer
**  Created: UI team, 22.09.00
*****************************************************************************/
#ifndef GLVIEWER_SELECTOR_H
#define GLVIEWER_SELECTOR_H

#include "GLViewer.h"
#include "GLViewer_Defs.h"

#include <qsize.h>
#include <qobject.h>

#include <Quantity_NameOfColor.hxx>

class GLViewer_Viewer;

/*! Class GLViewer_Selector
*   Based select manager for GLViewer
*/

class GLVIEWER_API GLViewer_Selector : public QObject
{
    Q_OBJECT

public:
    GLViewer_Selector( GLViewer_Viewer* );
    ~GLViewer_Selector();

public:
    //! Sets lock status (enable/disable interavtive)
    void                  lock( bool );
    void                  setMinRectSize( const QSize& minSize );

    //! Highlights in point (x,y)
    virtual void          detect( int x, int y ) = 0;    
    virtual void          undetectAll() = 0;

    //! Selects highlight objects
    virtual void          select( bool append = false ) = 0;
    //! Selects by rect
    virtual void          select( const QRect&, bool append = false ) = 0;
    virtual void          unselectAll() = 0;
    virtual int           numSelected() const = 0;

    virtual void          setHilightColor( Quantity_NameOfColor ) = 0;
    virtual void          setSelectColor( Quantity_NameOfColor ) = 0;
    //!Checks selection state and emits  'selSelectionDone' or 'selSelectionCancel'     
    /*!Should be called by after non-interactive selection. */
    virtual void          checkSelection( int, bool, int ) = 0;

    /*! Sets/returns the key for appending selected objects ( SHIFT by default ) */
    static int            appendKey() { return apppendKey; }
    static void           setAppendKey( int k ) { apppendKey = k; }

signals:
    void                  selSelectionCancel();
    void                  selSelectionDone( bool append, SelectionChangeStatus status );

protected:
//    void                  setStatus( SelectionChangeStatus theStatus ){ myStatus = theStatus; }
//    SelectionChangeStatus status( return myStatus; }

    GLViewer_Viewer*      myViewer;
    bool                  myLocked;
    QSize                 myMinRectSize;

private:
//    SelectionChangeStatus myStatus;
    static int            apppendKey;
};

#endif
