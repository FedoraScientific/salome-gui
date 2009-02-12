//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// File   : Style.h
// Author : Natalia Ermolaeva, Open CASCADE S.A.S.
//
#ifndef STYLE_H
#define STYLE_H

#if defined WIN32
#  if defined STYLE_SALOME_EXPORTS || defined STYLE_EXPORTS
#    define STYLE_SALOME_EXPORT _declspec( dllexport )
#  else
#    define STYLE_SALOME_EXPORT _declspec( dllimport )
#  endif
#else
#  define STYLE_SALOME_EXPORT  
#endif

#endif // STYLE_H
