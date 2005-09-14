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
//  File   : SVTK_InteractorStyle.cxx
//  Author : Christophe ATTANASIO
//  Module : SALOME
//  $Header$


#include "SVTK_InteractorStyle.h"

#include "VTKViewer_CellRectPicker.h"
#include "VTKViewer_Utilities.h"
#include "VTKViewer_RectPicker.h"

#include "SVTK_RenderWindowInteractor.h"
#include "SVTK_RenderWindow.h"
#include "SVTK_ViewWindow.h"

#include "SALOME_Actor.h"
#include "SVTK_Actor.h"
#include "SVTK_Selector.h"

#include "SALOME_ListIteratorOfListIO.hxx"
#include "SALOME_ListIO.hxx"

#include "SUIT_Session.h"
#include "CAM_Application.h"

#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkCommand.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkPicker.h>
#include <vtkPointPicker.h>
#include <vtkCellPicker.h>
#include <vtkLine.h> 
#include <vtkMapper.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>

#include <qapplication.h>
//VRV: porting on Qt 3.0.5
#if QT_VERSION >= 0x030005
#include <qpainter.h>
#endif
//VRV: porting on Qt 3.0.5
#include <algorithm>

using namespace std;


namespace
{
  int
  GetEdgeId(vtkPicker *thePicker, SALOME_Actor *theActor, int theObjId)
  {
    int anEdgeId = -1;
    if (vtkCell* aPickedCell = theActor->GetElemCell(theObjId)) {
      float aPickPosition[3];
      thePicker->GetPickPosition(aPickPosition);
      float aMinDist = 1000000.0, aDist = 0;
      for (int i = 0, iEnd = aPickedCell->GetNumberOfEdges(); i < iEnd; i++){
	if(vtkLine* aLine = vtkLine::SafeDownCast(aPickedCell->GetEdge(i))){
	  int subId;  float pcoords[3], closestPoint[3], weights[3];
	  aLine->EvaluatePosition(aPickPosition,closestPoint,subId,pcoords,aDist,weights);
	  if (aDist < aMinDist) {
	    aMinDist = aDist;
	    anEdgeId = i;
	  }
	}
      }
    }
    return anEdgeId;
  }
  

  
  bool CheckDimensionId(Selection_Mode theMode, SALOME_Actor *theActor, vtkIdType theObjId){
    switch(theMode){
    case CellSelection:
      return true;
    case EdgeSelection:
      return ( theActor->GetObjDimension( theObjId ) == 1 );
    case FaceSelection:
      return ( theActor->GetObjDimension( theObjId ) == 2 );
    case VolumeSelection:
      return ( theActor->GetObjDimension( theObjId ) == 3 );
    };
    return false;
  }
}  
  
//----------------------------------------------------------------------------
vtkStandardNewMacro(SVTK_InteractorStyle);
//----------------------------------------------------------------------------

SVTK_InteractorStyle
::SVTK_InteractorStyle() 
{
  myViewWindow = NULL;
  this->MotionFactor = 10.0;
  this->State = VTK_INTERACTOR_STYLE_CAMERA_NONE;
  this->RadianToDegree = 180.0 / vtkMath::Pi();
  this->ForcedState = VTK_INTERACTOR_STYLE_CAMERA_NONE;
  loadCursors();

  myPreSelectionActor = SVTK_Actor::New();
  myPreSelectionActor->GetProperty()->SetColor(0,1,1);
  myPreSelectionActor->GetProperty()->SetLineWidth(5);
  myPreSelectionActor->GetProperty()->SetPointSize(5);

  OnSelectionModeChanged();
}

//----------------------------------------------------------------------------
SVTK_InteractorStyle
::~SVTK_InteractorStyle() 
{
  myViewWindow->RemoveActor(myPreSelectionActor);
}

//----------------------------------------------------------------------------
SVTK_Selector*
SVTK_InteractorStyle
::GetSelector() 
{
  return myViewWindow->GetSelector();
}

//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::setPreselectionProp(const double& theRed, 
		      const double& theGreen, 
		      const double& theBlue, 
		      const int& theWidth) 
{
  if ( myPreSelectionActor->GetProperty() == 0 )
    return;
  myPreSelectionActor->GetProperty()->SetColor(theRed, theGreen, theBlue);
  myPreSelectionActor->GetProperty()->SetLineWidth(theWidth);
  myPreSelectionActor->GetProperty()->SetPointSize(theWidth);
}

//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::SetInteractor(vtkRenderWindowInteractor *theInteractor)
{
  myInteractor = dynamic_cast<SVTK_RenderWindowInteractor*>(theInteractor);
  Superclass::SetInteractor(theInteractor);
}

//----------------------------------------------------------------------------
int
SVTK_InteractorStyle
::GetState()
{
  return State | ForcedState;
}

//----------------------------------------------------------------------------
void 
SVTK_InteractorStyle
::setViewWindow(SVTK_ViewWindow* theViewWindow)
{
  myViewWindow = theViewWindow;
  myViewWindow->AddActor(myPreSelectionActor);
  myPreSelectionActor->Delete();
}

//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::setGUIWindow(QWidget* theWindow)
{
  myGUIWindow = theWindow;
}

//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::RotateXY(int dx, int dy)
{
  double rxf;
  double ryf;
  vtkCamera *cam;
  
  if (this->CurrentRenderer == NULL)
    {
      return;
    }
  
  int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();
  this->DeltaElevation = -20.0 / size[1];
  this->DeltaAzimuth = -20.0 / size[0];
  
  rxf = (double)dx * this->DeltaAzimuth *  this->MotionFactor;
  ryf = (double)dy * this->DeltaElevation * this->MotionFactor;
  
  cam = this->CurrentRenderer->GetActiveCamera();
  cam->Azimuth(rxf);
  cam->Elevation(ryf);
  cam->OrthogonalizeViewUp();
  ::ResetCameraClippingRange(this->CurrentRenderer); 
  //this->Interactor->Render();
  myGUIWindow->update();
}

//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::PanXY(int x, int y, int oldX, int oldY)
{
  TranslateView(x, y, oldX, oldY);   
  //this->Interactor->Render();
  myGUIWindow->update();
}


