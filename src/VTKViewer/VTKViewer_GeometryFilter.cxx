// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  File   : VTKViewer_GeometryFilter.cxx
//  Author : Michael ZORIN
//  Module : SALOME
//
#include "VTKViewer_GeometryFilter.h"
#include "VTKViewer_ConvexTool.h"
#include "VTKViewer_ArcBuilder.h"

#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkGenericCell.h>
#include <vtkHexagonalPrism.h>
#include <vtkHexahedron.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMergePoints.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkPyramid.h>
#include <vtkStructuredGrid.h>
#include <vtkTetra.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVoxel.h>
#include <vtkWedge.h>

#include <algorithm>
#include <iterator>
#include <vector>
#include <map>
#include <set>

#include "utilities.h"

#if defined __GNUC__
  #if __GNUC__ == 2
    #define __GNUC_2__
  #endif
#endif

#define VTK_XVERSION (VTK_MAJOR_VERSION*10000+VTK_MINOR_VERSION*100+VTK_BUILD_VERSION)

//#define __MYDEBUG__
//#define USE_ROBUST_TRIANGULATION

///////////////////////////////////////////////////////////////////////////////////////////////
// VSR 26/10/2012: fix of regression (issue 21924) - increased memory consumption
// for displaying of 3d elements, introduced by fix for issue 20314.
// ...
// The macro SHOW_COINCIDING_3D_PAL20314, when defined, allows correct visualization of
// coincident 3d elements but causes substantial increasing of memory consumption, as all 3d 
// elements are always shown, even if they are totally covered by surrounding faces.
// If this macro is not defined (commented), the behaviour is defined by another macro -
// SHOW_COINCIDING_3D_PAL21924, as follows:
// - If SHOW_COINCIDING_3D_PAL21924 is defined, an alternative solution for computing 
//   visibility of 3d elements is used; this solution allows to fix problem with visibility
//   of coinciding 3d elements in most cases (though some cases might not work), while not
//   causing significant increasing of memory consumption.
// - If SHOW_COINCIDING_3D_PAL21924 is not defined (commented), coinciding 3d elements are 
//   not shown at all (this corresponds to the state before issue 20314 fixing).
///////////////////////////////////////////////////////////////////////////////////////////////
//#define SHOW_COINCIDING_3D_PAL20314
#ifndef SHOW_COINCIDING_3D_PAL20314
#define SHOW_COINCIDING_3D_PAL21924
#endif
///////////////////////////////////////////////////////////////////////////////////////////////

vtkCxxRevisionMacro(VTKViewer_GeometryFilter, "$Revision$");
vtkStandardNewMacro(VTKViewer_GeometryFilter);

VTKViewer_GeometryFilter
::VTKViewer_GeometryFilter():
  myShowInside(0),
  myStoreMapping(0),
  myAppendCoincident3D(0),
  myIsWireframeMode(0),
  myIsBuildArc(false),
  myMaxArcAngle(2)
{}


VTKViewer_GeometryFilter
::~VTKViewer_GeometryFilter()
{}


int
VTKViewer_GeometryFilter
::RequestData(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numCells=input->GetNumberOfCells();

  if (numCells == 0)
    {
      return 0;
    }

  if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID){
    return this->UnstructuredGridExecute(input, output, outInfo);
  }else
    return Superclass::RequestData(request,inputVector,outputVector);

  return 1;
}


