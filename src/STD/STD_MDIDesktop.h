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
#ifndef STD_MDIDESKTOP_H
#define STD_MDIDESKTOP_H

#include "STD.h"

#include <SUIT_Desktop.h>

class QtxAction;
class QPopupMenu;
class QWorkspace;
class QtxWorkspaceAction;

#if defined WNT
#pragma warning( disable: 4251 )
#endif

class STD_EXPORT STD_MDIDesktop: public SUIT_Desktop 
{
  Q_OBJECT

public:
  enum { MenuWindowId = 6 };
  enum { Cascade, Tile, HTile, VTile };

public:
  STD_MDIDesktop();
  virtual ~STD_MDIDesktop();

  virtual SUIT_ViewWindow* activeWindow() const;
  virtual QPtrList<SUIT_ViewWindow> windows() const;

  void                     windowOperation( const int );

  void                     setWindowOperations( const int, ... );
  void                     setWindowOperations( const QValueList<int>& );

  QWorkspace*              workspace() const;

private slots:
  void                     onWindowActivated( QWidget* );

protected:
  void                     createActions();
  virtual QWidget*         parentArea() const;

private:
  int                      operationFlag( const int ) const;

private:
  QWorkspace*              myWorkspace;
  QtxWorkspaceAction*      myWorkspaceAction;
};

#if defined WNT
#pragma warning( default: 4251 )
#endif

#endif
