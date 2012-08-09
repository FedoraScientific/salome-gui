// Copyright (C) 2010-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef __vtkEDFCutter_h__
#define __vtkEDFCutter_h__

#include "vtkEDFOverloadsDefines.h"
#include "vtkCutter.h"

class VTKTOOLS_EXPORT vtkEDFCutter : public vtkCutter
{
public :
  static vtkEDFCutter* New();
  vtkTypeRevisionMacro(vtkEDFCutter, vtkCutter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // these ivars
  // control the name given to the field in which the ids are written into.  If
  // set to NULL, then vtkOriginalCellIds or vtkOriginalPointIds (the default)
  // is used, respectively.
  vtkSetStringMacro(OriginalCellIdsName);
  virtual const char *GetOriginalCellIdsName() {
    return (  this->OriginalCellIdsName
            ? this->OriginalCellIdsName : "vtkOriginalCellIds");
  }

protected :
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  virtual void  AssembleOutputTriangles(vtkPolyData* inpd,
                                        vtkPolyData* outpd);

  char* OriginalCellIdsName;

  vtkEDFCutter();
  ~vtkEDFCutter();

private:
  vtkEDFCutter(const vtkEDFCutter&);  // Not implemented.
  void operator=(const vtkEDFCutter&);  // Not implemented.
};

#endif //__vtkEDFCutter_h__
