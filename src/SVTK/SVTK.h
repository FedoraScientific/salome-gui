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
#ifndef SVTK_H
#define SVTK_H

#include "VTKViewer.h"

#ifdef WNT
#  ifdef SVTK_EXPORTS
#    define SVTK_EXPORT __declspec(dllexport)
#  else
#   define SVTK_EXPORT __declspec(dllimport)
#  endif
#else
#  define SVTK_EXPORT
#endif

#if defined WNT
#pragma warning ( disable: 4251 )
#pragma warning ( disable: 4786 )
#endif

#endif