//----------------------------------------------------------------------------
void 
SVTK_InteractorStyle
::DollyXY(int dx, int dy)
{
  if (this->CurrentRenderer == NULL) return;

  double dxf = this->MotionFactor * (double)(dx) / (double)(this->CurrentRenderer->GetCenter()[1]);
  double dyf = this->MotionFactor * (double)(dy) / (double)(this->CurrentRenderer->GetCenter()[1]);

  double zoomFactor = pow((double)1.1, dxf + dyf);
  
  vtkCamera *aCam = this->CurrentRenderer->GetActiveCamera();
  if (aCam->GetParallelProjection())
    aCam->SetParallelScale(aCam->GetParallelScale()/zoomFactor);
  else{
    aCam->Dolly(zoomFactor);
    ::ResetCameraClippingRange(this->CurrentRenderer);
  }

  //this->Interactor->Render();
  myGUIWindow->update();
}

//----------------------------------------------------------------------------
void 
SVTK_InteractorStyle
::SpinXY(int x, int y, int oldX, int oldY)
{
  vtkCamera *cam;

  if (this->CurrentRenderer == NULL)
    {
      return;
    }

  double newAngle = atan2((double)(y - this->CurrentRenderer->GetCenter()[1]),
			  (double)(x - this->CurrentRenderer->GetCenter()[0]));
  double oldAngle = atan2((double)(oldY -this->CurrentRenderer->GetCenter()[1]),
			  (double)(oldX - this->CurrentRenderer->GetCenter()[0]));
  
  newAngle *= this->RadianToDegree;
  oldAngle *= this->RadianToDegree;

  cam = this->CurrentRenderer->GetActiveCamera();
  cam->Roll(newAngle - oldAngle);
  cam->OrthogonalizeViewUp();
      
  //this->Interactor->Render();
  myGUIWindow->update();
}


//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::OnMouseMove(int vtkNotUsed(ctrl), 
	      int shift,
	      int x, int y) 
{
  myShiftState = shift;
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
    onOperation(QPoint(x, y));
  else if (ForcedState == VTK_INTERACTOR_STYLE_CAMERA_NONE)
    onCursorMove(QPoint(x, y));
}


//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::OnLeftButtonDown(int ctrl, int shift, 
		   int x, int y) 
{
  if (this->HasObserver(vtkCommand::LeftButtonPressEvent)) {
    this->InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
    return;
  }
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == NULL) {
    return;
  }
  myShiftState = shift;
  // finishing current viewer operation
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  myOtherPoint = myPoint = QPoint(x, y);
  if (ForcedState != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    startOperation(ForcedState);
  } else {
    if (ctrl)
      startOperation(VTK_INTERACTOR_STYLE_CAMERA_ZOOM);
    else
      startOperation(VTK_INTERACTOR_STYLE_CAMERA_SELECT);
  }
  return;
}


//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::OnLeftButtonUp(int vtkNotUsed(ctrl),
		 int shift, 
		 int vtkNotUsed(x),
		 int vtkNotUsed(y))
{
  myShiftState = shift;
  // finishing current viewer operation
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
}


//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::OnMiddleButtonDown(int ctrl,
		     int shift, 
		     int x, int y) 
{
  if (this->HasObserver(vtkCommand::MiddleButtonPressEvent)) 
    {
      this->InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
      return;
    }
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == NULL)
    {
      return;
    }
  myShiftState = shift;
  // finishing current viewer operation
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  myOtherPoint = myPoint = QPoint(x, y);
  if (ForcedState != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    startOperation(ForcedState);
  }
  else {
    if (ctrl)
      startOperation(VTK_INTERACTOR_STYLE_CAMERA_PAN);
  }
}


//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::OnMiddleButtonUp(int vtkNotUsed(ctrl),
		   int shift, 
		   int vtkNotUsed(x),
		   int vtkNotUsed(y))
{
  myShiftState = shift;
  // finishing current viewer operation
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
}


//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::OnRightButtonDown(int ctrl,
		    int shift, 
		    int x, int y) 
{
  if (this->HasObserver(vtkCommand::RightButtonPressEvent)) 
    {
      this->InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
      return;
    }
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == NULL)
    {
      return;
    }
  myShiftState = shift;
  // finishing current viewer operation
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  myOtherPoint = myPoint = QPoint(x, y);
  if (ForcedState != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    startOperation(ForcedState);
  }
  else {
    if (ctrl)
      startOperation(VTK_INTERACTOR_STYLE_CAMERA_ROTATE);  
  }
}

//----------------------------------------------------------------------------
void
SVTK_InteractorStyle
::OnRightButtonUp(int vtkNotUsed(ctrl),
		  int shift, 
		  int vtkNotUsed(x),
		  int vtkNotUsed(y))
{
  myShiftState = shift;
  // finishing current viewer operation
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE) {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
}

//----------------------------------------------------------------------------
/* XPM */
const char* imageZoomCursor[] = { 
"32 32 3 1",
". c None",
"a c #000000",
"# c #ffffff",
"................................",
"................................",
".#######........................",
"..aaaaaaa.......................",
"................................",
".............#####..............",
"...........##.aaaa##............",
"..........#.aa.....a#...........",
".........#.a.........#..........",
".........#a..........#a.........",
"........#.a...........#.........",
"........#a............#a........",
"........#a............#a........",
"........#a............#a........",
"........#a............#a........",
".........#...........#.a........",
".........#a..........#a.........",
".........##.........#.a.........",
"........#####.....##.a..........",
".......###aaa#####.aa...........",
"......###aa...aaaaa.......#.....",
".....###aa................#a....",
"....###aa.................#a....",
"...###aa...............#######..",
"....#aa.................aa#aaaa.",
".....a....................#a....",
"..........................#a....",
"...........................a....",
"................................",
"................................",
"................................",
"................................"};

