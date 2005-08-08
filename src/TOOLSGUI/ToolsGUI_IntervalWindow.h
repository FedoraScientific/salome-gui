//  SALOME RegistryDisplay : GUI for Registry server implementation
//
//  Copyright (C) 2003  CEA/DEN, EDF R&D
//
//
//
//  File   : IntervalWindow.hxx
//  Author : Oksana TCHEBANOVA
//  Module : SALOME

#ifndef IntervalWindow_HeaderFile
#define IntervalWindow_HeaderFile

# include <qwidget.h>
# include <qdialog.h>
# include <qpushbutton.h>
# include <qspinbox.h>

#ifndef WNT
using namespace std;
#endif

class ToolsGUI_IntervalWindow : public QDialog
{
public:
  ToolsGUI_IntervalWindow( QWidget* parent = 0 );
  ~ToolsGUI_IntervalWindow();
  
  QPushButton* Ok();
  QPushButton* Cancel();

  int getValue();
  void setValue( int );

private:
  QSpinBox* mySpinBox;
  QPushButton* myButtonOk;
  QPushButton* myButtonCancel;
};

#endif
