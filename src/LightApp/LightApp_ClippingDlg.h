#ifndef LIGHTAPP_CLIPPINGDLG_H
#define LIGHTAPP_CLIPPINGDLG_H

#include "LightApp.h"

#include <qdialog.h>

#include <AIS_Plane.hxx>

class QGroupBox;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;
class QtxDblSpinBox;

class OCCViewer_ViewWindow;


//=================================================================================
// class    : LightApp_ClippingDlg
// purpose  :
//=================================================================================
class LightApp_ClippingDlg : public QDialog
{
    Q_OBJECT
    
    public:
    LightApp_ClippingDlg(OCCViewer_ViewWindow* , QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~LightApp_ClippingDlg();
    
private :

    virtual void closeEvent( QCloseEvent* e );
    virtual void showEvent ( QShowEvent * );
    virtual void hideEvent ( QHideEvent * );
    void displayPreview();
    void erasePreview();

    bool isValid();
     
    QGroupBox* GroupPoint;
    QLabel* TextLabelX;
    QLabel* TextLabelY;
    QLabel* TextLabelZ;
    QtxDblSpinBox* SpinBox_X;
    QtxDblSpinBox* SpinBox_Y;
    QtxDblSpinBox* SpinBox_Z;
    QPushButton* resetButton;
    
    QGroupBox* GroupDirection;
    QLabel* TextLabelDx;
    QLabel* TextLabelDy;
    QLabel* TextLabelDz;
    QtxDblSpinBox* SpinBox_Dx;
    QtxDblSpinBox* SpinBox_Dy;
    QtxDblSpinBox* SpinBox_Dz;
    QPushButton* invertButton;
    
    QComboBox* DirectionCB;

    QCheckBox* PreviewChB;

    QPushButton*    buttonApply;
    QPushButton*    buttonClose;
    
    OCCViewer_ViewWindow* myView;

    Handle(AIS_Plane) myPreviewPlane;
    
    bool myBusy;
    
private slots:
  void ClickOnApply();
  void ClickOnClose();

  void onReset();
  void onInvert();
  void onModeChanged( int mode );
  void onValueChanged();
  void onPreview( bool on );
};

#endif // LIGHTAPP_CLIPPINGDLG_H
