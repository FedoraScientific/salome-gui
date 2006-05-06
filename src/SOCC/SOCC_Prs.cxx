//  SALOME OCCViewer : build OCC Viewer into Salome desktop
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SOCC_Prs.cxx
//  Author : Sergey ANIKIN
//  Module : SALOME
//  $Header$

#include "SOCC_Prs.h"
using namespace std;

/*!
  Default constructor
*/
SOCC_Prs::SOCC_Prs() 
{
  myToActivate = true;
}

/*!
  Standard constructor
*/
SOCC_Prs::SOCC_Prs( const Handle(AIS_InteractiveObject)& obj ) 
{  
  AddObject( obj ); 
}

/*!
  Destructor
*/
SOCC_Prs::~SOCC_Prs()
{ 
  myObjects.Clear(); 
}

/*!
  Get interactive objects list
*/
void SOCC_Prs::GetObjects( AIS_ListOfInteractive& list ) const 
{ 
  list = myObjects; 
}

/*!
  Add interactive object
*/
void SOCC_Prs::AddObject( const Handle(AIS_InteractiveObject)& obj ) 
{ 
  myObjects.Append( obj ); 
}

/*!
  \return 0 if list of the interactive objects is empty [ Reimplemented from SALOME_Prs ]
*/
bool SOCC_Prs::IsNull() const 
{ 
  return myObjects.IsEmpty(); 
}

/*!
  This method is used for activisation/deactivisation of
  objects in the moment of displaying
*/   
void SOCC_Prs::SetToActivate( const bool toActivate )
{
  myToActivate = toActivate;
}

bool SOCC_Prs::ToActivate() const
{
  return myToActivate;
}