int
VTKViewer_GeometryFilter
::UnstructuredGridExecute(
                          vtkDataSet *dataSetInput,
                          vtkPolyData *output,
                          vtkInformation *outInfo)
{

  vtkUnstructuredGrid *input= (vtkUnstructuredGrid *)dataSetInput;
  vtkCellArray *Connectivity = input->GetCells();
  // Check input
  if ( Connectivity == NULL )
    {
    vtkDebugMacro(<<"Nothing to extract");
    return 0;
    }

  vtkIdType cellId;
  int i;
  int allVisible;
  vtkIdType npts = 0;
  vtkIdType *pts = 0;
  vtkPoints *p = input->GetPoints();
  vtkIdType numCells=input->GetNumberOfCells();
  vtkPointData *pd = input->GetPointData();
  vtkCellData *cd = input->GetCellData();
  vtkPointData *outputPD = output->GetPointData();

  VTKViewer_OrderedTriangulator anOrderedTriangulator;
  VTKViewer_DelaunayTriangulator aDelaunayTriangulator;

  vtkCellData *outputCD = output->GetCellData();
  vtkGenericCell *cell = vtkGenericCell::New();

  vtkIdList *cellIds = vtkIdList::New();
  vtkIdList *faceIds = vtkIdList::New();
  vtkIdList *cellIdsTmp = vtkIdList::New();
  vtkIdList *faceIdsTmp = vtkIdList::New();

  char *cellVis;
  vtkIdType newCellId;
  int faceId, *faceVerts, numFacePts;
  vtkFloatingPointType *x;
  vtkIdType PixelConvert[4];
  // Change the type from int to vtkIdType in order to avoid compilation errors while using VTK
  // from ParaView-3.4.0 compiled on 64-bit Debian platform with VTK_USE_64BIT_IDS = ON
  vtkIdType aNewPts[VTK_CELL_SIZE];
  // ghost cell stuff
  unsigned char  updateLevel = (unsigned char)(output->GetUpdateGhostLevel());
  unsigned char  *cellGhostLevels = 0;

  PixelConvert[0] = 0;
  PixelConvert[1] = 1;
  PixelConvert[2] = 3;
  PixelConvert[3] = 2;

  vtkDebugMacro(<<"Executing geometry filter for unstructured grid input");

  vtkDataArray* temp = 0;
  if (cd)
    {
    temp = cd->GetArray("vtkGhostLevels");
    }
  if ( (!temp) || (temp->GetDataType() != VTK_UNSIGNED_CHAR)
    || (temp->GetNumberOfComponents() != 1))
    {
    vtkDebugMacro("No appropriate ghost levels field available.");
    }
  else
    {
    cellGhostLevels = ((vtkUnsignedCharArray*)temp)->GetPointer(0);
    }

  // Determine nature of what we have to do
  if ( (!this->CellClipping) && (!this->PointClipping) &&
       (!this->ExtentClipping) )
    {
    allVisible = 1;
    cellVis = NULL;
    }
  else
    {
    allVisible = 0;
    cellVis = new char[numCells];
    }

  // Issue 0020115: [CEA 308] Quadratic elements visualization
  // Fix of remark described in note 0005222 - SIGSEGV
  vtkPoints* outputPoints = vtkPoints::New();
  outputPoints->DeepCopy(input->GetPoints());
  output->SetPoints(outputPoints);
  outputPoints->Delete();

  outputPD->PassData(pd);

  outputCD->CopyAllocate(cd,numCells,numCells/2);

  output->Allocate(numCells/4+1,numCells);

  // Loop over the cells determining what's visible
  if (!allVisible)
    {
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
      {
      cellVis[cellId] = 1;
      if ( ( this->CellClipping && cellId < this->CellMinimum ) ||
           cellId > this->CellMaximum )
        {
        cellVis[cellId] = 0;
        }
      else
        {
        for (i=0; i < npts; i++)
          {
          x = p->GetPoint(pts[i]);
          if ( ( ( ( this->PointClipping && (pts[i] < this->PointMinimum ) ) ||
                                             pts[i] > this->PointMaximum) ) ||
               ( this->ExtentClipping &&
                ( x[0] < this->Extent[0] || x[0] > this->Extent[1] ||
                  x[1] < this->Extent[2] || x[1] > this->Extent[3] ||
                  x[2] < this->Extent[4] || x[2] > this->Extent[5] )) )
            {
            cellVis[cellId] = 0;
            break;
            }//point/extent clipping
          }//for each point
        }//if point clipping needs checking
      }//for all cells
    }//if not all visible

  // Loop over all cells now that visibility is known
  // (Have to compute visibility first for 3D cell boundarys)
  int progressInterval = numCells/20 + 1;
  TMapOfVectorId aDimension2VTK2ObjIds;
  if(myStoreMapping){
    myVTK2ObjIds.clear();
    myVTK2ObjIds.reserve(numCells);
  }
  for (cellId=0, Connectivity->InitTraversal();
       Connectivity->GetNextCell(npts,pts);
       cellId++)
    {
    //Progress and abort method support
    if ( !(cellId % progressInterval) )
      {
      vtkDebugMacro(<<"Process cell #" << cellId);
      this->UpdateProgress ((float)cellId/numCells);
      }

    // Handle ghost cells here.  Another option was used cellVis array.
    if (cellGhostLevels && cellGhostLevels[cellId] > updateLevel)
      { // Do not create surfaces in outer ghost cells.
      continue;
      }

    if (allVisible || cellVis[cellId])  //now if visible extract geometry
      {
      //special code for nonlinear cells - rarely occurs, so right now it
      //is slow.
      vtkIdType aCellType = input->GetCellType(cellId);
      switch (aCellType)
        {
        case VTK_EMPTY_CELL:
          break;

        case VTK_VERTEX:
        case VTK_POLY_VERTEX:
          newCellId = output->InsertNextCell(aCellType,npts,pts);
          if(myStoreMapping){
            InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
          }
          outputCD->CopyData(cd,cellId,newCellId);
          break;

        case VTK_LINE:
        case VTK_POLY_LINE:
          newCellId = output->InsertNextCell(aCellType,npts,pts);
          if(myStoreMapping)
            InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
          outputCD->CopyData(cd,cellId,newCellId);
          break;

        case VTK_TRIANGLE:
        case VTK_QUAD:
        case VTK_POLYGON:
          newCellId = output->InsertNextCell(aCellType,npts,pts);
          if(myStoreMapping)
            InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
          outputCD->CopyData(cd,cellId,newCellId);
          break;

        case VTK_TRIANGLE_STRIP:
          newCellId = output->InsertNextCell(aCellType,npts,pts);
          if(myStoreMapping)
            InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
          outputCD->CopyData(cd,cellId,newCellId);
          break;

        case VTK_PIXEL:
          newCellId = output->InsertNextCell(aCellType,npts,pts);
          if(myStoreMapping)
            InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
          outputCD->CopyData(cd,cellId,newCellId);
          break;

        case VTK_CONVEX_POINT_SET: {
          bool anIsOk = anOrderedTriangulator.Execute(input,
                                                      cd,
                                                      cellId,
                                                      myShowInside,
                                                      allVisible,
                                                      GetAppendCoincident3D(),
                                                      cellVis,
                                                      output,
                                                      outputCD,
                                                      myStoreMapping,
                                                      myVTK2ObjIds,
                                                      aDimension2VTK2ObjIds,
                                                      true);
          if(!anIsOk)
            aDelaunayTriangulator.Execute(input,
                                          cd,
                                          cellId,
                                          myShowInside,
                                          allVisible,
                                          GetAppendCoincident3D(),
                                          cellVis,
                                          output,
                                          outputCD,
                                          myStoreMapping,
                                          myVTK2ObjIds,
                                          aDimension2VTK2ObjIds,
                                          false);

          break;
        }
        case VTK_TETRA: {
#ifdef SHOW_COINCIDING_3D_PAL21924
	  faceIdsTmp->Reset();
	  for (int ai=0; ai<npts; ai++)
	    faceIdsTmp->InsertNextId(pts[ai]);
	  input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
#endif
          for (faceId = 0; faceId < 4; faceId++)
            {
            faceIds->Reset();
            faceVerts = vtkTetra::GetFaceArray(faceId);
            faceIds->InsertNextId(pts[faceVerts[0]]);
            faceIds->InsertNextId(pts[faceVerts[1]]);
            faceIds->InsertNextId(pts[faceVerts[2]]);
            aCellType = VTK_TRIANGLE;
            numFacePts = 3;
            input->GetCellNeighbors(cellId, faceIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
	    int nbNeighbors = 0;
	    for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
	      if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
	    }
            bool process = nbNeighbors <= 0;
#else
            bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
            if ( process || myShowInside ||
                 (!allVisible && !cellVis[cellIds->GetId(0)]) )
              {
              for ( i=0; i < numFacePts; i++)
                aNewPts[i] = pts[faceVerts[i]];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);
              }
            }
          break;
        }
        case VTK_VOXEL: {
#ifdef SHOW_COINCIDING_3D_PAL21924
	  faceIdsTmp->Reset();
	  for (int ai=0; ai<npts; ai++)
	    faceIdsTmp->InsertNextId(pts[ai]);
	  input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
#endif
          for (faceId = 0; faceId < 6; faceId++)
            {
            faceIds->Reset();
            faceVerts = vtkVoxel::GetFaceArray(faceId);
            faceIds->InsertNextId(pts[faceVerts[0]]);
            faceIds->InsertNextId(pts[faceVerts[1]]);
            faceIds->InsertNextId(pts[faceVerts[2]]);
            faceIds->InsertNextId(pts[faceVerts[3]]);
            aCellType = VTK_QUAD;
            numFacePts = 4;
            input->GetCellNeighbors(cellId, faceIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
	    int nbNeighbors = 0;
	    for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
	      if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
	    }
            bool process = nbNeighbors <= 0;
#else
            bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
            if ( process || myShowInside ||
                 (!allVisible && !cellVis[cellIds->GetId(0)]) )
              {
              for ( i=0; i < numFacePts; i++)
                aNewPts[i] = pts[faceVerts[PixelConvert[i]]];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);
              }
            }
          break;
        }
        case VTK_HEXAHEDRON: {
#ifdef SHOW_COINCIDING_3D_PAL21924
	  faceIdsTmp->Reset();
	  for (int ai=0; ai<npts; ai++)
	    faceIdsTmp->InsertNextId(pts[ai]);
	  input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
#endif
          for (faceId = 0; faceId < 6; faceId++)
            {
            faceIds->Reset();
            faceVerts = vtkHexahedron::GetFaceArray(faceId);
            faceIds->InsertNextId(pts[faceVerts[0]]);
            faceIds->InsertNextId(pts[faceVerts[1]]);
            faceIds->InsertNextId(pts[faceVerts[2]]);
            faceIds->InsertNextId(pts[faceVerts[3]]);
            aCellType = VTK_QUAD;
            numFacePts = 4;
            input->GetCellNeighbors(cellId, faceIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
	    int nbNeighbors = 0;
	    for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
	      if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
	    }
            bool process = nbNeighbors <= 0;
#else
            bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
            if ( process || myShowInside ||
                 (!allVisible && !cellVis[cellIds->GetId(0)]) )
              {
              for ( i=0; i < numFacePts; i++)
                aNewPts[i] = pts[faceVerts[i]];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);
              }
            }
          break;
        }
        case VTK_WEDGE: {
#ifdef SHOW_COINCIDING_3D_PAL21924
	  faceIdsTmp->Reset();
	  for (int ai=0; ai<npts; ai++)
	    faceIdsTmp->InsertNextId(pts[ai]);
	  input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
#endif
          for (faceId = 0; faceId < 5; faceId++)
            {
            faceIds->Reset();
            faceVerts = vtkWedge::GetFaceArray(faceId);
            faceIds->InsertNextId(pts[faceVerts[0]]);
            faceIds->InsertNextId(pts[faceVerts[1]]);
            faceIds->InsertNextId(pts[faceVerts[2]]);
            aCellType = VTK_TRIANGLE;
            numFacePts = 3;
            if (faceVerts[3] >= 0)
              {
              faceIds->InsertNextId(pts[faceVerts[3]]);
              aCellType = VTK_QUAD;
              numFacePts = 4;
              }

            input->GetCellNeighbors(cellId, faceIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
	    int nbNeighbors = 0;
	    for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
	      if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
	    }
            bool process = nbNeighbors <= 0;
#else
            bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
            if ( process || myShowInside ||
                 (!allVisible && !cellVis[cellIds->GetId(0)]) )
              {
              for ( i=0; i < numFacePts; i++)
                aNewPts[i] = pts[faceVerts[i]];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);
              }
            }
          break;
        }
        case VTK_HEXAGONAL_PRISM: {
#ifdef SHOW_COINCIDING_3D_PAL21924
	  faceIdsTmp->Reset();
	  for (int ai=0; ai<npts; ai++)
	    faceIdsTmp->InsertNextId(pts[ai]);
	  input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
#endif
          for (faceId = 0; faceId < 8; faceId++)
          {
            faceVerts = vtkHexagonalPrism::GetFaceArray(faceId);
            faceIds->Reset();
            faceIds->InsertNextId(pts[faceVerts[0]]);
            faceIds->InsertNextId(pts[faceVerts[1]]);
            faceIds->InsertNextId(pts[faceVerts[2]]);
            faceIds->InsertNextId(pts[faceVerts[3]]);
            aCellType = VTK_QUAD;
            numFacePts = 4;
            if (faceVerts[5] >= 0)
            {
              faceIds->InsertNextId(pts[faceVerts[4]]);
              faceIds->InsertNextId(pts[faceVerts[5]]);
              aCellType = VTK_POLYGON;
              numFacePts = 6;
            }
            input->GetCellNeighbors(cellId, faceIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
	    int nbNeighbors = 0;
	    for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
	      if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
	    }
            bool process = nbNeighbors <= 0;
#else
            bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
            if ( process || myShowInside ||
                 (!allVisible && !cellVis[cellIds->GetId(0)]) )
            {
              for ( i=0; i < numFacePts; i++)
                aNewPts[i] = pts[faceVerts[i]];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);
            }
            }
          break;
        }
        case VTK_PYRAMID: {
#ifdef SHOW_COINCIDING_3D_PAL21924
	  faceIdsTmp->Reset();
	  for (int ai=0; ai<npts; ai++)
	    faceIdsTmp->InsertNextId(pts[ai]);
	  input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
#endif
          for (faceId = 0; faceId < 5; faceId++)
            {
            faceIds->Reset();
            faceVerts = vtkPyramid::GetFaceArray(faceId);
            faceIds->InsertNextId(pts[faceVerts[0]]);
            faceIds->InsertNextId(pts[faceVerts[1]]);
            faceIds->InsertNextId(pts[faceVerts[2]]);
            aCellType = VTK_TRIANGLE;
            numFacePts = 3;
            if (faceVerts[3] >= 0)
              {
              faceIds->InsertNextId(pts[faceVerts[3]]);
              aCellType = VTK_QUAD;
              numFacePts = 4;
              }
            input->GetCellNeighbors(cellId, faceIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
	    int nbNeighbors = 0;
	    for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
	      if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
	    }
            bool process = nbNeighbors <= 0;
#else
            bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
            if ( process || myShowInside ||
                 (!allVisible && !cellVis[cellIds->GetId(0)]) )
              {
              for ( i=0; i < numFacePts; i++)
                aNewPts[i] = pts[faceVerts[i]];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);
              }
            }
          break;
        }

