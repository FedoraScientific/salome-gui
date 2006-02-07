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
//  File   : SVTK_DeviceActor.h
//  Author : 
//  Module : 
//  $Header$

#ifndef SVTK_DEVICE_ACTOR_H
#define SVTK_DEVICE_ACTOR_H

#include <vector>

#include <vtkLODActor.h>
#include <vtkProperty.h>

class VTKViewer_Transform;
class VTKViewer_TransformFilter;
class VTKViewer_PassThroughFilter;
class VTKViewer_GeometryFilter;

class vtkCell;
class vtkDataSet;
class vtkShrinkFilter;
class vtkDataSetMapper;

//----------------------------------------------------------------------------
namespace SVTK
{
  namespace Representation
  {
    typedef int Type;
    const Type Points = VTK_POINTS;
    const Type Wireframe = VTK_WIREFRAME;
    const Type Surface = VTK_SURFACE;
    const Type Insideframe = Surface + 1;
  }
}


//----------------------------------------------------------------------------
class SVTK_DeviceActor: public vtkLODActor
{
 public:
  vtkTypeMacro(SVTK_DeviceActor,vtkLODActor);

  static
  SVTK_DeviceActor* 
  New();

  //! Apply a view transformation
  virtual
  void
  SetTransform(VTKViewer_Transform* theTransform); 

  //! To insert some additional filters and then sets the given #vtkMapper
  virtual
  void
  SetMapper(vtkMapper* theMapper); 

  //! Allows to get initial #vtkDataSet
  virtual
  vtkDataSet* 
  GetInput(); 

  //! Allows to set initial #vtkDataSet
  virtual
  void
  SetInput(vtkDataSet* theDataSet); 

  /** @name For selection mapping purpose */
  //@{
  virtual
  int
  GetNodeObjId(int theVtkID);

  virtual
  float* 
  GetNodeCoord(int theObjID);

  virtual
  int
  GetElemObjId(int theVtkID);

  virtual
  vtkCell* 
  GetElemCell(int theObjID);

  //! To provide VTK to Object and backward mapping
  virtual 
  void
  SetStoreMapping(bool theStoreMapping);
  //@}

  virtual 
  unsigned long int 
  GetMTime();

  /** @name For shrink mamnagement purpose */
  //@{
  float
  GetShrinkFactor();

  virtual 
  void  
  SetShrinkFactor(float value);

  virtual
  void
  SetShrinkable(bool theIsShrinkable);

  bool
  IsShrunkable();

  bool
  IsShrunk();

  virtual
  void
  SetShrink(); 

  virtual
  void
  UnShrink(); 
  //@}

  /** @name For representation mamnagement purpose */
  virtual
  void 
  SetRepresentation(SVTK::Representation::Type theMode);

  SVTK::Representation::Type 
  GetRepresentation();

  virtual
  float
  GetDefaultPointSize();

  virtual
  float
  GetDefaultLineWidth();

  bool
  IsShaded();

  void
  SetShaded(bool theShaded);
  //@}

  virtual
  void
  Render(vtkRenderer *, vtkMapper *);

 protected:
  SVTK::Representation::Type myRepresentation;
  vtkProperty *myProperty;
  bool myIsShaded;

  //! To initialize internal pipeline
  void
  InitPipeLine(vtkMapper* theMapper); 

  VTKViewer_GeometryFilter *myGeomFilter;
  VTKViewer_TransformFilter *myTransformFilter;
  std::vector<VTKViewer_PassThroughFilter*> myPassFilter;
  vtkShrinkFilter* myShrinkFilter;
  vtkDataSetMapper* myMapper;

  bool myIsShrinkable;
  bool myIsShrunk;
  
  bool myIsResolveCoincidentTopology;
  float myPolygonOffsetFactor;
  float myPolygonOffsetUnits;

  void SetPolygonOffsetParameters(float factor, float units);
  void GetPolygonOffsetParameters(float& factor, float& units);

  SVTK_DeviceActor();
  ~SVTK_DeviceActor();

 private:
  SVTK_DeviceActor(const SVTK_DeviceActor&); // Not implemented
  void operator=(const SVTK_DeviceActor&); // Not implemented

};


#endif //SVTK_DEVICE_ACTOR_H
