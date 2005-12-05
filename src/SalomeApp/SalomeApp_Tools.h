// Copyright (C) 2005  OPEN CASCADE, CEA/DEN, EDF R&D, PRINCIPIA R&D
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License.
// 
// This library is distributed in the hope that it will be useful 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public  
// License along with this library; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/
//
#ifndef SALOMEAPP_TOOLS_H
#define SALOMEAPP_TOOLS_H

#include "SalomeApp.h"

#include <SUIT_Tools.h>

#include <qcolor.h>
#include <qstring.h>

#include <Quantity_Color.hxx>

#include <SALOMEconfig.h>
#include CORBA_CLIENT_HEADER(SALOME_Exception)

/*! 
  Class which provide color converter and exception message box.
*/
class SALOMEAPP_EXPORT SalomeApp_Tools : public SUIT_Tools
{
public:
  static Quantity_Color  color( const QColor& );
  static QColor          color( const Quantity_Color& );

  static QString         ExceptionToString( const SALOME::SALOME_Exception& );
  static void            QtCatchCorbaException( const SALOME::SALOME_Exception& );
};

#endif