#if VTK_XVERSION > 50700
        case VTK_POLYHEDRON:
          {
            //MESSAGE("VTK_POLYHEDRON geometry filter");
            vtkIdType nFaces = 0;
            vtkIdType* ptIds = 0;
            int idp = 0;
            input->GetFaceStream(cellId, nFaces, ptIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
	    faceIdsTmp->Reset();
	    for (int ai=0; ai<npts; ai++)
	      faceIdsTmp->InsertNextId(pts[ai]);
	    input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
#endif
            for (faceId = 0; faceId < nFaces; faceId++)
              {
                faceIds->Reset();
                numFacePts = ptIds[idp];
                //MESSAGE("numFacePts="<< numFacePts);
                int pt0 = ++idp;
                for (i = 0; i < numFacePts; i++)
                  {
                    //MESSAGE("ptIds[" << idp + i << "]=" << ptIds[idp + i]);
                    faceIds->InsertNextId(ptIds[idp + i]);
                  }
                idp += numFacePts;
                switch (numFacePts)
                  {
                  case 3:
                    aCellType = VTK_TRIANGLE;
                    break;
                  case 4:
                    aCellType = VTK_QUAD;
                    break;
                  default:
                    aCellType = VTK_POLYGON;
                    break;
                  }
                // TODO understand and fix display of several polyhedrons                
                input->GetCellNeighbors(cellId, faceIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
		int nbNeighbors = 0;
		for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
		  if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
		}
		bool process = nbNeighbors <= 0;
#else
		bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
                if (process || myShowInside
                    || (!allVisible && !cellVis[cellIds->GetId(0)]))
                  {
                    for (i = 0; i < numFacePts; i++)
                      aNewPts[i] = ptIds[pt0 + i];
                    newCellId = output->InsertNextCell(aCellType, numFacePts, aNewPts);
                    if (myStoreMapping)
                      InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
                    outputCD->CopyData(cd, cellId, newCellId);
                  }
              }
            break;
          }
#endif
        //Quadratic cells
        case VTK_QUADRATIC_EDGE:
        case VTK_QUADRATIC_TRIANGLE:
        case VTK_QUADRATIC_QUAD:
        case VTK_BIQUADRATIC_QUAD:
        case VTK_QUADRATIC_TETRA:
        case VTK_QUADRATIC_HEXAHEDRON:
        case VTK_TRIQUADRATIC_HEXAHEDRON:
        case VTK_QUADRATIC_WEDGE:
        case VTK_QUADRATIC_PYRAMID:
          if(!myIsWireframeMode) {
            input->GetCell(cellId,cell);
            vtkIdList *lpts = vtkIdList::New();
            vtkPoints *coords = vtkPoints::New();
            vtkIdList *cellIds = vtkIdList::New();
            vtkIdType newCellId;

            if ( cell->GetCellDimension() == 1 ) {
              vtkIdType arcResult = -1;
              if(myIsBuildArc) {
                arcResult = Build1DArc(cellId, input, output, pts, myMaxArcAngle);
                newCellId = arcResult;
              }

              if(!myIsBuildArc || arcResult == -1 ) {
                aCellType = VTK_LINE;
                numFacePts = 2;
                cell->Triangulate(0,lpts,coords);
                for (i=0; i < lpts->GetNumberOfIds(); i+=2) {
                  aNewPts[0] = lpts->GetId(i);
                  aNewPts[1] = lpts->GetId(i+1);
                  newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
                  if(myStoreMapping)
                    InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
                  outputCD->CopyData(cd,cellId,newCellId);
                }
              }
              else {
                if(myStoreMapping)
                  InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
                outputCD->CopyData(cd,cellId,newCellId);
              }
            }
            else if ( cell->GetCellDimension() == 2 ) {
              if(!myIsBuildArc) {
                aCellType = VTK_TRIANGLE;
                numFacePts = 3;
                cell->Triangulate(0,lpts,coords);
                for (i=0; i < lpts->GetNumberOfIds(); i+=3) {
                  aNewPts[0] = lpts->GetId(i);
                  aNewPts[1] = lpts->GetId(i+1);
                  aNewPts[2] = lpts->GetId(i+2);
                  newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
                  if(myStoreMapping)
                    InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
                  outputCD->CopyData(cd,cellId,newCellId);
                }
              }
              else{
                BuildArcedPolygon(cellId,input,output,aDimension2VTK2ObjIds,true);
              }
            }
            else //3D nonlinear cell
            {
#ifdef SHOW_COINCIDING_3D_PAL21924
	      faceIdsTmp->Reset();
	      int npts1 = 0;
	      switch (aCellType ){
	      case VTK_QUADRATIC_TETRA:         npts1 = 4; break;
	      case VTK_QUADRATIC_HEXAHEDRON:    npts1 = 8; break;
	      case VTK_TRIQUADRATIC_HEXAHEDRON: npts1 = 8; break;
	      case VTK_QUADRATIC_WEDGE:         npts1 = 6; break;
	      case VTK_QUADRATIC_PYRAMID:       npts1 = 5; break;
	      }
	      if ( npts1 > 0 ) {
		for (int ai=0; ai<npts; ai++)
		  faceIdsTmp->InsertNextId(pts[ai]);
		input->GetCellNeighbors(cellId, faceIdsTmp, cellIdsTmp);
	      }
#endif
              aCellType = VTK_TRIANGLE;
              numFacePts = 3;
              for (int j=0; j < cell->GetNumberOfFaces(); j++){
                vtkCell *face = cell->GetFace(j);
                input->GetCellNeighbors(cellId, face->PointIds, cellIds);
#ifdef SHOW_COINCIDING_3D_PAL21924
		int nbNeighbors = 0;
		for(int ai=0;ai<cellIds->GetNumberOfIds();ai++) {
		  if (cellIdsTmp->IsId(cellIds->GetId(ai)) == -1) nbNeighbors++;
		}
		bool process = nbNeighbors <= 0;
#else
		bool process = cellIds->GetNumberOfIds() <= 0 || GetAppendCoincident3D();
#endif
                if ( process || myShowInside ) {
                  face->Triangulate(0,lpts,coords);
                  for (i=0; i < lpts->GetNumberOfIds(); i+=3) {
                    aNewPts[0] = lpts->GetId(i);
                    aNewPts[1] = lpts->GetId(i+1);
                    aNewPts[2] = lpts->GetId(i+2);
                    newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
                    if(myStoreMapping)
                      InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
                    outputCD->CopyData(cd,cellId,newCellId);
                  }
                }
              }
            } //3d nonlinear cell
            cellIds->Delete();
            coords->Delete();
            lpts->Delete();
            break;
          } else { // wireframe
            switch(aCellType) {
            case VTK_QUADRATIC_EDGE: {
              vtkIdType arcResult =-1;
              if(myIsBuildArc) {
               arcResult = Build1DArc(cellId, input, output, pts,myMaxArcAngle);
               newCellId = arcResult;
              }
              if(!myIsBuildArc || arcResult == -1) {
                aCellType = VTK_POLY_LINE;
                numFacePts = 3;

                aNewPts[0] = pts[0];
                aNewPts[2] = pts[1];
                aNewPts[1] = pts[2];

                newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              }

              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);
              break;
            }
            case VTK_QUADRATIC_TRIANGLE: {
              if(!myIsBuildArc) {
                aCellType = VTK_POLYGON;
                numFacePts = 6;

                aNewPts[0] = pts[0];
                aNewPts[1] = pts[3];
                aNewPts[2] = pts[1];
                aNewPts[3] = pts[4];
                aNewPts[4] = pts[2];
                aNewPts[5] = pts[5];

                newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
                if(myStoreMapping)
                  InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

                outputCD->CopyData(cd,cellId,newCellId);
              }
              else
                BuildArcedPolygon(cellId,input,output,aDimension2VTK2ObjIds);
              break;
            }
            case VTK_BIQUADRATIC_QUAD:
            case VTK_QUADRATIC_QUAD: {
              if(!myIsBuildArc) {
                aCellType = VTK_POLYGON;
                numFacePts = 8;

                aNewPts[0] = pts[0];
                aNewPts[1] = pts[4];
                aNewPts[2] = pts[1];
                aNewPts[3] = pts[5];
                aNewPts[4] = pts[2];
                aNewPts[5] = pts[6];
                aNewPts[6] = pts[3];
                aNewPts[7] = pts[7];

                newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
                if(myStoreMapping)
                  InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

                outputCD->CopyData(cd,cellId,newCellId);
              }
              else
                BuildArcedPolygon(cellId,input,output,aDimension2VTK2ObjIds);
              break;
            }
            case VTK_QUADRATIC_TETRA: {
              aCellType = VTK_POLYGON;
              numFacePts = 6;

              //---------------------------------------------------------------
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[4];
              aNewPts[2] = pts[1];
              aNewPts[3] = pts[5];
              aNewPts[4] = pts[2];
              aNewPts[5] = pts[6];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[7];
              aNewPts[2] = pts[3];
              aNewPts[3] = pts[8];
              aNewPts[4] = pts[1];
              aNewPts[5] = pts[4];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[1];
              aNewPts[1] = pts[8];
              aNewPts[2] = pts[3];
              aNewPts[3] = pts[9];
              aNewPts[4] = pts[2];
              aNewPts[5] = pts[5];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[2];
              aNewPts[1] = pts[9];
              aNewPts[2] = pts[3];
              aNewPts[3] = pts[7];
              aNewPts[4] = pts[0];
              aNewPts[5] = pts[6];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              break;
            }
            case VTK_QUADRATIC_WEDGE: {
              aCellType = VTK_POLYGON;
              numFacePts = 6;
              //---------------------------------------------------------------
              //Face 1
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[6];
              aNewPts[2] = pts[1];
              aNewPts[3] = pts[7];
              aNewPts[4] = pts[2];
              aNewPts[5] = pts[8];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              //Face 2
              aNewPts[0] = pts[3];
              aNewPts[1] = pts[9];
              aNewPts[2] = pts[4];
              aNewPts[3] = pts[10];
              aNewPts[4] = pts[5];
              aNewPts[5] = pts[11];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              //Face 3
              numFacePts = 8;
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[8];
              aNewPts[2] = pts[2];
              aNewPts[3] = pts[14];
              aNewPts[4] = pts[5];
              aNewPts[5] = pts[11];
              aNewPts[6] = pts[3];
              aNewPts[7] = pts[12];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              //Face 4
              aNewPts[0] = pts[1];
              aNewPts[1] = pts[13];
              aNewPts[2] = pts[4];
              aNewPts[3] = pts[10];
              aNewPts[4] = pts[5];
              aNewPts[5] = pts[14];
              aNewPts[6] = pts[2];
              aNewPts[7] = pts[7];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              //Face 5
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[12];
              aNewPts[2] = pts[3];
              aNewPts[3] = pts[9];
              aNewPts[4] = pts[4];
              aNewPts[5] = pts[13];
              aNewPts[6] = pts[1];
              aNewPts[7] = pts[6];
              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );
              outputCD->CopyData(cd,cellId,newCellId);
              break;
            }
            case VTK_TRIQUADRATIC_HEXAHEDRON:
            case VTK_QUADRATIC_HEXAHEDRON: {
              aCellType = VTK_POLYGON;
              numFacePts = 8;

              //---------------------------------------------------------------
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[8];
              aNewPts[2] = pts[1];
              aNewPts[3] = pts[17];
              aNewPts[4] = pts[5];
              aNewPts[5] = pts[12];
              aNewPts[6] = pts[4];
              aNewPts[7] = pts[16];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[1];
              aNewPts[1] = pts[9];
              aNewPts[2] = pts[2];
              aNewPts[3] = pts[18];
              aNewPts[4] = pts[6];
              aNewPts[5] = pts[13];
              aNewPts[6] = pts[5];
              aNewPts[7] = pts[17];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[2];
              aNewPts[1] = pts[10];
              aNewPts[2] = pts[3];
              aNewPts[3] = pts[19];
              aNewPts[4] = pts[7];
              aNewPts[5] = pts[14];
              aNewPts[6] = pts[6];
              aNewPts[7] = pts[18];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[3];
              aNewPts[1] = pts[11];
              aNewPts[2] = pts[0];
              aNewPts[3] = pts[16];
              aNewPts[4] = pts[4];
              aNewPts[5] = pts[15];
              aNewPts[6] = pts[7];
              aNewPts[7] = pts[19];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[8];
              aNewPts[2] = pts[1];
              aNewPts[3] = pts[9];
              aNewPts[4] = pts[2];
              aNewPts[5] = pts[10];
              aNewPts[6] = pts[3];
              aNewPts[7] = pts[11];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[4];
              aNewPts[1] = pts[12];
              aNewPts[2] = pts[5];
              aNewPts[3] = pts[13];
              aNewPts[4] = pts[6];
              aNewPts[5] = pts[14];
              aNewPts[6] = pts[7];
              aNewPts[7] = pts[15];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              break;
            }
            case VTK_QUADRATIC_PYRAMID: {
              aCellType = VTK_POLYGON;
              numFacePts = 6;

              //---------------------------------------------------------------
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[8];
              aNewPts[2] = pts[3];
              aNewPts[3] = pts[12];
              aNewPts[4] = pts[4];
              aNewPts[5] = pts[9];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[9];
              aNewPts[2] = pts[4];
              aNewPts[3] = pts[10];
              aNewPts[4] = pts[1];
              aNewPts[5] = pts[5];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[1];
              aNewPts[1] = pts[10];
              aNewPts[2] = pts[4];
              aNewPts[3] = pts[11];
              aNewPts[4] = pts[2];
              aNewPts[5] = pts[6];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              aNewPts[0] = pts[2];
              aNewPts[1] = pts[11];
              aNewPts[2] = pts[4];
              aNewPts[3] = pts[12];
              aNewPts[4] = pts[3];
              aNewPts[5] = pts[7];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              //---------------------------------------------------------------
              numFacePts = 8;
              aNewPts[0] = pts[0];
              aNewPts[1] = pts[5];
              aNewPts[2] = pts[1];
              aNewPts[3] = pts[6];
              aNewPts[4] = pts[2];
              aNewPts[5] = pts[7];
              aNewPts[6] = pts[3];
              aNewPts[7] = pts[8];

              newCellId = output->InsertNextCell(aCellType,numFacePts,aNewPts);
              if(myStoreMapping)
                InsertId( cellId, aCellType, myVTK2ObjIds, aDimension2VTK2ObjIds );

              outputCD->CopyData(cd,cellId,newCellId);

              break;
            } // case VTK_QUADRATIC_PYRAMID:
            } // switch by type
          } // end WIREFRAME
          break;
        } //switch
      } //if visible
    } //for all cells

  output->Squeeze();

  vtkDebugMacro(<<"Extracted " << input->GetNumberOfPoints() << " points,"
                << output->GetNumberOfCells() << " cells.");

  cell->Delete();

  cellIds->Delete();
  faceIds->Delete();
  cellIdsTmp->Delete();
  faceIdsTmp->Delete();

  if ( cellVis )
  {
    delete [] cellVis;
  }

  // to sort myVTK2ObjIds vector by cell dimension (ascending)
  if( myStoreMapping && !aDimension2VTK2ObjIds.empty() )
  {
    myVTK2ObjIds.clear();
    for( vtkIdType aDimension = 0; aDimension <= 2; aDimension++ )
    {
      TMapOfVectorId::const_iterator anIter = aDimension2VTK2ObjIds.find( aDimension );
      if( anIter != aDimension2VTK2ObjIds.end() )
      {
        const TVectorId& aCellIds = anIter->second;
        TVectorId::const_iterator anIdIter, anIdIterEnd = aCellIds.end();
        for( anIdIter = aCellIds.begin(); anIdIter != anIdIterEnd; anIdIter++ )
        {
          const vtkIdType aCellId = *anIdIter;
          myVTK2ObjIds.push_back( aCellId );
        }
      }
    }
  }

  return 1;
}

