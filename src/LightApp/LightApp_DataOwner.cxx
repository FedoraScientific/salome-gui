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
#include "LightApp_DataOwner.h"

#include "LightApp_DataObject.h"

#ifndef WNT
#include <typeinfo>
#define _typeinfo std::type_info
#else
#include <typeinfo.h>
#define _typeinfo type_info
#endif

#include <iostream>

/*!Constructor. Initialize by \a theEntry.*/
LightApp_DataOwner
::LightApp_DataOwner( const QString& theEntry ): 
  myEntry( theEntry )
{
}

/*!Constructor. Initialize by \a SALOME_InteractiveObject.*/
LightApp_DataOwner
::LightApp_DataOwner( const Handle(SALOME_InteractiveObject)& theIO ):
  myEntry(!theIO.IsNull()? theIO->getEntry(): ""),
  myIO(theIO)
{
}

/*!Destructor. Do nothing.*/
LightApp_DataOwner
::~LightApp_DataOwner()
{
}

/*!Checks: Is current data owner equal \a obj.*/
bool
LightApp_DataOwner
::isEqual( const SUIT_DataOwner& obj ) const
{
  const LightApp_DataOwner* other = dynamic_cast<const LightApp_DataOwner*>( &obj );

  return other && entry() == other->entry();
}

/*!Gets entry.*/
QString
LightApp_DataOwner
::entry() const
{
  return myEntry;
}

/*!Gets SALOME_InteractiveObject.*/
const Handle(SALOME_InteractiveObject)&
LightApp_DataOwner
::IO() const
{
  return myIO;
}
