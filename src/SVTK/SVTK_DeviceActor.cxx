//  SVTK OBJECT : interactive object for SVTK visualization
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
//  File   : SVTK_DeviceActor.cxx
//  Author : 
//  Module : 
//  $Header$


#include "SVTK_DeviceActor.h"

#include "VTKViewer_Transform.h"
#include "VTKViewer_TransformFilter.h"
#include "VTKViewer_PassThroughFilter.h"
#include "VTKViewer_GeometryFilter.h"

// VTK Includes
#include <vtkObjectFactory.h>
#include <vtkShrinkFilter.h>

#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include <vtkPolyDataMapper.h>
#include <vtkDataSetMapper.h>

using namespace std;

//----------------------------------------------------------------------------
vtkStandardNewMacro(SVTK_DeviceActor);


//----------------------------------------------------------------------------
SVTK_DeviceActor
::SVTK_DeviceActor()
{
  myIsShrunk = false;
  myIsShrinkable = true;

  myIsShaded = true;
  myProperty = vtkProperty::New();
  myRepresentation = SVTK::Representation::Surface;

  myIsResolveCoincidentTopology = false;
  vtkMapper::GetResolveCoincidentTopologyPolygonOffsetParameters(myPolygonOffsetFactor,
								 myPolygonOffsetUnits);

  myMapper = vtkDataSetMapper::New();

  myShrinkFilter = vtkShrinkFilter::New();

  myGeomFilter = VTKViewer_GeometryFilter::New();

  myTransformFilter = VTKViewer_TransformFilter::New();

  for(int i = 0; i < 6; i++)
    myPassFilter.push_back(VTKViewer_PassThroughFilter::New());
}


//----------------------------------------------------------------------------
SVTK_DeviceActor
::~SVTK_DeviceActor()
{
  myMapper->Delete();

  myProperty->Delete();

  myGeomFilter->Delete();

  myTransformFilter->Delete();

  myShrinkFilter->Delete();

  for(int i = 0, iEnd = myPassFilter.size(); i < iEnd; i++)
    myPassFilter[i]->Delete();
}


//----------------------------------------------------------------------------
void
SVTK_DeviceActor
::SetMapper(vtkMapper* theMapper)
{
  InitPipeLine(theMapper);
}

void
SVTK_DeviceActor
::InitPipeLine(vtkMapper* theMapper)
{
  if(theMapper){
    int anId = 0;
    myPassFilter[ anId ]->SetInput( theMapper->GetInput() );
    myPassFilter[ anId + 1]->SetInput( myPassFilter[ anId ]->GetOutput() );
    
    anId++; // 1
    myGeomFilter->SetInput( myPassFilter[ anId ]->GetOutput() );

    anId++; // 2
    myPassFilter[ anId ]->SetInput( myGeomFilter->GetOutput() ); 
    myPassFilter[ anId + 1 ]->SetInput( myPassFilter[ anId ]->GetOutput() );

    anId++; // 3
    myTransformFilter->SetInput( myPassFilter[ anId ]->GetPolyDataOutput() );

    anId++; // 4
    myPassFilter[ anId ]->SetInput( myTransformFilter->GetOutput() );
    myPassFilter[ anId + 1 ]->SetInput( myPassFilter[ anId ]->GetOutput() );

    anId++; // 5
    if(vtkDataSetMapper* aMapper = dynamic_cast<vtkDataSetMapper*>(theMapper)){
      aMapper->SetInput(myPassFilter[anId]->GetOutput());
    }else if(vtkPolyDataMapper* aMapper = dynamic_cast<vtkPolyDataMapper*>(theMapper)){
      aMapper->SetInput(myPassFilter[anId]->GetPolyDataOutput());
    }
  }else
    myPassFilter[ 0 ]->SetInput( NULL );
  Superclass::SetMapper(theMapper);
}

//----------------------------------------------------------------------------
vtkDataSet* 
SVTK_DeviceActor
::GetInput()
{
  return myPassFilter.front()->GetOutput();
}

void
SVTK_DeviceActor
::SetInput(vtkDataSet* theDataSet)
{
  myMapper->SetInput(theDataSet);
  InitPipeLine(myMapper);
}

//----------------------------------------------------------------------------
void
SVTK_DeviceActor::
SetStoreMapping(bool theStoreMapping)
{
  myGeomFilter->SetStoreMapping(theStoreMapping);
}



//----------------------------------------------------------------------------
unsigned long int 
SVTK_DeviceActor
::GetMTime()
{
  unsigned long mTime = this->Superclass::GetMTime();

  mTime = max(mTime,myGeomFilter->GetMTime());

  mTime = max(mTime,myTransformFilter->GetMTime());

  if(myIsShrunk)
    mTime = max(mTime,myShrinkFilter->GetMTime());

  for(int i = 0, iEnd = myPassFilter.size(); i < iEnd; i++)
    max(mTime,myPassFilter[i]->GetMTime());

  return mTime;
}


//----------------------------------------------------------------------------
void 
SVTK_DeviceActor
::SetTransform(VTKViewer_Transform* theTransform)
{
  myTransformFilter->SetTransform(theTransform);
}


