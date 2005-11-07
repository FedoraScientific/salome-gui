#include "LightApp_GLSelector.h"

#include "LightApp_DataOwner.h"

#include <SALOME_GLOwner.h>

#include <GLViewer_Context.h>

/*!Constructor. Initialize by GLViewer_Viewer2d and SUIT_SelectionMgr.*/
LightApp_GLSelector::LightApp_GLSelector( GLViewer_Viewer2d* viewer, SUIT_SelectionMgr* mgr )
: SUIT_Selector( mgr, viewer ),
  myViewer( viewer )
{
  if ( myViewer )
    connect( myViewer, SIGNAL( selectionChanged( SelectionChangeStatus ) ), 
             this, SLOT( onSelectionChanged() ) );
}

/*!Destructor. Do nothing.*/
LightApp_GLSelector::~LightApp_GLSelector()
{
}

/*!Gets viewer*/
GLViewer_Viewer2d* LightApp_GLSelector::viewer() const
{
  return myViewer;
}

/*!On selection changed event.*/
void LightApp_GLSelector::onSelectionChanged()
{
  selectionChanged();
}

/*!Gets list of selected Data Owner objects.*/
void LightApp_GLSelector::getSelection( SUIT_DataOwnerPtrList& aList ) const
{
  if ( !myViewer )
    return;

  GLViewer_Context* cont = myViewer->getGLContext();
  if ( !cont )
    return;

  for ( cont->InitSelected(); cont->MoreSelected(); cont->NextSelected() )
  {
    GLViewer_Object* obj = cont->SelectedObject();
    if ( obj )
    {
      SALOME_GLOwner* owner = dynamic_cast< SALOME_GLOwner* >( obj->owner() );
      if( owner )
        aList.append( SUIT_DataOwnerPtr( new LightApp_DataOwner( owner->entry() ) ) );
    }
  }
}

/*!Sets to selected list of Data Owner objects.*/
void LightApp_GLSelector::setSelection( const SUIT_DataOwnerPtrList& aList )
{
  if ( !myViewer )
    return;

  GLViewer_Context* cont = myViewer->getGLContext();
  if ( !cont )
    return;

  QMap<QString, GLViewer_Object*> aDisplayed;
  const ObjList& displayed = cont->getObjects();
  for ( ObjList::const_iterator it = displayed.begin(); it != displayed.end(); ++it )
  {
    GLViewer_Object* obj = *it;
    if ( obj && obj->getVisible() )
    {
      SALOME_GLOwner* owner = dynamic_cast< SALOME_GLOwner* >( obj->owner() );
      if ( owner )
	aDisplayed.insert( owner->entry(), obj );
    }
  }

  int Nb = 0;
  cont->clearSelected( false );
  for ( SUIT_DataOwnerPtrList::const_iterator itr = aList.begin(); itr != aList.end(); ++itr )
  {
    const LightApp_DataOwner* owner = dynamic_cast<const LightApp_DataOwner*>( (*itr).operator->() );

    if ( !owner )
      continue;

    if ( aDisplayed.contains( owner->entry() ) )
    {
      cont->setSelected( aDisplayed[owner->entry()], false );
      Nb++;
    }
  }

  if ( Nb > 0 )
    myViewer->updateAll();
}