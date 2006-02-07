//  SALOME VTKViewer : build VTK viewer into Salome desktop
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : 
//  Author : 
//  Module : SALOME
//  $Header$

#include "SVTK_GenericRenderWindowInteractor.h"
#include "SVTK_Selector.h"

#include <vtkObjectFactory.h>
#include <vtkCommand.h>

#include <qtimer.h>

using namespace std;

//----------------------------------------------------------------------------
vtkStandardNewMacro(QVTK_GenericRenderWindowInteractor);

QVTK_GenericRenderWindowInteractor
::QVTK_GenericRenderWindowInteractor()
{
  myTimer = new QTimer( ) ;
  connect(myTimer, SIGNAL(timeout()), this, SLOT(OnTimeOut())) ;
}

QVTK_GenericRenderWindowInteractor
::~QVTK_GenericRenderWindowInteractor()
{
  delete myTimer;
}


//----------------------------------------------------------------------------
void
QVTK_GenericRenderWindowInteractor
::OnTimeOut() 
{
  if( GetEnabled() ) {
    this->InvokeEvent(vtkCommand::TimerEvent,NULL);
  }
}

int
QVTK_GenericRenderWindowInteractor
::CreateTimer(int vtkNotUsed(timertype)) 
{
  //
  // Start a one-shot timer for <DELAY> ms. 
  //
  static int DELAY = 1;
  myTimer->start(DELAY,TRUE);
  return 1;
}

int
QVTK_GenericRenderWindowInteractor
::DestroyTimer(void) 
{
  //
  // :TRICKY: Tue May  2 00:17:32 2000 Pagey
  //
  // QTimer will automatically expire after 10ms. So 
  // we do not need to do anything here. In fact, we 
  // should not even Stop() the QTimer here because doing 
  // this will skip some of the processing that the TimerFunc()
  // does and will result in undesirable effects. For 
  // example, this will result in vtkLODActor to leave
  // the models in low-res mode after the mouse stops
  // moving. 
  //
  return 1;
}


//----------------------------------------------------------------------------
vtkStandardNewMacro(SVTK_GenericRenderWindowInteractor);

SVTK_GenericRenderWindowInteractor
::SVTK_GenericRenderWindowInteractor():
  myRenderWidget(NULL)
{
}

SVTK_GenericRenderWindowInteractor
::~SVTK_GenericRenderWindowInteractor()
{
}

SVTK_Selector*
SVTK_GenericRenderWindowInteractor
::GetSelector()
{
  return mySelector.GetPointer();
}

void
SVTK_GenericRenderWindowInteractor
::SetSelector(SVTK_Selector* theSelector)
{
  mySelector = theSelector;
}

QWidget*
SVTK_GenericRenderWindowInteractor
::GetRenderWidget()
{
  return myRenderWidget;
}

void
SVTK_GenericRenderWindowInteractor
::SetRenderWidget(QWidget* theRenderWidget)
{
  myRenderWidget = theRenderWidget;
}
