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

#include "LightApp_ShowHideOp.h"
#include "LightApp_Application.h"
#include "LightApp_DataOwner.h"
#include "LightApp_Module.h"
#include "LightApp_Study.h"
#include "LightApp_Displayer.h"
#include "CAM_Study.h"

#include "LightApp_SelectionMgr.h"
#include "LightApp_Selection.h"

#ifndef DISABLE_SALOMEOBJECT
  #include <SALOME_ListIO.hxx>
  #include <SALOME_ListIteratorOfListIO.hxx>
#endif

LightApp_ShowHideOp::LightApp_ShowHideOp( ActionType type )
: LightApp_Operation(),
  myActionType( type )
{
}

LightApp_ShowHideOp::~LightApp_ShowHideOp()
{
}

void LightApp_ShowHideOp::startOperation()
{
  LightApp_Application* app = dynamic_cast<LightApp_Application*>( application() );
  LightApp_Study* study = app ? dynamic_cast<LightApp_Study*>( app->activeStudy() ) : 0;
  if( !app || !study )
  {
    abort();
    return;
  }

  LightApp_SelectionMgr* mgr = app->selectionMgr();
  LightApp_Selection sel; sel.init( "", mgr );
  if( sel.count()==0 && myActionType!=ERASE_ALL )
  {
    abort();
    return;
  }

  QString mod_name;
  if( sel.count()>0 )
  {
    QString aStr =  sel.param( 0, "component" ).toString();
    mod_name = app->moduleTitle( aStr );
  }
  else if( app->activeModule() )
    mod_name = app->moduleTitle( app->activeModule()->name() );

  LightApp_Displayer* d = LightApp_Displayer::FindDisplayer( mod_name, true );
  if( !d )
  {
    abort();
    return;
  }

  if( myActionType==DISPLAY_ONLY || myActionType==ERASE_ALL )
  {
    //ERASE ALL
    QStringList comps;
    study->components( comps );
    QStringList::const_iterator anIt = comps.begin(), aLast = comps.end();
    for( ; anIt!=aLast; anIt++ )
    {
      LightApp_Displayer* disp = LightApp_Displayer::FindDisplayer( app->moduleTitle( *anIt ), true );
      if( disp )
	disp->EraseAll( false, false, 0 );
    }
    if( myActionType==ERASE_ALL )
    {
      d->UpdateViewer();
      commit();
      return;
    }
  }

  QStringList entries;

#ifndef DISABLE_SALOMEOBJECT
  SALOME_ListIO selObjs;
  mgr->selectedObjects( selObjs );
  SALOME_ListIteratorOfListIO anIt( selObjs );
  for( ; anIt.More(); anIt.Next() )
    if( !anIt.Value().IsNull() )
#else
  QStringList selObjs;
  mgr->selectedObjects( selObjs );
  QStringList::const_iterator anIt = selObjs.begin(), aLast = selObjs.end();
  for( ; ; anIt!=aLast )
#endif
    {
      QString entry = 
#ifndef DISABLE_SALOMEOBJECT
        anIt.Value()->getEntry();
#else
        *anIt;
#endif

      if( study->isComponent( entry ) )
        study->children( entry, entries );
      else
        entries.append( entry );
    }

  for( QStringList::const_iterator it = entries.begin(), last = entries.end(); it!=last; it++ )
  {
    QString e = study->referencedToEntry( *it );
    if( myActionType==DISPLAY || myActionType==DISPLAY_ONLY )
      d->Display( e, false, 0 );
    else if( myActionType==ERASE )
      d->Erase( e, false, false, 0 );
  }
  d->UpdateViewer();
  commit();
}