const char* imageRotateCursor[] = { 
"32 32 3 1",
". c None",
"a c #000000",
"# c #ffffff",
"................................",
"................................",
"................................",
"................................",
"........#.......................",
".......#.a......................",
"......#######...................",
".......#aaaaa#####..............",
"........#..##.a#aa##........##..",
".........a#.aa..#..a#.....##.aa.",
".........#.a.....#...#..##.aa...",
".........#a.......#..###.aa.....",
"........#.a.......#a..#aa.......",
"........#a.........#..#a........",
"........#a.........#a.#a........",
"........#a.........#a.#a........",
"........#a.........#a.#a........",
".........#.........#a#.a........",
"........##a........#a#a.........",
"......##.a#.......#.#.a.........",
"....##.aa..##.....##.a..........",
"..##.aa.....a#####.aa...........",
"...aa.........aaa#a.............",
"................#.a.............",
"...............#.a..............",
"..............#.a...............",
"...............a................",
"................................",
"................................",
"................................",
"................................",
"................................"};


//----------------------------------------------------------------------------
// loads cursors for viewer operations - zoom, pan, etc...
void
SVTK_InteractorStyle
::loadCursors()
{
  myDefCursor       = QCursor(ArrowCursor);
  myHandCursor      = QCursor(PointingHandCursor);
  myPanCursor       = QCursor(SizeAllCursor);
  myZoomCursor      = QCursor(QPixmap(imageZoomCursor));
  myRotateCursor    = QCursor(QPixmap(imageRotateCursor));
  mySpinCursor      = QCursor(QPixmap(imageRotateCursor)); // temporarly !!!!!!
  myGlobalPanCursor = QCursor(CrossCursor);
  myCursorState     = false;
}