void
VTKViewer_GeometryFilter
::InsertId( const vtkIdType theCellId,
            const vtkIdType theCellType,
            TVectorId& theVTK2ObjIds,
            TMapOfVectorId& theDimension2VTK2ObjIds )
{
  theVTK2ObjIds.push_back( theCellId );

  int aDimension = 0;
  switch( theCellType )
  {
    case VTK_VERTEX:
    case VTK_POLY_VERTEX:
      aDimension = 0;
      break;
    case VTK_LINE:
    case VTK_POLY_LINE:
      aDimension = 1;
      break;
    case VTK_TRIANGLE:
    case VTK_TRIANGLE_STRIP:
    case VTK_POLYGON:
    case VTK_PIXEL:
    case VTK_QUAD:
      aDimension = 2;
      break;
  }

  TVectorId& aCellIds = theDimension2VTK2ObjIds[ aDimension ];
  aCellIds.push_back( theCellId );
}

void
VTKViewer_GeometryFilter
::SetInside(int theShowInside)
{
  if(myShowInside == theShowInside)
    return;

  myShowInside = theShowInside;
  this->Modified();
}

int
VTKViewer_GeometryFilter
::GetInside()
{
  return myShowInside;
}


void
VTKViewer_GeometryFilter
::SetWireframeMode(int theIsWireframeMode)
{
  if(myIsWireframeMode == theIsWireframeMode)
    return;

  myIsWireframeMode = theIsWireframeMode;
  this->Modified();
}

