//  SALOME VTKViewer : build VTK viewer into Salome desktop
//
//  Copyright (C) 2004  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SVTK_Prs.h
//  Author : Sergey ANIKIN
//  Module : SALOME
//  $Header$

#ifndef SVTK_Prs_H
#define SVTK_Prs_H

#include <SVTK.h>
#include "SALOME_Prs.h"

#include <vtkActorCollection.h>

class SVTK_EXPORT SVTK_Prs : public SALOME_VTKPrs
{
public:
  SVTK_Prs();
  // Default constructor
  SVTK_Prs( const vtkActor* obj );
  // Standard constructor
  ~SVTK_Prs();
  // Destructor

  vtkActorCollection* GetObjects() const;
  // Get actors list
  void AddObject( const vtkActor* obj );
  // Add actor
  
  bool IsNull() const;
  // Reimplemented from SALOME_Prs

private:
  vtkActorCollection* myObjects;    // list of actors
};

#endif
