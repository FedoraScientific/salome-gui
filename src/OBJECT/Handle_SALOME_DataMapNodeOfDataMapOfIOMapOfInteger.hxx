//  SALOME SALOMEGUI : implementation of desktop and GUI kernel
//
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
//  File   : Handle_SALOME_DataMapNodeOfDataMapOfIOMapOfInteger.hxx
//  Module : SALOME

#ifndef _Handle_SALOME_DataMapNodeOfDataMapOfIOMapOfInteger_HeaderFile
#define _Handle_SALOME_DataMapNodeOfDataMapOfIOMapOfInteger_HeaderFile

#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif
#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif

#ifndef _Handle_TCollection_MapNode_HeaderFile
#include <Handle_TCollection_MapNode.hxx>
#endif

class Standard_Transient;
class Handle_Standard_Type;
class Handle(TCollection_MapNode);
class SALOME_DataMapNodeOfDataMapOfIOMapOfInteger;
Standard_EXPORT Handle_Standard_Type& STANDARD_TYPE(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger);

class Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger) : public Handle(TCollection_MapNode) {
  public:
    inline void* operator new(size_t,void* anAddress) 
      {
        return anAddress;
      }
    inline void* operator new(size_t size) 
      { 
        return Standard::Allocate(size); 
      }
    inline void  operator delete(void *anAddress) 
      { 
        if (anAddress) Standard::Free((Standard_Address&)anAddress); 
      }
//    inline void  operator delete(void *anAddress, size_t size) 
//      { 
//        if (anAddress) Standard::Free((Standard_Address&)anAddress,size); 
//      }
    Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)():Handle(TCollection_MapNode)() {} 
    Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)(const Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)& aHandle) : Handle(TCollection_MapNode)(aHandle) 
     {
     }

    Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)(const SALOME_DataMapNodeOfDataMapOfIOMapOfInteger* anItem) : Handle(TCollection_MapNode)((TCollection_MapNode *)anItem) 
     {
     }

    Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)& operator=(const Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)& aHandle)
     {
      Assign(aHandle.Access());
      return *this;
     }

    Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)& operator=(const SALOME_DataMapNodeOfDataMapOfIOMapOfInteger* anItem)
     {
      Assign((Standard_Transient *)anItem);
      return *this;
     }

    SALOME_DataMapNodeOfDataMapOfIOMapOfInteger* operator->() 
     {
      return (SALOME_DataMapNodeOfDataMapOfIOMapOfInteger *)ControlAccess();
     }

    SALOME_DataMapNodeOfDataMapOfIOMapOfInteger* operator->() const 
     {
      return (SALOME_DataMapNodeOfDataMapOfIOMapOfInteger *)ControlAccess();
     }

   Standard_EXPORT ~Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger)();
 
   Standard_EXPORT static const Handle(SALOME_DataMapNodeOfDataMapOfIOMapOfInteger) DownCast(const Handle(Standard_Transient)& AnObject);
};
#endif
