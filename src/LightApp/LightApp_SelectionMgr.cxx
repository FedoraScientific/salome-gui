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
#include "LightApp_SelectionMgr.h"

#include "LightApp_Study.h"
#include "LightApp_DataOwner.h"
#include "LightApp_DataSubOwner.h"
#include "LightApp_Application.h"

#include <SUIT_Session.h>

#include <SALOME_ListIO.hxx>
#include <SALOME_ListIteratorOfListIO.hxx>

// Open CASCADE Include
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

/*!
  Constructor.
*/
LightApp_SelectionMgr::LightApp_SelectionMgr( LightApp_Application* app, const bool fb )
: SUIT_SelectionMgr( fb ),
myApp( app )
{
}

/*!
  Destructor.
*/
LightApp_SelectionMgr::~LightApp_SelectionMgr()
{
}

/*!
  Gets application.
*/
LightApp_Application* LightApp_SelectionMgr::application() const
{
  return myApp;
}

/*!
  Get all selected objects from selection manager
*/
void LightApp_SelectionMgr::selectedObjects( SALOME_ListIO& theList, const QString& theType,
                                             const bool convertReferences ) const
{
  theList.Clear();

  SUIT_DataOwnerPtrList aList;
  selected( aList, theType );

  QMap<QString,int> entryMap;

  QString entry;
  for ( SUIT_DataOwnerPtrList::const_iterator itr = aList.begin(); itr != aList.end(); ++itr )
  {
    const LightApp_DataOwner* owner = dynamic_cast<const LightApp_DataOwner*>( (*itr).operator->() );
    if( !owner )
      continue;

    LightApp_Study* study = dynamic_cast<LightApp_Study*>( application()->activeStudy() );
    if ( !study )
      return;

    entry = owner->entry();
    if ( convertReferences ) {
      QString refEntry = study->referencedToEntry( entry );
      if( !entryMap.contains( entry ) ) {
        if ( refEntry != entry ) {
          QString component = study->componentDataType( refEntry );
          theList.Append( new SALOME_InteractiveObject( refEntry, component, ""/*refobj->Name().c_str()*/ ) );
        }
        else
          theList.Append( owner->IO() );
      }
    }
    else {
      if( !entryMap.contains( entry ) )
	theList.Append( owner->IO() );
    }

    entryMap.insert(owner->entry(), 1);
  }
}

/*!
  Append selected objects.
*/
void LightApp_SelectionMgr::setSelectedObjects( const SALOME_ListIO& lst, const bool append )
{
  SUIT_DataOwnerPtrList owners;
  for ( SALOME_ListIteratorOfListIO it( lst ); it.More(); it.Next() )
  {
    if ( it.Value()->hasEntry() )
      owners.append( new LightApp_DataOwner( it.Value() ) );
  }

  setSelected( owners, append );
}

/*!
  Emit current selection changed.
*/
void LightApp_SelectionMgr::selectionChanged( SUIT_Selector* theSel )
{
  SUIT_SelectionMgr::selectionChanged( theSel );

  emit currentSelectionChanged();
}

/*!
  get map of indexes for the given SALOME_InteractiveObject
*/
void LightApp_SelectionMgr::GetIndexes( const Handle(SALOME_InteractiveObject)& IObject, 
                                        TColStd_IndexedMapOfInteger& theIndex)
{
  theIndex.Clear();

  SUIT_DataOwnerPtrList aList;
  selected( aList );

  for ( SUIT_DataOwnerPtrList::const_iterator itr = aList.begin(); itr != aList.end(); ++itr )
  {
    const LightApp_DataSubOwner* subOwner = dynamic_cast<const LightApp_DataSubOwner*>( (*itr).operator->() );
    if ( subOwner )
      if ( subOwner->entry() == QString(IObject->getEntry()) )
	theIndex.Add( subOwner->index() );
  }
  
}

/*!
  get map of indexes for the given entry of SALOME_InteractiveObject
*/
void LightApp_SelectionMgr::GetIndexes( const QString& theEntry, TColStd_IndexedMapOfInteger& theIndex )
{
  theIndex.Clear();

  SUIT_DataOwnerPtrList aList;
  selected( aList );

  for ( SUIT_DataOwnerPtrList::const_iterator itr = aList.begin(); itr != aList.end(); ++itr )
  {
    const LightApp_DataSubOwner* subOwner = dynamic_cast<const LightApp_DataSubOwner*>( (*itr).operator->() );
    if ( subOwner )
      if ( subOwner->entry() == theEntry )
	theIndex.Add( subOwner->index() );
  }

}

