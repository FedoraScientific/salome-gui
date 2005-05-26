#ifndef VTKVIEWER_RENDERWINDOWINTERACTOR_H
#define VTKVIEWER_RENDERWINDOWINTERACTOR_H

#include "VTKViewer.h"
#include "VTKViewer_Actor.h"

#include <qtimer.h>
#include <qobject.h>

// Open CASCADE Includes
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>

class vtkPicker;
class vtkCellPicker;
class vtkPointPicker;
class vtkActorCollection;

class VTKViewer_Actor;
class VTKViewer_ViewWindow;
class VTKViewer_RenderWindow;
class VTKViewer_InteractorStyle;

#include "VTKViewer_Algorithm.h"

#include <vtkActor.h>
#include <vtkVersion.h>
#include <vtkRenderWindowInteractor.h>

class VTKVIEWER_EXPORT VTKViewer_RenderWindowInteractor : public QObject, public vtkRenderWindowInteractor
{
  Q_OBJECT

public:
  static VTKViewer_RenderWindowInteractor *New();

  vtkTypeMacro(VTKViewer_RenderWindowInteractor,vtkRenderWindowInteractor);

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initializes the event handlers without an XtAppContext.  This is
  // good for when you don`t have a user interface, but you still
  // want to have mouse interaction.
  virtual void Initialize();

  virtual void               SetInteractorStyle(vtkInteractorObserver *);
  VTKViewer_InteractorStyle* GetInteractorStyle() const
  {
    return myInteractorStyle;
  }

  // Description:
  // This will start up the X event loop and never return. If you
  // call this method it will loop processing X events until the
  // application is exited.
  virtual void Start();
  
  // Description:
  // Enable/Disable interactions.  By default interactors are enabled when
  // initialized.  Initialize() must be called prior to enabling/disabling
  // interaction. These methods are used when a window/widget is being
  // shared by multiple renderers and interactors.  This allows a "modal"
  // display where one interactor is active when its data is to be displayed
  // and all other interactors associated with the widget are disabled
  // when their data is not displayed.
  virtual void Enable();
  virtual void Disable();

  // Description:
  // Event loop notification member for Window size change
  virtual void UpdateSize(int x,int y);

  // Description:
  // Timer methods must be overridden by platform dependent subclasses.
  // flag is passed to indicate if this is first timer set or an update
  // as Win32 uses repeating timers, whereas X uses One shot more timer
  // if flag==VTKXI_TIMER_FIRST Win32 and X should createtimer
  // otherwise Win32 should exit and X should perform AddTimeOut()
  virtual int CreateTimer(int ) ; 
  virtual int DestroyTimer() ; 
  
  // Description:
  // This function is called on 'q','e' keypress if exitmethod is not
  // specified and should be overidden by platform dependent subclasses
  // to provide a termination procedure if one is required.
  virtual void TerminateApp(void) { /* empty */ }
  
  // Description:
  // These methods correspond to the the Exit, User and Pick
  // callbacks. They allow for the Style to invoke them.
  //virtual void ExitCallback();
  //virtual void UserCallback();
  //virtual void StartPickCallback();
  //virtual void EndPickCallback();
  
  /* Selection Management */
  bool highlightCell(const TColStd_IndexedMapOfInteger& MapIndex,
		     VTKViewer_Actor* theMapActor,
		     bool hilight,
		     bool update = true );
  bool highlightEdge(const TColStd_IndexedMapOfInteger& MapIndex,
		     VTKViewer_Actor* theMapActor,
		     bool hilight,
		     bool update = true );
  bool highlightPoint(const TColStd_IndexedMapOfInteger& MapIndex,
		      VTKViewer_Actor* theMapActor,
		      bool hilight,
		      bool update = true );

  void unHighlightSubSelection();
  bool unHighlightAll();

  //void SetSelectionMode(Selection_Mode mode);
  void SetSelectionProp(const double& theRed = 1, const double& theGreen = 1,
			const double& theBlue = 0, const int& theWidth = 5);
  void SetSelectionTolerance(const double& theTolNodes = 0.025, const double& theTolCell = 0.001);