int
VTKViewer_GeometryFilter
::GetWireframeMode()
{
  return myIsWireframeMode;
}


void
VTKViewer_GeometryFilter
::SetStoreMapping(int theStoreMapping)
{
  if(myStoreMapping == theStoreMapping)
    return;

  myStoreMapping = theStoreMapping;
  this->Modified();
}

int
VTKViewer_GeometryFilter
::GetStoreMapping()
{
  return myStoreMapping;
}


vtkIdType VTKViewer_GeometryFilter::GetElemObjId( int theVtkID )
{
  if( theVtkID < 0 || theVtkID >= (int)myVTK2ObjIds.size() )
    return -1;
  return myVTK2ObjIds[theVtkID];
}


void VTKViewer_GeometryFilter::BuildArcedPolygon(vtkIdType cellId,
                                                 vtkUnstructuredGrid* input,
                                                 vtkPolyData *output,
                                                 TMapOfVectorId& theDimension2VTK2ObjIds,
                                                 bool triangulate){
  vtkIdType aCellType = VTK_POLYGON;
  vtkIdType *aNewPoints = NULL;
  vtkIdType aNbPoints = 0;
  vtkIdType newCellId;

  //Input and output cell data
  vtkCellData *cd = input->GetCellData();
  vtkCellData *outputCD = output->GetCellData();

  //Input and output scalars on point data
  vtkDataArray* inputScalars = input->GetPointData()->GetScalars();
  vtkDataArray* outputScalars = output->GetPointData()->GetScalars();

  std::vector<vtkPoints*> aCollection;
  std::vector< std::vector<double> > aScalarCollection;

  vtkCell* aCell = input->GetCell(cellId);
  switch(aCell->GetCellType()) {
    case VTK_QUADRATIC_TRIANGLE:
    {
      //Get All points from input cell
      Pnt P0 = CreatePnt( aCell, inputScalars, 0 );
      Pnt P1 = CreatePnt( aCell, inputScalars, 1 );
      Pnt P2 = CreatePnt( aCell, inputScalars, 2 );
      Pnt P3 = CreatePnt( aCell, inputScalars, 3 );
      Pnt P4 = CreatePnt( aCell, inputScalars, 4 );
      Pnt P5 = CreatePnt( aCell, inputScalars, 5 );

      VTKViewer_ArcBuilder aBuilder1(P0,P3,P1,myMaxArcAngle); //Build arc using 0, 3 and 1 points
#ifdef __MYDEBUG__
      cout << "Quadrangle arc 1 " << ( aBuilder1.GetStatus() == VTKViewer_ArcBuilder::Arc_Done ? "" : "NOT " ) << "done !!!" << endl;
#endif

      VTKViewer_ArcBuilder aBuilder2(P1,P4,P2,myMaxArcAngle); //Build arc using 1, 4 and 2 points
#ifdef __MYDEBUG__
      cout << "Quadrangle arc 2 " << ( aBuilder2.GetStatus() == VTKViewer_ArcBuilder::Arc_Done ? "" : "NOT " ) << "done !!!" << endl;
#endif

      VTKViewer_ArcBuilder aBuilder3(P2,P5,P0,myMaxArcAngle); //Build arc using 2, 5 and 0 points
#ifdef __MYDEBUG__
      cout << "Quadrangle arc 3 " << ( aBuilder3.GetStatus() == VTKViewer_ArcBuilder::Arc_Done ? "" : "NOT " ) << "done !!!" << endl;
#endif

      aCollection.push_back(aBuilder1.GetPoints());
      aCollection.push_back(aBuilder2.GetPoints());
      aCollection.push_back(aBuilder3.GetPoints());

      aScalarCollection.push_back(aBuilder1.GetScalarValues());
      aScalarCollection.push_back(aBuilder2.GetScalarValues());
      aScalarCollection.push_back(aBuilder3.GetScalarValues());
      break;
    }
    case VTK_BIQUADRATIC_QUAD:
    case VTK_QUADRATIC_QUAD:
    {
      //Get All points from input cell
      Pnt P0 = CreatePnt( aCell, inputScalars, 0 );
      Pnt P1 = CreatePnt( aCell, inputScalars, 1 );
      Pnt P2 = CreatePnt( aCell, inputScalars, 2 );
      Pnt P3 = CreatePnt( aCell, inputScalars, 3 );
      Pnt P4 = CreatePnt( aCell, inputScalars, 4 );
      Pnt P5 = CreatePnt( aCell, inputScalars, 5 );
      Pnt P6 = CreatePnt( aCell, inputScalars, 6 );
      Pnt P7 = CreatePnt( aCell, inputScalars, 7 );

      VTKViewer_ArcBuilder aBuilder1(P0,P4,P1,myMaxArcAngle); //Build arc using 0, 4 and 1 points
#ifdef __MYDEBUG__
      cout << "Quadrangle arc 1 " << ( aBuilder1.GetStatus() == VTKViewer_ArcBuilder::Arc_Done ? "" : "NOT " ) << "done !!!" << endl;
#endif

      VTKViewer_ArcBuilder aBuilder2(P1,P5,P2,myMaxArcAngle); //Build arc using 1, 5 and 2 points
#ifdef __MYDEBUG__
      cout << "Quadrangle arc 2 " << ( aBuilder2.GetStatus() == VTKViewer_ArcBuilder::Arc_Done ? "" : "NOT " ) << "done !!!" << endl;
#endif

      VTKViewer_ArcBuilder aBuilder3(P2,P6,P3,myMaxArcAngle); //Build arc using 2, 6 and 3 points
#ifdef __MYDEBUG__
      cout << "Quadrangle arc 3 " << ( aBuilder3.GetStatus() == VTKViewer_ArcBuilder::Arc_Done ? "" : "NOT " ) << "done !!!" << endl;
#endif

      VTKViewer_ArcBuilder aBuilder4(P3,P7,P0,myMaxArcAngle); //Build arc using 3, 7 and 0 points
#ifdef __MYDEBUG__
      cout << "Quadrangle arc 4 " << ( aBuilder4.GetStatus() == VTKViewer_ArcBuilder::Arc_Done ? "" : "NOT " ) << "done !!!" << endl;
#endif

      aCollection.push_back(aBuilder1.GetPoints());
      aCollection.push_back(aBuilder2.GetPoints());
      aCollection.push_back(aBuilder3.GetPoints());
      aCollection.push_back(aBuilder4.GetPoints());

      aScalarCollection.push_back(aBuilder1.GetScalarValues());
      aScalarCollection.push_back(aBuilder2.GetScalarValues());
      aScalarCollection.push_back(aBuilder3.GetScalarValues());
      aScalarCollection.push_back(aBuilder4.GetScalarValues());
      break;
    }
    default: //Unsupported cell type
      return;
  }

  if(triangulate){
    const vtkIdType numFacePts = 3;
    vtkIdList *pts = vtkIdList::New();
    vtkPoints *coords = vtkPoints::New();
    aCellType = VTK_TRIANGLE;
    vtkIdType aNewPts[numFacePts];
    vtkIdType aTriangleId;

    vtkPolygon *aPlg = vtkPolygon::New();
    std::map<int, double> aPntId2ScalarValue;
    aNbPoints = MergevtkPoints(aCollection, aScalarCollection, aPlg->GetPoints(), aPntId2ScalarValue, aNewPoints);
    aPlg->GetPointIds()->SetNumberOfIds(aNbPoints);

    for(vtkIdType i = 0; i < aNbPoints;i++) {
      aPlg->GetPointIds()->SetId(i, aNewPoints[i]);
    }

    aPlg->Triangulate(0,pts,coords);

    for (vtkIdType i=0; i < pts->GetNumberOfIds(); i+=3) {
      aNewPts[0] = output->GetPoints()->InsertNextPoint(coords->GetPoint(i));
      aNewPts[1] = output->GetPoints()->InsertNextPoint(coords->GetPoint(i+1));
      aNewPts[2] = output->GetPoints()->InsertNextPoint(coords->GetPoint(i+2));

      if(outputScalars) {
        outputScalars->InsertNextTuple1(aPntId2ScalarValue[pts->GetId(i)]);
        outputScalars->InsertNextTuple1(aPntId2ScalarValue[pts->GetId(i+1)]);
        outputScalars->InsertNextTuple1(aPntId2ScalarValue[pts->GetId(i+2)]);
      }

      aTriangleId = output->InsertNextCell(aCellType,numFacePts,aNewPts);

      if(myStoreMapping)
        InsertId( cellId, aCellType, myVTK2ObjIds, theDimension2VTK2ObjIds );
      outputCD->CopyData(cd,cellId,aTriangleId);
    }
    pts->Delete();
    coords->Delete();
    aPlg->Delete();
  }
  else {
    std::map<int, double> aPntId2ScalarValue;
    aNbPoints = MergevtkPoints(aCollection, aScalarCollection, output->GetPoints(), aPntId2ScalarValue, aNewPoints);
    if(outputScalars)
      for(vtkIdType i = 0; i < aNbPoints; i++)
        outputScalars->InsertNextTuple1(aPntId2ScalarValue[aNewPoints[i]]);
    newCellId = output->InsertNextCell(aCellType,aNbPoints,aNewPoints);
    outputCD->CopyData(cd,cellId,newCellId);

    if(myStoreMapping)
      InsertId( cellId, aCellType, myVTK2ObjIds, theDimension2VTK2ObjIds );
  }

  if (aNewPoints)
    delete [] aNewPoints;
}