//----------------------------------------------------------------------------
// event filter - controls mouse and keyboard events during viewer operations
bool
SVTK_InteractorStyle
::eventFilter(QObject* object, QEvent* event)
{
  if (!myGUIWindow) return false;
  if ( (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyPress) && object != myGUIWindow)
  {
    qApp->removeEventFilter(this);
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  return QObject::eventFilter(object, event);
}


//----------------------------------------------------------------------------
// starts Zoom operation (e.g. through menu command)
void
SVTK_InteractorStyle
::startZoom()
{
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
  {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  setCursor(VTK_INTERACTOR_STYLE_CAMERA_ZOOM);
  ForcedState = VTK_INTERACTOR_STYLE_CAMERA_ZOOM;
  qApp->installEventFilter(this);
}


//----------------------------------------------------------------------------
// starts Pan operation (e.g. through menu command)
void
SVTK_InteractorStyle
::startPan()
{
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
  {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  setCursor(VTK_INTERACTOR_STYLE_CAMERA_PAN);
  ForcedState = VTK_INTERACTOR_STYLE_CAMERA_PAN;
  qApp->installEventFilter(this);
}

//----------------------------------------------------------------------------
// starts Rotate operation (e.g. through menu command)
void 
SVTK_InteractorStyle
::startRotate()
{
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
  {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  setCursor(VTK_INTERACTOR_STYLE_CAMERA_ROTATE);
  ForcedState = VTK_INTERACTOR_STYLE_CAMERA_ROTATE;
  qApp->installEventFilter(this);
}


//----------------------------------------------------------------------------
// starts Spin operation (e.g. through menu command)
void
SVTK_InteractorStyle
::startSpin()
{
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
  {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  setCursor(VTK_INTERACTOR_STYLE_CAMERA_SPIN);
  ForcedState = VTK_INTERACTOR_STYLE_CAMERA_SPIN;
  qApp->installEventFilter(this);
}



//----------------------------------------------------------------------------
// starts Fit Area operation (e.g. through menu command)
void
SVTK_InteractorStyle
::startFitArea()
{
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
  {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  setCursor(VTK_INTERACTOR_STYLE_CAMERA_FIT);
  ForcedState = VTK_INTERACTOR_STYLE_CAMERA_FIT;
  qApp->installEventFilter(this);
}


//----------------------------------------------------------------------------
// starts Global Panning operation (e.g. through menu command)
void
SVTK_InteractorStyle
::startGlobalPan()
{
  if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
  {
    onFinishOperation();
    startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
  }
  setCursor(VTK_INTERACTOR_STYLE_CAMERA_GLOBAL_PAN);
  ForcedState = VTK_INTERACTOR_STYLE_CAMERA_GLOBAL_PAN;

  // store current zoom scale
  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();
  myScale = cam->GetParallelScale();

  if (myViewWindow) myViewWindow->onFitAll();

  if (myGUIWindow) myGUIWindow->update();
  
  qApp->installEventFilter(this);
}


//----------------------------------------------------------------------------
// returns TRUE if needs redrawing
bool
SVTK_InteractorStyle
::needsRedrawing()
{
  return State == VTK_INTERACTOR_STYLE_CAMERA_ZOOM   ||
         State == VTK_INTERACTOR_STYLE_CAMERA_PAN    ||
         State == VTK_INTERACTOR_STYLE_CAMERA_ROTATE ||
         State == VTK_INTERACTOR_STYLE_CAMERA_SPIN   ||
         State == VTK_INTERACTOR_STYLE_CAMERA_NONE;
}


//----------------------------------------------------------------------------
// fits viewer contents to rect
void
SVTK_InteractorStyle
::fitRect(const int left, 
	  const int top, 
	  const int right, 
	  const int bottom)
{
  if (this->CurrentRenderer == NULL) return;
 
  // move camera
  int x = (left + right)/2;
  int y = (top + bottom)/2;
  int *aSize = this->CurrentRenderer->GetRenderWindow()->GetSize();
  int oldX = aSize[0]/2;
  int oldY = aSize[1]/2;
  TranslateView(oldX, oldY, x, y);

  // zoom camera
  double dxf = (double)(aSize[0]) / (double)(abs(right - left));
  double dyf = (double)(aSize[1]) / (double)(abs(bottom - top));
  double zoomFactor = (dxf + dyf)/2 ;

  vtkCamera *aCam = this->CurrentRenderer->GetActiveCamera();
  if(aCam->GetParallelProjection())
    aCam->SetParallelScale(aCam->GetParallelScale()/zoomFactor);
  else{
    aCam->Dolly(zoomFactor);
    ::ResetCameraClippingRange(this->CurrentRenderer);
  }
  
  myGUIWindow->update();
}


//----------------------------------------------------------------------------
// starts viewer operation (!internal usage!)
void
SVTK_InteractorStyle
::startOperation(int operation)
{
  switch(operation)
  { 
  case VTK_INTERACTOR_STYLE_CAMERA_GLOBAL_PAN:
  case VTK_INTERACTOR_STYLE_CAMERA_ZOOM:
  case VTK_INTERACTOR_STYLE_CAMERA_PAN:
  case VTK_INTERACTOR_STYLE_CAMERA_ROTATE:
  case VTK_INTERACTOR_STYLE_CAMERA_SPIN:
  case VTK_INTERACTOR_STYLE_CAMERA_FIT:
  case VTK_INTERACTOR_STYLE_CAMERA_SELECT:
    if (State != VTK_INTERACTOR_STYLE_CAMERA_NONE)
      startOperation(VTK_INTERACTOR_STYLE_CAMERA_NONE);
    State = operation;
    if (State != VTK_INTERACTOR_STYLE_CAMERA_SELECT)
      setCursor(operation);
    onStartOperation();
    break;
  case VTK_INTERACTOR_STYLE_CAMERA_NONE:
  default:
    setCursor(VTK_INTERACTOR_STYLE_CAMERA_NONE);
    State = ForcedState = VTK_INTERACTOR_STYLE_CAMERA_NONE;
    break;
  }
}


//----------------------------------------------------------------------------
// sets proper cursor for window when viewer operation is activated
void
SVTK_InteractorStyle
::setCursor(const int operation)
{
  if (!myGUIWindow) return;
  switch (operation)
  {
    case VTK_INTERACTOR_STYLE_CAMERA_ZOOM:
      myGUIWindow->setCursor(myZoomCursor); 
      myCursorState = true;
      break;
    case VTK_INTERACTOR_STYLE_CAMERA_PAN:
      myGUIWindow->setCursor(myPanCursor); 
      myCursorState = true;
      break;
    case VTK_INTERACTOR_STYLE_CAMERA_ROTATE:
      myGUIWindow->setCursor(myRotateCursor); 
      myCursorState = true;
      break;
    case VTK_INTERACTOR_STYLE_CAMERA_SPIN:
      myGUIWindow->setCursor(mySpinCursor); 
      myCursorState = true;
      break;
    case VTK_INTERACTOR_STYLE_CAMERA_GLOBAL_PAN:
      myGUIWindow->setCursor(myGlobalPanCursor); 
      myCursorState = true;
      break;
    case VTK_INTERACTOR_STYLE_CAMERA_FIT:
    case VTK_INTERACTOR_STYLE_CAMERA_SELECT:
      myGUIWindow->setCursor(myHandCursor); 
      myCursorState = true;
      break;
    case VTK_INTERACTOR_STYLE_CAMERA_NONE:
    default:
      myGUIWindow->setCursor(myDefCursor); 
      myCursorState = false;
      break;
  }
}


//----------------------------------------------------------------------------
// called when viewer operation started (!put necessary initialization here!)
void
SVTK_InteractorStyle
::onStartOperation()
{
  if (!myGUIWindow) return;
  // VSV: LOD actor activisation
  //  this->Interactor->GetRenderWindow()->SetDesiredUpdateRate(this->Interactor->GetDesiredUpdateRate());
  switch (State) {
    case VTK_INTERACTOR_STYLE_CAMERA_SELECT:
    case VTK_INTERACTOR_STYLE_CAMERA_FIT:
    {
      QPainter p(myGUIWindow);
      p.setPen(Qt::lightGray);
      p.setRasterOp(Qt::XorROP);
      p.drawRect(QRect(myPoint, myOtherPoint));
      break;
    }
    case VTK_INTERACTOR_STYLE_CAMERA_ZOOM:
    case VTK_INTERACTOR_STYLE_CAMERA_PAN:
    case VTK_INTERACTOR_STYLE_CAMERA_ROTATE:
    case VTK_INTERACTOR_STYLE_CAMERA_GLOBAL_PAN:
    case VTK_INTERACTOR_STYLE_CAMERA_SPIN:
      break;
  }
}


//----------------------------------------------------------------------------
// called when viewer operation finished (!put necessary post-processing here!)
void
SVTK_InteractorStyle
::onFinishOperation() 
{
  if (!myGUIWindow) 
    return;

  // VSV: LOD actor activisation
  //  rwi->GetRenderWindow()->SetDesiredUpdateRate(rwi->GetStillUpdateRate());

  Selection_Mode aSelectionMode = myViewWindow->SelectionMode();
  bool aSelActiveCompOnly = false;

  QString aComponentDataType;
  if(SUIT_Session* aSession = SUIT_Session::session())
    if(SUIT_Application* aSUITApp = aSession->activeApplication())
      if(CAM_Application* aCAMApp = dynamic_cast<CAM_Application*>(aSUITApp))
	if(CAM_Module* aModule = aCAMApp->activeModule())
	  aComponentDataType = aModule->name();

  switch (State) {
    case VTK_INTERACTOR_STYLE_CAMERA_SELECT:
    case VTK_INTERACTOR_STYLE_CAMERA_FIT:
    {
      QPainter p(myGUIWindow);
      p.setPen(Qt::lightGray);
      p.setRasterOp(Qt::XorROP);
      QRect rect(myPoint, myOtherPoint);
      p.drawRect(rect);
      rect = rect.normalize();
      if (State == VTK_INTERACTOR_STYLE_CAMERA_FIT) {
        // making fit rect opeation 
        int w, h;
        myInteractor->GetSize(w, h);
        int x1, y1, x2, y2;
        x1 = rect.left(); 
        y1 = h - rect.top() - 1;
        x2 = rect.right(); 
        y2 = h - rect.bottom() - 1;
        fitRect(x1, y1, x2, y2);
      }
      else {
        if (myPoint == myOtherPoint) {
	  // process point selection
          int w, h, x, y;
          myInteractor->GetSize(w, h);
          x = myPoint.x(); 
          y = h - myPoint.y() - 1;

          this->FindPokedRenderer(x, y);
	  myInteractor->StartPickCallback();

	  vtkPicker* aPicker = vtkPicker::SafeDownCast(myInteractor->GetPicker());
          aPicker->Pick(x, y, 0.0, this->CurrentRenderer);
    
	  SALOME_Actor* aSActor = SALOME_Actor::SafeDownCast(aPicker->GetActor());

          if (vtkCellPicker* picker = vtkCellPicker::SafeDownCast(aPicker)) {
	    int aVtkId = picker->GetCellId();
	    if ( aVtkId >= 0 && aSActor && aSActor->hasIO() && IsValid( aSActor, aVtkId ) ) {
	      int anObjId = aSActor->GetElemObjId(aVtkId);
	      if(anObjId >= 0){
		Handle(SALOME_InteractiveObject) anIO = aSActor->getIO();
		if(aSelectionMode != EdgeOfCellSelection) {
		  if(CheckDimensionId(aSelectionMode,aSActor,anObjId)){
		    if (GetSelector()->IsSelected(anIO)) {
		      // This IO is already in the selection
		      GetSelector()->AddOrRemoveIndex(anIO,anObjId,myShiftState);
		    } else {
		      if (!myShiftState) {
			this->HighlightProp( NULL );
			GetSelector()->ClearIObjects();
		      }
		      GetSelector()->AddOrRemoveIndex(anIO,anObjId,myShiftState);
		      GetSelector()->AddIObject(aSActor);
		    }
		  }
		}else{
		  if (!myShiftState) {
		    this->HighlightProp( NULL );
		    GetSelector()->ClearIObjects();
		  }
		  int anEdgeId = GetEdgeId(picker,aSActor,anObjId);
		  if (anEdgeId >= 0) {
		    GetSelector()->AddOrRemoveIndex(anIO,anObjId,false);
		    GetSelector()->AddOrRemoveIndex(anIO,-anEdgeId-1,true);
		    GetSelector()->AddIObject(aSActor);
		  } 
		}
	      }
	    } else {
	      this->HighlightProp( NULL );
	      GetSelector()->ClearIObjects();
	    }
          } else if ( vtkPointPicker* picker = vtkPointPicker::SafeDownCast(aPicker) ) {
	    int aVtkId = picker->GetPointId();
	    if ( aVtkId >= 0 && IsValid( aSActor, aVtkId, true ) ) {
	      if ( aSActor && aSActor->hasIO() ) {
		int anObjId = aSActor->GetNodeObjId(aVtkId);
		if(anObjId >= 0){
		  Handle(SALOME_InteractiveObject) anIO = aSActor->getIO();
		  if(GetSelector()->IsSelected(anIO)) {
		    // This IO is already in the selection
		    GetSelector()->AddOrRemoveIndex(anIO,anObjId,myShiftState);
		  } else {
		    if(!myShiftState) {
		      this->HighlightProp( NULL );
		      GetSelector()->ClearIObjects();
		    }
		    GetSelector()->AddOrRemoveIndex(anIO,anObjId,myShiftState);
		    GetSelector()->AddIObject(aSActor);
		  }
		}
	      }
	    } else {
	      this->HighlightProp( NULL );
	      GetSelector()->ClearIObjects();
	    } 
	  } else {
	    if ( aSActor && aSActor->hasIO() ) {
	      this->PropPicked++;
	      Handle(SALOME_InteractiveObject) anIO = aSActor->getIO();
	      if(GetSelector()->IsSelected(anIO)) {
		// This IO is already in the selection
		if(myShiftState) {
		  GetSelector()->RemoveIObject(aSActor);
		}
	      }
	      else {
		if(!myShiftState) {
		  this->HighlightProp( NULL );
		  GetSelector()->ClearIObjects();
		}
		GetSelector()->AddIObject(aSActor);
	      }
	    }else{
	      // No selection clear all
	      this->PropPicked = 0;
	      this->HighlightProp( NULL );
	      GetSelector()->ClearIObjects();
	    }
	  }
	  myInteractor->EndPickCallback();
        } else {
          //processing rectangle selection
	  if(aSelActiveCompOnly && aComponentDataType.isEmpty()) return;
	  myInteractor->StartPickCallback();

	  if (!myShiftState) {
	    this->PropPicked = 0;
	    this->HighlightProp( NULL );
	    GetSelector()->ClearIObjects();
	  }

	  // Compute bounds
	  //	  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();
	  QRect rect(myPoint, myOtherPoint);
	  rect = rect.normalize();
	  int w, h;
	  myInteractor->GetSize(w, h);
	  int x1, y1, x2, y2;
	  x1 = rect.left(); 
	  y1 = h - rect.top() - 1;
	  x2 = rect.right(); 
	  y2 = h - rect.bottom() - 1;

	  switch (aSelectionMode) {
	  case NodeSelection: {
	    if ( vtkPointPicker* aPointPicker = vtkPointPicker::SafeDownCast(myInteractor->GetPicker()) ) {
	      vtkActorCollection* aListActors = this->CurrentRenderer->GetActors();
	      aListActors->InitTraversal();
	      while (vtkActor* aActor = aListActors->GetNextActor()) {
		if (!aActor->GetVisibility()) 
		  continue;
		if(SALOME_Actor* aSActor = SALOME_Actor::SafeDownCast(aActor)) {
		  if (aSActor->hasIO()) {
		    Handle(SALOME_InteractiveObject) anIO = aSActor->getIO();
		    if (anIO.IsNull()) 
		      continue;
		    if (aSelActiveCompOnly && aComponentDataType != anIO->getComponentDataType())
		      continue;
		    if (vtkDataSet* aDataSet = aSActor->GetInput()) {
		      TColStd_MapOfInteger anIndices;
		      for(int i = 0; i < aDataSet->GetNumberOfPoints(); i++) {
			float aPoint[3];
			aDataSet->GetPoint(i,aPoint);
			if (IsInRect(aPoint,x1,y1,x2,y2)){
			  float aDisp[3];
			  ComputeWorldToDisplay(aPoint[0],aPoint[1],aPoint[2],aDisp);
			  if(aPointPicker->Pick(aDisp[0],aDisp[1],0.0,CurrentRenderer)){
			    if(vtkActorCollection *anActorCollection = aPointPicker->GetActors()){
			      if(anActorCollection->IsItemPresent(aSActor)){
				float aPickedPoint[3];
				aPointPicker->GetMapperPosition(aPickedPoint);
				vtkIdType aVtkId = aDataSet->FindPoint(aPickedPoint);
				if ( aVtkId >= 0 && IsValid( aSActor, aVtkId, true ) ){
				  int anObjId = aSActor->GetNodeObjId(aVtkId);
				  anIndices.Add(anObjId);
				}
			      }
			    }
			  }
			}
		      }
		      if (!anIndices.IsEmpty()) {
			GetSelector()->AddOrRemoveIndex(anIO,anIndices,true); // ENK false to true
			GetSelector()->AddIObject(aSActor);
			anIndices.Clear();
		      }else{
			GetSelector()->RemoveIObject(aSActor);
		      }
		    }
		  }
		}
	      }
	    }
	    break;
	  }
	  case CellSelection:
	  case EdgeOfCellSelection:
	  case EdgeSelection:
	  case FaceSelection:
	  case VolumeSelection: 
	    {
	      vtkSmartPointer<VTKViewer_CellRectPicker> picker = VTKViewer_CellRectPicker::New();
	      picker->SetTolerance(0.001);
	      picker->Pick(x1, y1, 0.0, x2, y2, 0.0, this->CurrentRenderer);
	      
	      vtkActorCollection* aListActors = picker->GetActors();
	      aListActors->InitTraversal();
	      while(vtkActor* aActor = aListActors->GetNextActor()) {
		if (SALOME_Actor* aSActor = SALOME_Actor::SafeDownCast(aActor)) {
		  if (aSActor->hasIO()) {
		    Handle(SALOME_InteractiveObject) anIO = aSActor->getIO();
		    if (aSelActiveCompOnly && aComponentDataType != anIO->getComponentDataType())
		      continue;
		    VTKViewer_CellDataSet cellList = picker->GetCellData(aActor);
		    if ( !cellList.empty() ) {
		      TColStd_MapOfInteger anIndexes;
		      VTKViewer_CellDataSet::iterator it;
		      for ( it = cellList.begin(); it != cellList.end(); ++it ) {
			int aCellId = (*it).cellId;
			
			if ( !IsValid( aSActor, aCellId ) )
			  continue;
			
			int anObjId = aSActor->GetElemObjId(aCellId);
			if (anObjId != -1){
			  if ( CheckDimensionId(aSelectionMode,aSActor,anObjId) ) {
			    anIndexes.Add(anObjId);
			  }
			}
		      }
		      GetSelector()->AddOrRemoveIndex(anIO,anIndexes,true);
		      GetSelector()->AddIObject(aSActor);
		    }
		  }
		}
	      }
	    }
	    break;	    
	  case ActorSelection: // objects selection
	    {
	      vtkSmartPointer<VTKViewer_RectPicker> picker = VTKViewer_RectPicker::New();
	      picker->SetTolerance(0.001);
	      picker->Pick(x1, y1, 0.0, x2, y2, 0.0, this->CurrentRenderer);

	      vtkActorCollection* aListActors = picker->GetActors();
	      aListActors->InitTraversal();
	      while(vtkActor* aActor = aListActors->GetNextActor()) {
		if (SALOME_Actor* aSActor = SALOME_Actor::SafeDownCast(aActor)) {
		  if (aSActor->hasIO()) {
		    Handle(SALOME_InteractiveObject) anIO = aSActor->getIO();
		    GetSelector()->AddIObject(aSActor);
		    this->PropPicked++;
		  }
		}
	      }
	    } // end case 4
	  } //end switch
	  myInteractor->EndPickCallback();
	}
	myViewWindow->onSelectionChanged();
      } 
    } 
    break;
  case VTK_INTERACTOR_STYLE_CAMERA_ZOOM:
  case VTK_INTERACTOR_STYLE_CAMERA_PAN:
  case VTK_INTERACTOR_STYLE_CAMERA_ROTATE:
  case VTK_INTERACTOR_STYLE_CAMERA_SPIN:
    break;
  case VTK_INTERACTOR_STYLE_CAMERA_GLOBAL_PAN: 
    {
      int w, h, x, y;
      myInteractor->GetSize(w, h);
      x = myPoint.x(); 
      y = h - myPoint.y() - 1;
      Place(x, y);
    }
    break;
  }
  if (myGUIWindow) myGUIWindow->update();

}


// called during viewer operation when user moves mouse (!put necessary processing here!)
void
SVTK_InteractorStyle
::onOperation(QPoint mousePos) 
{
  if (!myGUIWindow) return;
  int w, h;
  GetInteractor()->GetSize(w, h);
  switch (State) {
  case VTK_INTERACTOR_STYLE_CAMERA_PAN: 
    {
      // processing panning
      //this->FindPokedCamera(mousePos.x(), mousePos.y());
      this->PanXY(mousePos.x(), myPoint.y(), myPoint.x(), mousePos.y());
      myPoint = mousePos;
      break;
    }
  case VTK_INTERACTOR_STYLE_CAMERA_ZOOM: 
    {    
      // processing zooming
      //this->FindPokedCamera(mousePos.x(), mousePos.y());
      this->DollyXY(mousePos.x() - myPoint.x(), mousePos.y() - myPoint.y());
      myPoint = mousePos;
      break;
    }
  case VTK_INTERACTOR_STYLE_CAMERA_ROTATE: 
    {
      // processing rotation
      //this->FindPokedCamera(mousePos.x(), mousePos.y());
      this->RotateXY(mousePos.x() - myPoint.x(), myPoint.y() - mousePos.y());
      myPoint = mousePos;
      break;
    }
  case VTK_INTERACTOR_STYLE_CAMERA_SPIN: 
    {
      // processing spinning
      //this->FindPokedCamera(mousePos.x(), mousePos.y());
      this->SpinXY(mousePos.x(), mousePos.y(), myPoint.x(), myPoint.y());
      myPoint = mousePos;
      break;
    }
  case VTK_INTERACTOR_STYLE_CAMERA_GLOBAL_PAN: 
    {    
      break;
    }
  case VTK_INTERACTOR_STYLE_CAMERA_SELECT:
    {
      if (!myCursorState)
        setCursor(VTK_INTERACTOR_STYLE_CAMERA_SELECT);
    }
  case VTK_INTERACTOR_STYLE_CAMERA_FIT:
    {
      QPainter p(myGUIWindow);
      p.setPen(Qt::lightGray);
      p.setRasterOp(Qt::XorROP);
      p.drawRect(QRect(myPoint, myOtherPoint));
      myOtherPoint = mousePos;
      p.drawRect(QRect(myPoint, myOtherPoint));
      break;
    }
  }
  this->LastPos[0] = mousePos.x();
  this->LastPos[1] = h - mousePos.y() - 1;
}

// called when selection mode changed (!put necessary initialization here!)
void
SVTK_InteractorStyle
::OnSelectionModeChanged()
{
  
  myPreSelectionActor->SetVisibility(false);
  myElemId = myEdgeId = myNodeId = -1;
  mySelectedActor = NULL;
}

// called when user moves mouse inside viewer window and there is no active viewer operation 
// (!put necessary processing here!)
void
SVTK_InteractorStyle
::onCursorMove(QPoint mousePos) 
{
  // processing highlighting
  Selection_Mode aSelectionMode = myViewWindow->SelectionMode();

  int w, h, x, y;
  myInteractor->GetSize(w, h);
  x = mousePos.x(); y = h - mousePos.y() - 1;

  this->FindPokedRenderer(x,y);
  myInteractor->StartPickCallback();
  myPreSelectionActor->SetVisibility(false);

  vtkPicker* aPicker = vtkPicker::SafeDownCast(myInteractor->GetPicker());
  aPicker->Pick(x, y, 0.0, this->CurrentRenderer);

  SALOME_Actor* aSActor = SALOME_Actor::SafeDownCast(aPicker->GetActor());

  if (aSActor && myPreSelectionActor){
    float aPos[3];
    aSActor->GetPosition(aPos);
    myPreSelectionActor->SetPosition(aPos);
  }

  if (vtkCellPicker* picker = vtkCellPicker::SafeDownCast(aPicker)) {
    int aVtkId = picker->GetCellId();
    if ( aVtkId >= 0 ) {
      int anObjId = aSActor->GetElemObjId(aVtkId);
      if ( aSActor && aSActor->hasIO() && IsValid( aSActor, aVtkId ) ) {
	bool anIsSameObjId = (mySelectedActor == aSActor && myElemId == anObjId);
	bool aResult = anIsSameObjId;
	if(!anIsSameObjId) {
	  if(aSelectionMode != EdgeOfCellSelection) {
	    aResult = CheckDimensionId(aSelectionMode,aSActor,anObjId);
	    if(aResult){
	      mySelectedActor = aSActor;
	      myElemId = anObjId;
	      myInteractor->setCellData(anObjId,aSActor,myPreSelectionActor);
	    }
	  }
	}
	if(aSelectionMode == EdgeOfCellSelection){
	  int anEdgeId = GetEdgeId(picker,aSActor,anObjId);
	  bool anIsSameEdgeId = (myEdgeId != anEdgeId) && anIsSameObjId;
	  aResult = anIsSameEdgeId;
	  if(!anIsSameEdgeId) {
	    aResult = (anEdgeId >= 0);
	    if (aResult) {
	      mySelectedActor = aSActor;
	      myEdgeId = anEdgeId;
	      myElemId = anObjId;
	      myInteractor->setEdgeData(anObjId,aSActor,-anEdgeId-1,myPreSelectionActor);
	    } 
	  }
	}
	if(aResult) {
	  myPreSelectionActor->GetProperty()->SetRepresentationToSurface();
	  myPreSelectionActor->SetVisibility(true);
	}
      }
    }
  }
  else if (vtkPointPicker* picker = vtkPointPicker::SafeDownCast(aPicker)) {
    int aVtkId = picker->GetPointId();
    if ( aVtkId >= 0 && IsValid( aSActor, aVtkId, true ) ) {
      if ( aSActor && aSActor->hasIO() ) {
	int anObjId = aSActor->GetNodeObjId(aVtkId);
	bool anIsSameObjId = (mySelectedActor == aSActor && myNodeId == anObjId);
	if(!anIsSameObjId) {
	  mySelectedActor = aSActor;
	  myNodeId = anObjId;
	  myInteractor->setPointData(anObjId,aSActor,myPreSelectionActor);
	}
	myPreSelectionActor->GetProperty()->SetRepresentationToSurface();
	myPreSelectionActor->SetVisibility(true);
      }
    }
  }
  else if ( vtkPicker::SafeDownCast(aPicker) ) {
    if ( aSActor ) {
      if ( myPreViewActor != aSActor ) {
	if ( myPreViewActor != NULL ) {
	  myPreViewActor->SetPreSelected( false );
	}
	myPreViewActor = aSActor;
	      
	if ( aSActor->hasIO() ) {
	  Handle( SALOME_InteractiveObject) IO = aSActor->getIO();
	  if ( !GetSelector()->IsSelected(IO) ) {
            // Find All actors with same IO
	    vtkActorCollection* theActors = this->CurrentRenderer->GetActors();
	    theActors->InitTraversal();
	    while( vtkActor *ac = theActors->GetNextActor() ) {
	      if ( SALOME_Actor* anActor = SALOME_Actor::SafeDownCast( ac ) ) {
		if ( anActor->hasIO() ) {
		  Handle(SALOME_InteractiveObject) IOS = anActor->getIO();
		  if(IO->isSame(IOS)) {
		    anActor->SetPreSelected( true );
		  }
		}
	      }
	    }
	  }
	}
      }
    } else {
      myPreViewActor = NULL;
      vtkActorCollection* theActors = this->CurrentRenderer->GetActors();
      theActors->InitTraversal();
      while( vtkActor *ac = theActors->GetNextActor() ) {
        if ( SALOME_Actor* anActor = SALOME_Actor::SafeDownCast( ac ) ) {
          anActor->SetPreSelected( false );
        }
      }
    }
  }
  myInteractor->EndPickCallback();
  //myInteractor->Render();
  myGUIWindow->update();
  
  this->LastPos[0] = x;
  this->LastPos[1] = y;
}

// called on finsh GlobalPan operation 
void
SVTK_InteractorStyle
::Place(const int theX, const int theY) 
{
  if (this->CurrentRenderer == NULL) {
    return;
  }

  //translate view
  int *aSize = this->CurrentRenderer->GetRenderWindow()->GetSize();
  int centerX = aSize[0]/2;
  int centerY = aSize[1]/2;

  TranslateView(centerX, centerY, theX, theY);

  // restore zoom scale
  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();
  cam->SetParallelScale(myScale);
  ::ResetCameraClippingRange(this->CurrentRenderer);

  if (myGUIWindow) myGUIWindow->update();

}



// Translates view from Point to Point
void
SVTK_InteractorStyle
::TranslateView(int toX, int toY, int fromX, int fromY)
{
  vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();
  double viewFocus[4], focalDepth, viewPoint[3];
  float newPickPoint[4], oldPickPoint[4], motionVector[3];
  cam->GetFocalPoint(viewFocus);

  this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1],
			      viewFocus[2], viewFocus);
  focalDepth = viewFocus[2];

  this->ComputeDisplayToWorld(double(toX), double(toY),
			      focalDepth, newPickPoint);
  this->ComputeDisplayToWorld(double(fromX),double(fromY),
			      focalDepth, oldPickPoint);
  
  // camera motion is reversed
  motionVector[0] = oldPickPoint[0] - newPickPoint[0];
  motionVector[1] = oldPickPoint[1] - newPickPoint[1];
  motionVector[2] = oldPickPoint[2] - newPickPoint[2];
  
  cam->GetFocalPoint(viewFocus);
  cam->GetPosition(viewPoint);
  cam->SetFocalPoint(motionVector[0] + viewFocus[0],
		     motionVector[1] + viewFocus[1],
		     motionVector[2] + viewFocus[2]);
  cam->SetPosition(motionVector[0] + viewPoint[0],
		   motionVector[1] + viewPoint[1],
		   motionVector[2] + viewPoint[2]);
}


/// Checks: is the given Actor within display coordinates?
bool
SVTK_InteractorStyle
::IsInRect(vtkActor* theActor, 
	   const int left, const int top, 
	   const int right, const int bottom)
{
  float* aBounds = theActor->GetBounds();
  float aMin[3], aMax[3];
  ComputeWorldToDisplay(aBounds[0], aBounds[2], aBounds[4], aMin);
  ComputeWorldToDisplay(aBounds[1], aBounds[3], aBounds[5], aMax);
  if (aMin[0] > aMax[0]) {
    float aBuf = aMin[0];
    aMin[0] = aMax[0];
    aMax[0] = aBuf;
  }
  if (aMin[1] > aMax[1]) {
    float aBuf = aMin[1];
    aMin[1] = aMax[1];
    aMax[1] = aBuf;    
  }

  return ((aMin[0]>left) && (aMax[0]<right) && (aMin[1]>bottom) && (aMax[1]<top));
}


/// Checks: is the given Cell within display coordinates?
bool
SVTK_InteractorStyle
::IsInRect(vtkCell* theCell, 
	   const int left, const int top, 
	   const int right, const int bottom)
{
  float* aBounds = theCell->GetBounds();
  float aMin[3], aMax[3];
  ComputeWorldToDisplay(aBounds[0], aBounds[2], aBounds[4], aMin);
  ComputeWorldToDisplay(aBounds[1], aBounds[3], aBounds[5], aMax);
  if (aMin[0] > aMax[0]) {
    float aBuf = aMin[0];
    aMin[0] = aMax[0];
    aMax[0] = aBuf;
  }
  if (aMin[1] > aMax[1]) {
    float aBuf = aMin[1];
    aMin[1] = aMax[1];
    aMax[1] = aBuf;    
  }

  return ((aMin[0]>left) && (aMax[0]<right) && (aMin[1]>bottom) && (aMax[1]<top));
}


bool
SVTK_InteractorStyle
::IsInRect(float* thePoint, 
	   const int left, const int top, 
	   const int right, const int bottom)
{
  float aPnt[3];
  ComputeWorldToDisplay(thePoint[0], thePoint[1], thePoint[2], aPnt);

  return ((aPnt[0]>left) && (aPnt[0]<right) && (aPnt[1]>bottom) && (aPnt[1]<top));
}

void
SVTK_InteractorStyle
::SetFilter( const Handle(VTKViewer_Filter)& theFilter )
{
  myFilters[ theFilter->GetId() ] = theFilter;
}

bool
SVTK_InteractorStyle
::IsFilterPresent( const int theId )
{
  return myFilters.find( theId ) != myFilters.end();
}

void  
SVTK_InteractorStyle
::RemoveFilter( const int theId )
{
  if ( IsFilterPresent( theId ) )
    myFilters.erase( theId );
}


bool
SVTK_InteractorStyle
::IsValid( SALOME_Actor* theActor,
	   const int     theId,
	   const bool    theIsNode )
{
  std::map<int, Handle(VTKViewer_Filter)>::const_iterator anIter;
  for ( anIter = myFilters.begin(); anIter != myFilters.end(); ++anIter )
  {
    const Handle(VTKViewer_Filter)& aFilter = anIter->second;
    if ( theIsNode == aFilter->IsNodeFilter() &&
         !aFilter->IsValid( theActor, theId ) )
      return false;
  }
  return true;
}

Handle(VTKViewer_Filter) 
SVTK_InteractorStyle
::GetFilter( const int theId )
{
  return IsFilterPresent( theId ) ? myFilters[ theId ] : Handle(VTKViewer_Filter)();
}

void
SVTK_InteractorStyle
::IncrementalPan( const int incrX, const int incrY )
{
  this->PanXY( incrX, incrY, 0, 0 );
}

void
SVTK_InteractorStyle
::IncrementalZoom( const int incr )
{
  this->DollyXY( incr, incr );
}

void
SVTK_InteractorStyle
::IncrementalRotate( const int incrX, const int incrY )
{
  this->RotateXY( incrX, -incrY );
}
