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
#include "SalomeApp_TypeFilter.h"

#include "LightApp_DataOwner.h"
#include "SalomeApp_Study.h"

/*!
  Constructor.
*/
SalomeApp_TypeFilter::SalomeApp_TypeFilter( SalomeApp_Study* study, const QString& kind )
  : SalomeApp_Filter( study ) 
{
  myKind = kind;
}

/*!
  Destructor.
*/
SalomeApp_TypeFilter::~SalomeApp_TypeFilter()
{
}

/*!
  Check: data owner is valid?
*/
bool SalomeApp_TypeFilter::isOk( const SUIT_DataOwner* sOwner ) const
{  
  const LightApp_DataOwner* owner = dynamic_cast<const LightApp_DataOwner*> ( sOwner );

  SalomeApp_Study* aDoc =  getStudy();
  if (owner && aDoc && aDoc->studyDS())
    {
      _PTR(Study) aStudy = aDoc->studyDS();
      QString entry = owner->entry();
      
      _PTR(SObject) aSObj( aStudy->FindObjectID( entry.latin1() ) );
      if (aSObj)
	{
	  _PTR(SComponent) aComponent(aSObj->GetFatherComponent());
	  if ( aComponent && (aComponent->ComponentDataType() == myKind.latin1()) )
	    return true;
	}
    }

  return false;
}
