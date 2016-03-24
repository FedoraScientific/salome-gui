// Copyright (C) 2015-2016  OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
// File   : PyEditor_SettingsDlg.h
// Author : Maxim GLIBIN, Open CASCADE S.A.S. (maxim.glibin@opencascade.com)
//

#ifndef PYEDITOR_SETTINGSDLG_H
#define PYEDITOR_SETTINGSDLG_H

#include <QDialog>
#include "PyEditor.h"

class PyEditor_Editor;
class QCheckBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QtxFontEdit;

class PYEDITOR_EXPORT PyEditor_SettingsDlg : public QDialog
{
  Q_OBJECT

public:
  PyEditor_SettingsDlg( PyEditor_Editor*, QWidget* = 0 );

  bool    isSetAsDefault();

private Q_SLOTS:
  void onVerticalEdgeChecked( bool );
  void onOk();
  void onHelp();

Q_SIGNALS:
  void onHelpClicked();

private:
  void settingsToGui();
  void settingsFromGui();
  void setSettings();
  
  QCheckBox*        w_HighlightCurrentLine;
  QCheckBox*        w_TextWrapping;
  QCheckBox*        w_CenterCursorOnScroll;
  QCheckBox*        w_LineNumberArea;

  QCheckBox*        w_TabSpaceVisible;
  QSpinBox*         w_TabSize;

  QCheckBox*        w_VerticalEdge;
  QSpinBox*         w_NumberColumns;
  QLabel*           lbl_NumColumns;

  QtxFontEdit*      w_FontWidget;

  QCheckBox*        w_DefaultCheck;

  QPushButton*      myOkBtn;
  QPushButton*      myCancelBtn;
  QPushButton*      myHelpBtn;

  PyEditor_Editor*  my_Editor;
};

#endif // PYEDITOR_SETTINGSDLG_H
