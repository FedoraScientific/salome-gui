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
#ifndef SALOME_GLOWNER_H
#define SALOME_GLOWNER_H

#include <string>
//#include <GLViewer.h>
#include <GLViewer_Object.h>

#ifdef WNT
#define SALOME_OBJECT_EXPORT __declspec (dllexport)
#else
#define SALOME_OBJECT_EXPORT
#endif

class SALOME_OBJECT_EXPORT SALOME_GLOwner : public GLViewer_Owner
{
public:
  SALOME_GLOwner( const char* );
  ~SALOME_GLOwner();

  const char*       entry() const;
  void              setEntry( const char* );

private:
  std::string       myEntry;
};

#endif
