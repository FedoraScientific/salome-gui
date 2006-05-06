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
#ifndef SVTK_VIEWMANAGER_H
#define SVTK_VIEWMANAGER_H

#include "SUIT_ViewManager.h"
#include "SVTK.h"

class SUIT_Desktop;

//! Extend SUIT_ViewManager to deal with SVTK_Viewer
class SVTK_EXPORT SVTK_ViewManager : public SUIT_ViewManager
{
  Q_OBJECT

public:
  //! Construct the view manager
  SVTK_ViewManager( SUIT_Study* study, SUIT_Desktop* );

  //! Destroy the view manager
  virtual ~SVTK_ViewManager();

  SUIT_Desktop* getDesktop();

protected:
  void setViewName( SUIT_ViewWindow* theView );

private:
  int               myId;
  static  int       _SVTKViewMgr_Id;
};

#endif