/*!
  Add or remove interactive objects from selection manager.
*/
bool LightApp_SelectionMgr::AddOrRemoveIndex( const Handle(SALOME_InteractiveObject)& IObject, 
					       const TColStd_MapOfInteger& theIndexes, 
					       bool modeShift)
{
  SUIT_DataOwnerPtrList remainsOwners;
  
  SUIT_DataOwnerPtrList aList;
  selected( aList );

  if ( !modeShift ) {
    for ( SUIT_DataOwnerPtrList::const_iterator itr = aList.begin(); itr != aList.end(); ++itr )
    {
      const LightApp_DataOwner* owner = dynamic_cast<const LightApp_DataOwner*>( (*itr).operator->() );
      if ( owner ) 
      {
	if ( owner->entry() != QString(IObject->getEntry()) ) 
	{	  
	  const LightApp_DataSubOwner* subOwner = dynamic_cast<const LightApp_DataSubOwner*>( owner );
	  if ( subOwner )
	    remainsOwners.append( new LightApp_DataSubOwner( subOwner->entry(), subOwner->index() ) );
	  else
	    remainsOwners.append( new LightApp_DataOwner( owner->entry() ) );
	}
      }
    }
  }
  else
    remainsOwners = aList;

  TColStd_MapIteratorOfMapOfInteger It;
  It.Initialize(theIndexes);
  for(;It.More();It.Next())
    remainsOwners.append( new LightApp_DataSubOwner( QString(IObject->getEntry()), It.Key() ) );
  
  bool append = false;
  setSelected( remainsOwners, append );

  emit currentSelectionChanged();

  TColStd_IndexedMapOfInteger anIndexes;
  GetIndexes( IObject, anIndexes );
  return !anIndexes.IsEmpty();

}

/*!
  select 'subobjects' with given indexes
*/
void LightApp_SelectionMgr::selectObjects( const Handle(SALOME_InteractiveObject)& IObject, 
					    TColStd_IndexedMapOfInteger theIndex, bool append )
{
  SUIT_DataOwnerPtrList aList;

  if ( theIndex.IsEmpty() )
    aList.append( new LightApp_DataOwner( QString(IObject->getEntry()) ) );
  else
    {
      int i;
      for ( i = 1; i <= theIndex.Extent(); i++ )
	aList.append( new LightApp_DataSubOwner( QString(IObject->getEntry()), theIndex( i ) ) );
    }

  setSelected( aList, append );

}

/*!
  select 'subobjects' with given indexes
*/
void LightApp_SelectionMgr::selectObjects( MapIOOfMapOfInteger theMapIO, bool append )
{
  SUIT_DataOwnerPtrList aList;

  MapIOOfMapOfInteger::Iterator it;
  for ( it = theMapIO.begin(); it != theMapIO.end(); ++it ) 
    {
      if ( it.data().IsEmpty() )
	aList.append( new LightApp_DataOwner( QString(it.key()->getEntry()) ) );
      else
	{
	  int i;
	  for ( i = 1; i <= it.data().Extent(); i++ )
	    aList.append( new LightApp_DataSubOwner( QString(it.key()->getEntry()), it.data()( i ) ) );
	}
    }
  
  setSelected( aList, append );

}

/*!
  get map of selected subowners : object's entry <-> map of indexes
*/
void LightApp_SelectionMgr::selectedSubOwners( MapEntryOfMapOfInteger& theMap )
{
  theMap.clear();

  TColStd_IndexedMapOfInteger anIndexes;

  SUIT_DataOwnerPtrList aList;
  selected( aList );

  for ( SUIT_DataOwnerPtrList::const_iterator itr = aList.begin(); itr != aList.end(); ++itr )
  {
    const LightApp_DataSubOwner* subOwner = dynamic_cast<const LightApp_DataSubOwner*>( (*itr).operator->() );
    if ( subOwner ) 
    {
      if ( !theMap.contains( subOwner->entry() ) )
      {
	anIndexes.Clear();
	GetIndexes( subOwner->entry(), anIndexes );
	theMap.insert( subOwner->entry(), anIndexes );
      }
    }
  }
}
