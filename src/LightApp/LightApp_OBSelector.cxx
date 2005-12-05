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
#include "LightApp_OBSelector.h"

#include "LightApp_DataOwner.h"
#include "LightApp_DataObject.h"

#include <OB_Browser.h>

#include <SUIT_DataObjectIterator.h>

/*!
  Constructor
*/
LightApp_OBSelector::LightApp_OBSelector( OB_Browser* ob, SUIT_SelectionMgr* mgr )
: SUIT_Selector( mgr, ob ),
  myBrowser( ob )
{
  if ( myBrowser ) {
    connect( myBrowser, SIGNAL( selectionChanged() ), this, SLOT( onSelectionChanged() ) );
  }    
}

/*!
  Destructor
*/
LightApp_OBSelector::~LightApp_OBSelector()
{
}

/*!
  Gets browser.
*/
OB_Browser* LightApp_OBSelector::browser() const
{
  return myBrowser;
}

/*!
  Gets selection.
*/
void LightApp_OBSelector::getSelection( SUIT_DataOwnerPtrList& theList ) const
{
  if ( !myBrowser )
    return;

  DataObjectList objlist;
  myBrowser->getSelected( objlist );
  for ( DataObjectListIterator it( objlist ); it.current(); ++it )
  {
    LightApp_DataObject* obj = dynamic_cast<LightApp_DataObject*>( it.current() );
    if ( obj )
    {
      Handle( SALOME_InteractiveObject ) aSObj = new SALOME_InteractiveObject
	( obj->entry(), obj->componentDataType(), obj->name() );
      LightApp_DataOwner* owner = new LightApp_DataOwner( aSObj  );
      theList.append( SUIT_DataOwnerPtr( owner ) );
    }
  }
}

/*!Sets selection.*/
void LightApp_OBSelector::setSelection( const SUIT_DataOwnerPtrList& theList )
{
  if ( !myBrowser )
    return;

  QMap<QString, LightApp_DataObject*> themap;
  fillEntries( themap );

  DataObjectList objList;
  for ( SUIT_DataOwnerPtrList::const_iterator it = theList.begin(); it != theList.end(); ++it )
  {
    const LightApp_DataOwner* owner = dynamic_cast<const LightApp_DataOwner*>( (*it).operator->() );
    if ( owner && themap.contains( owner->entry() ) )
      objList.append( themap[owner->entry()] );
  }

  myBrowser->setSelected( objList );
}

/*!On selection changed.*/
void LightApp_OBSelector::onSelectionChanged()
{
  selectionChanged();
}

/*!Fill entries.*/
void LightApp_OBSelector::fillEntries( QMap<QString, LightApp_DataObject*>& entires )
{
  entires.clear();

  if ( !myBrowser )
    return;

  for ( SUIT_DataObjectIterator it( myBrowser->getRootObject(),
                                    SUIT_DataObjectIterator::DepthLeft ); it.current(); ++it )
  {
    LightApp_DataObject* obj = dynamic_cast<LightApp_DataObject*>( it.current() );
    if ( obj )
      entires.insert( obj->entry(), obj );
  }
}
