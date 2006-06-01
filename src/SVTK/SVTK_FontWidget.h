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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : 
//  Author : Sergey LITONIN
//  Module : 


#ifndef SVTK_FontWidget_H
#define SVTK_FontWidget_H

#include <qhbox.h>

class QToolButton;
class QComboBox;
class QCheckBox;
class QColor;


/*!
 * Class       : SVTK_FontWidget
 * Description : Dialog for specifynig font
 */
class SVTK_FontWidget : public QHBox
{
  Q_OBJECT

public:
                SVTK_FontWidget( QWidget* );
  virtual       ~SVTK_FontWidget();

  void          SetColor( const QColor& );
  QColor        GetColor() const;

  void          SetData( const QColor&, const int, const bool, const bool, const bool );

  void          GetData( QColor&, int&, bool&, bool&, bool& ) const;

private slots:
  void          onColor();

private:
  QToolButton*  myColorBtn;
  QComboBox*    myFamily;
  QCheckBox*    myBold;
  QCheckBox*    myItalic;
  QCheckBox*    myShadow;
};

#endif