  // Displaymode management
  int GetDisplayMode();
  void SetDisplayMode(int);

  // Change all actors to wireframe or surface
  void ChangeRepresentationToWireframe();
  void ChangeRepresentationToSurface();

  // Change to wireframe or surface a list of vtkactor
  void ChangeRepresentationToWireframe(vtkActorCollection* ListofActors);
  void ChangeRepresentationToSurface(vtkActorCollection* ListofActors);

  // Erase Display functions
  void EraseAll();
  void DisplayAll();
  void RemoveAll( const bool immediatly );

  void Display( VTKViewer_Actor* SActor, bool immediatly = true );
  void Erase( VTKViewer_Actor* SActor, bool immediatly = true );
  void Remove( VTKViewer_Actor* SActor, bool updateViewer = true );

  void Update();

  vtkRenderer* GetRenderer();

  void setViewWindow( VTKViewer_ViewWindow* theViewWnd );

  void setCellData(const int& theIndex,
		   VTKViewer_Actor* theMapActor,
		   VTKViewer_Actor* theActor) {}
  void setEdgeData(const int& theCellIndex,
		   VTKViewer_Actor* theMapActor,
		   const int& theEdgeIndex,
		   VTKViewer_Actor* theActor ) {} //NB
  void setPointData(const int& theIndex,
		    VTKViewer_Actor* theMapActor,
		    VTKViewer_Actor* theActor) {}

  typedef void (*TUpdateActor)(const TColStd_IndexedMapOfInteger& theMapIndex,
			       VTKViewer_Actor* theMapActor,
			       VTKViewer_Actor* theActor);
 protected:

  VTKViewer_RenderWindowInteractor();
  ~VTKViewer_RenderWindowInteractor();

  VTKViewer_InteractorStyle* myInteractorStyle;

  bool highlight(const TColStd_IndexedMapOfInteger& theMapIndex,
		 VTKViewer_Actor* theMapActor, VTKViewer_Actor* theActor,
		 TUpdateActor theFun, bool hilight, bool update);
  void setActorData(const TColStd_IndexedMapOfInteger& theMapIndex,
		    VTKViewer_Actor* theMapActor,
		    VTKViewer_Actor *theActor,
		    TUpdateActor theFun);

  // Timer used during various mouse events to figure 
  // out mouse movements. 
  QTimer *mTimer ;

  int myDisplayMode;

  //NRI: Selection mode
  VTKViewer_Actor* myPointActor;
  VTKViewer_Actor* myEdgeActor;
  VTKViewer_Actor* myCellActor;
  void MoveInternalActors();

  vtkPicker* myBasicPicker;
  vtkCellPicker* myCellPicker;
  vtkPointPicker* myPointPicker;
  
  // User for switching to stereo mode.
  int PositionBeforeStereo[2];

 public slots:
  void MouseMove(QMouseEvent *event) ;
  void LeftButtonPressed(const QMouseEvent *event) ;
  void LeftButtonReleased(const QMouseEvent *event) ;
  void MiddleButtonPressed(const QMouseEvent *event) ;
  void MiddleButtonReleased(const QMouseEvent *event) ;
  void RightButtonPressed(const QMouseEvent *event) ;
  void RightButtonReleased(const QMouseEvent *event) ;
  void ButtonPressed(const QMouseEvent *event) ;
  void ButtonReleased(const QMouseEvent *event) ;
  void KeyPressed(QKeyEvent *event) ;

  private slots:
    // Not all of these slots are needed in VTK_MAJOR_VERSION=3,
    // but moc does not understand "#if VTK_MAJOR_VERSION". Hence, 
    // we have to include all of these for the time being. Once,
    // this bug in MOC is fixed, we can separate these. 
    void TimerFunc() ;

signals:
  void RenderWindowModified() ;
  void contextMenuRequested( QContextMenuEvent *e );

private:
  friend class VTKViewer_ViewWindow;

  VTKViewer_ViewWindow* myViewWnd;
  double       myTolNodes;
  double       myTolItems;
};

#endif
