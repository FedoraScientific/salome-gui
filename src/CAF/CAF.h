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
#ifndef CAF_H
#define CAF_H

#if defined CAF_EXPORTS
#if defined WNT
#define CAF_EXPORT __declspec( dllexport )
#else
#define CAF_EXPORT
#endif
#else
#if defined WNT
#define CAF_EXPORT __declspec( dllimport )
#else
#define CAF_EXPORT
#endif
#endif

#if defined SOLARIS
#define bool  int
#define false 0
#define true  1
#endif

#if defined WNT
#pragma warning ( disable: 4251 )
#endif

#if defined ( _DEBUG ) || defined ( DEBUG )
#include <assert.h>
#define CAF_VERIFY(x) assert(x);
#define CAF_ASSERT(x) assert(x);
#else
#define CAF_VERIFY(x) x
#define CAF_ASSERT(x)
#endif

#endif
