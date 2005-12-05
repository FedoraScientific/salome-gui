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
#ifndef VTKVIEWER_TRANSFORMFILTER_H
#define VTKVIEWER_TRANSFORMFILTER_H

#include "VTKViewer.h"

#include <vtkTransformFilter.h>

/*!Transform points and associated normals and vectors
 *@see vtkTransformFilter
 */
class VTKVIEWER_EXPORT VTKViewer_TransformFilter : public vtkTransformFilter
{
public:
  /*!Create new instance of VTKViewer_TransformFilter.*/
  static VTKViewer_TransformFilter *New();
  vtkTypeMacro(VTKViewer_TransformFilter,vtkTransformFilter);

protected:
  /*!Constructor.*/
  VTKViewer_TransformFilter() {/*!Do nothing*/}
  /*!Destructor.*/
  ~VTKViewer_TransformFilter() {/*!Do nothing*/}
  /*!Copy constructor.*/
  VTKViewer_TransformFilter(const VTKViewer_TransformFilter&) {/*!Do nothing*/}
  /*!Operator = */
  void operator=(const VTKViewer_TransformFilter&) {/*!Do nothing*/}
  void Execute();
};

#endif