void VTKViewer_GeometryFilter::SetQuadraticArcMode(bool theFlag)
{
  if(myIsBuildArc != theFlag) {
    myIsBuildArc = theFlag;
    this->Modified();
  }
}
bool VTKViewer_GeometryFilter::GetQuadraticArcMode() const
{
  return myIsBuildArc;
}

void VTKViewer_GeometryFilter::SetQuadraticArcAngle(vtkFloatingPointType theMaxAngle)
{
  if(myMaxArcAngle != theMaxAngle) {
    myMaxArcAngle = theMaxAngle;
    this->Modified();
  }
}

vtkFloatingPointType VTKViewer_GeometryFilter:: GetQuadraticArcAngle() const
{
  return myMaxArcAngle;
}

int VTKViewer_GeometryFilter::GetAppendCoincident3D() const {
// VSR 26/10/2012: see description of SHOW_COINCIDING_3D_PAL20314
// in the top of this file
#ifdef SHOW_COINCIDING_3D_PAL20314
  return myAppendCoincident3D;
#else
  return false;
#endif
}

void VTKViewer_GeometryFilter::SetAppendCoincident3D(int theFlag) {
  if(myAppendCoincident3D != theFlag){
    myAppendCoincident3D = theFlag;
    this->Modified();
  }
}