//----------------------------------------------------------------------------
bool
SVTK_DeviceActor
::IsShrunkable() 
{ 
  return myIsShrinkable;
}
  
void
SVTK_DeviceActor
::SetShrinkable(bool theIsShrinkable) 
{ 
  myIsShrinkable = theIsShrinkable;
}
  
bool
SVTK_DeviceActor
::IsShrunk() 
{ 
  return myIsShrunk;
}

void
SVTK_DeviceActor
::SetShrink() 
{
  if ( !myIsShrinkable ) 
    return;
  if ( vtkDataSet* aDataSet = myPassFilter[ 0 ]->GetOutput() )
  {
    myShrinkFilter->SetInput( aDataSet );
    myPassFilter[ 1 ]->SetInput( myShrinkFilter->GetOutput() );
    myIsShrunk = true;
  }
}

void 
SVTK_DeviceActor
::UnShrink() 
{
  if ( !myIsShrunk ) return;
  if ( vtkDataSet* aDataSet = myPassFilter[ 0 ]->GetOutput() )
  {    
    myPassFilter[ 1 ]->SetInput( aDataSet );
    myIsShrunk = false;
  }
}

float
SVTK_DeviceActor
::GetShrinkFactor()
{
  return myShrinkFilter->GetShrinkFactor();
}

void 
SVTK_DeviceActor
::SetShrinkFactor(float theValue)
{
  myShrinkFilter->SetShrinkFactor(theValue);
}



//----------------------------------------------------------------------------
void
SVTK_DeviceActor
::SetRepresentation(SVTK::Representation::Type theMode)
{ 
  using namespace SVTK::Representation;
  if(IsShaded()){
    switch(myRepresentation){
    case Points : 
    case Surface : 
      myProperty->DeepCopy(GetProperty());
    }
    
    switch(theMode){
    case Points : 
    case Surface : 
      GetProperty()->DeepCopy(myProperty);
      break;
    default:
      GetProperty()->SetAmbient(1.0);
      GetProperty()->SetDiffuse(0.0);
      GetProperty()->SetSpecular(0.0);
    }
  }

  switch(theMode){
  case Insideframe : 
    myGeomFilter->SetInside(true);
    myGeomFilter->SetWireframeMode(true);
    GetProperty()->SetRepresentation(VTK_WIREFRAME);
    break;
  case Points : 
    GetProperty()->SetPointSize(GetDefaultPointSize());  
    GetProperty()->SetRepresentation(VTK_POINTS);
    myGeomFilter->SetWireframeMode(false);
    myGeomFilter->SetInside(false);
    break;
  case Wireframe : 
    GetProperty()->SetRepresentation(VTK_WIREFRAME);
    myGeomFilter->SetWireframeMode(true);
    myGeomFilter->SetInside(false);
    break;
  case Surface : 
    GetProperty()->SetRepresentation(VTK_SURFACE);
    myGeomFilter->SetWireframeMode(false);
    myGeomFilter->SetInside(false);
    break;
  }

  myRepresentation = theMode;
}

SVTK::Representation::Type 
SVTK_DeviceActor
::GetRepresentation()
{
  return myRepresentation;
}

float
SVTK_DeviceActor
::GetDefaultPointSize()
{
  return 5;
}

float
SVTK_DeviceActor
::GetDefaultLineWidth()
{
  return 3;
}


bool
SVTK_DeviceActor
::IsShaded()
{
  return myIsShaded;
}

void
SVTK_DeviceActor
::SetShaded(bool theShaded)
{
  myIsShaded = theShaded;
}


//----------------------------------------------------------------------------
int
SVTK_DeviceActor
::GetNodeObjId(int theVtkID)
{
  return theVtkID;
}

float* 
SVTK_DeviceActor
::GetNodeCoord(int theObjID)
{
  return GetInput()->GetPoint(theObjID);
}


vtkCell* 
SVTK_DeviceActor
::GetElemCell(int theObjID)
{
  return GetInput()->GetCell(theObjID);
}

int
SVTK_DeviceActor
::GetElemObjId(int theVtkID) 
{ 
  return theVtkID;
}


//----------------------------------------------------------------------------
void
SVTK_DeviceActor
::Render(vtkRenderer *ren, vtkMapper* m)
{
  if(myIsResolveCoincidentTopology){
    int aResolveCoincidentTopology = vtkMapper::GetResolveCoincidentTopology();
    float aFactor, aUnit; 
    vtkMapper::GetResolveCoincidentTopologyPolygonOffsetParameters(aFactor,aUnit);
    
    vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
    vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(myPolygonOffsetFactor,
								   myPolygonOffsetUnits);
    Superclass::Render(ren,m);
    
    vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(aFactor,aUnit);
    vtkMapper::SetResolveCoincidentTopology(aResolveCoincidentTopology);
  }else{
    Superclass::Render(ren,m);
  }
}


void
SVTK_DeviceActor
::SetPolygonOffsetParameters(float factor, float units)
{
  myPolygonOffsetFactor = factor;
  myPolygonOffsetUnits = units;
}

void
SVTK_DeviceActor
::GetPolygonOffsetParameters(float& factor, float& units)
{
  factor = myPolygonOffsetFactor;
  units = myPolygonOffsetUnits;
}