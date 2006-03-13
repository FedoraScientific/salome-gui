// File:      SalomeApp_Module.cxx
// Created:   10/25/2004 11:39:56 AM
// Author:    Sergey LITONIN
// Copyright (C) CEA 2004

#include "SalomeApp_Module.h"
#include "SalomeApp_DataModel.h"
#include "SalomeApp_Application.h"
#include "SalomeApp_Study.h"

#include "LightApp_Selection.h"
#include "LightApp_Operation.h"
#include "LightApp_Preferences.h"
//#include "LightApp_Displayer.h"

#include "CAM_DataModel.h"

#include "OB_Browser.h"

#include <SALOME_ListIO.hxx>
#include <SALOME_ListIteratorOfListIO.hxx>
#include <SALOME_InteractiveObject.hxx>
//#include <SALOME_Actor.h>

//#include "SALOMEDS_IParameters.hxx"

#include <SUIT_Session.h>
#include <SUIT_ViewModel.h>

#include <SVTK_ViewWindow.h>
//#include <SVTK_ViewModel.h>
//#include <SVTK_MainWindow.h>
//#include <SVTK_RenderWindowInteractor.h>

#include <qstring.h>
#include <qmap.h>

//#include <vtkActorCollection.h>
//#include <vtkRenderer.h>

/*!Constructor.*/
SalomeApp_Module::SalomeApp_Module( const QString& name )
: LightApp_Module( name )
{
}

/*!Destructor.*/
SalomeApp_Module::~SalomeApp_Module()
{
}

/*!Gets application.*/
SalomeApp_Application* SalomeApp_Module::getApp() const
{
  return (SalomeApp_Application*)application();
}

/*!Create new instance of data model and return it.*/
CAM_DataModel* SalomeApp_Module::createDataModel()
{
  return new SalomeApp_DataModel(this);
}

/*!Create and return instance of LightApp_Selection.*/
LightApp_Selection* SalomeApp_Module::createSelection() const
{
  return LightApp_Module::createSelection();
}

void SalomeApp_Module::extractContainers( const SALOME_ListIO& source, SALOME_ListIO& dest ) const
{
  SalomeApp_Study* study = dynamic_cast<SalomeApp_Study*>( SUIT_Session::session()->activeApplication()->activeStudy() );
  if( !study )
  {
    dest = source;
    return;
  }

  SALOME_ListIteratorOfListIO anIt( source );
  for( ; anIt.More(); anIt.Next() )
  {
    Handle( SALOME_InteractiveObject ) obj = anIt.Value();
    if( obj->hasEntry() )
    {
      _PTR(SObject) SO = study->studyDS()->FindObjectID( obj->getEntry() );
      if( SO && QString( SO->GetID().c_str() ) == SO->GetFatherComponent()->GetID().c_str() )
      { //component is selected
	_PTR(SComponent) SC( SO->GetFatherComponent() );
	_PTR(ChildIterator) anIter ( study->studyDS()->NewChildIterator( SC ) );
	anIter->InitEx( true );
	while( anIter->More() )
	{
	  _PTR(SObject) valSO ( anIter->Value() );
	  _PTR(SObject) refSO;
	  if( !valSO->ReferencedObject( refSO ) )
	  {
	    QString id = valSO->GetID().c_str(),
	            comp = SC->ComponentDataType().c_str(),
		    val = valSO->GetName().c_str();

	    Handle( SALOME_InteractiveObject ) new_obj =
	      new SALOME_InteractiveObject( id.latin1(), comp.latin1(), val.latin1() );
	    dest.Append( new_obj );
	  }
	  anIter->Next();
	}
	continue;
      }
    }
    dest.Append( obj );
  }
}

/*!
 * \brief Virtual public
 *
 * This method is called just before the study document is saved, so the module has a possibility
 * to store visual parameters in AttributeParameter attribue(s)
 */
void SalomeApp_Module::storeVisualParameters(int savePoint)
{
}

/*!
 * \brief Virtual public
 *
 * This method is called after the study document is opened, so the module has a possibility to restore
 * visual parameters
 */
void SalomeApp_Module::restoreVisualParameters(int savePoint)
{
}

