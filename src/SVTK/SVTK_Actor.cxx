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

#include "SVTK_Actor.h"
#include "SALOME_Actor.h"

// VTK Includes
#include <vtkObjectFactory.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkRenderer.h>

#include <vtkCell.h>
#include <vtkPolyData.h>

using namespace std;


static 
void
CopyPoints(vtkUnstructuredGrid* theGrid, vtkDataSet *theSourceDataSet)
{
  vtkPoints *aPoints = vtkPoints::New();
  vtkIdType iEnd = theSourceDataSet->GetNumberOfPoints();
  aPoints->SetNumberOfPoints(iEnd);
  for(vtkIdType i = 0; i < iEnd; i++){
    aPoints->SetPoint(i,theSourceDataSet->GetPoint(i));
  }
  theGrid->SetPoints(aPoints);
  aPoints->Delete();
}

//=======================================================================

vtkStandardNewMacro(SVTK_Actor);

SVTK_Actor
::SVTK_Actor():
  myUnstructuredGrid(vtkUnstructuredGrid::New())
{
  myIsShaded = true;

  Visibility = Pickable = false;

  myUnstructuredGrid->Delete();
  myUnstructuredGrid->Allocate();
}

//----------------------------------------------------------------------------
void
SVTK_Actor
::Initialize()
{
  SetInput(GetSource());
}


//----------------------------------------------------------------------------
void
SVTK_Actor
::SetSource(vtkUnstructuredGrid* theUnstructuredGrid)
{
  if(GetSource() == theUnstructuredGrid)
    return;

  myUnstructuredGrid = theUnstructuredGrid;

  SetInput(theUnstructuredGrid);
}

vtkUnstructuredGrid*
SVTK_Actor
::GetSource()
{
  return myUnstructuredGrid.GetPointer();
}


//----------------------------------------------------------------------------
SVTK_Actor
::~SVTK_Actor()
{
}


//----------------------------------------------------------------------------
const TColStd_IndexedMapOfInteger&
SVTK_Actor
::GetMapIndex() const
{
  return myMapIndex;
}


//----------------------------------------------------------------------------
void
SVTK_Actor
::MapCells(SALOME_Actor* theMapActor,
	   const TColStd_IndexedMapOfInteger& theMapIndex)
{
  myUnstructuredGrid->Initialize();
  myUnstructuredGrid->Allocate();

  vtkDataSet *aSourceDataSet = theMapActor->GetInput();
  CopyPoints(GetSource(),aSourceDataSet);

  int aNbOfParts = theMapIndex.Extent();
  for(int ind = 1; ind <= aNbOfParts; ind++){
    int aPartId = theMapIndex( ind );
    if(vtkCell* aCell = theMapActor->GetElemCell(aPartId))
      myUnstructuredGrid->InsertNextCell(aCell->GetCellType(),aCell->GetPointIds());
  }

  UnShrink();
  if(theMapActor->IsShrunk()){
    SetShrinkFactor(theMapActor->GetShrinkFactor());
    SetShrink();
  }

  myMapIndex = theMapIndex;
}


//----------------------------------------------------------------------------
void 
SVTK_Actor
::MapPoints(SALOME_Actor* theMapActor,
	    const TColStd_IndexedMapOfInteger& theMapIndex)
{
  myUnstructuredGrid->Initialize();
  myUnstructuredGrid->Allocate();

  if(int aNbOfParts = theMapIndex.Extent()){
    vtkPoints *aPoints = vtkPoints::New();
    aPoints->SetNumberOfPoints(aNbOfParts);
    for(int i = 0; i < aNbOfParts; i++){
      int aPartId = theMapIndex( i+1 );
      if(float* aCoord = theMapActor->GetNodeCoord(aPartId)){
	aPoints->SetPoint(i,aCoord);
	myUnstructuredGrid->InsertNextCell(VTK_VERTEX,1,&i);
      }
    }
    myUnstructuredGrid->SetPoints(aPoints);
    aPoints->Delete();
  }

  UnShrink();

  myMapIndex = theMapIndex;
}


//----------------------------------------------------------------------------
void
SVTK_Actor
::MapEdge(SALOME_Actor* theMapActor,
	  const TColStd_IndexedMapOfInteger& theMapIndex)
{
  myUnstructuredGrid->Initialize();
  myUnstructuredGrid->Allocate();

  vtkDataSet *aSourceDataSet = theMapActor->GetInput();
  CopyPoints(GetSource(),aSourceDataSet);


  if(theMapIndex.Extent() == 2){
    int anEdgeId = theMapIndex(1) < 0 ? theMapIndex(1) : theMapIndex(2);
    int aCellId = theMapIndex(1) < 0 ? theMapIndex(2) : theMapIndex(1);

    if(aCellId > 0){
      if(vtkCell* aCell = theMapActor->GetElemCell(aCellId)){
	if(anEdgeId < 0){
	  anEdgeId = -anEdgeId - 1;
	  int aNbOfEdges = aCell->GetNumberOfEdges();
	  if(0 <= anEdgeId || anEdgeId < aNbOfEdges){
	    if(vtkCell* anEdge = aCell->GetEdge(anEdgeId))
	      myUnstructuredGrid->InsertNextCell(VTK_LINE,anEdge->GetPointIds());
	  }
	}
      }
    }
  }

  UnShrink();
  if(theMapActor->IsShrunk()){
    SetShrinkFactor(theMapActor->GetShrinkFactor());
    SetShrink();
  }

  myMapIndex = theMapIndex;
}

//----------------------------------------------------------------------------
