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
#ifndef CAF_OPERATION_H
#define CAF_OPERATION_H

#include "CAF.h"

#include "SUIT_Operation.h"

#include <qobject.h>
#include <qstring.h>

#include <Standard.hxx>

class CAF_Study;
//! OCC OCAF Std document
class Handle(TDocStd_Document);

/*!
  \class CAF_Operation
  Base operation for all operations used in CAF package
  Operation has link to OCC OCAF std document
*/
class CAF_EXPORT CAF_Operation : public SUIT_Operation
{
	Q_OBJECT

public:
	CAF_Operation( SUIT_Application* );
	virtual ~CAF_Operation();

protected:
  Handle(TDocStd_Document) stdDoc() const;
};

#endif
