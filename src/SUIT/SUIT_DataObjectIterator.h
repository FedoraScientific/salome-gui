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
#ifndef SUIT_DATAOBJECT_ITERATOR_H
#define SUIT_DATAOBJECT_ITERATOR_H

#include "SUIT_DataObject.h"

/*!
  \class SUIT_DataObjectIterator
  Provides functionality of SUIT tree detour
  It is possible to use "to-depth" or "to-breadth" detour and
  to control level detour (from left or from right)
*/
class SUIT_EXPORT SUIT_DataObjectIterator
{
public:
  typedef enum { DepthLeft, DepthRight, BreadthLeft, BreadthRight, User } DetourType;

  SUIT_DataObjectIterator( SUIT_DataObject* root, const int det, const bool fromTrueRoot = false );

  virtual void      operator++();
  SUIT_DataObject*  current() const;

  int               depth() const;
  int               detour() const;

protected: 
  SUIT_DataObject*  parent( SUIT_DataObject* ) const;
  SUIT_DataObject*  extreme( DataObjectList&, bool FromLeft ) const;
  SUIT_DataObject*  globalSibling( SUIT_DataObject*, bool next ) const;

protected:
  SUIT_DataObject*  myRoot;
  SUIT_DataObject*  myCurrent;

private:
  SUIT_DataObject*  myExtremeChild;
  int               myDetourType, myCurrentLevel;
};

/*!
  \class SUIT_DataObjectIterator
  Provides functionality of SUIT tree detour between certain levels 
  Only "to-breadth" detour is used
*/
class SUIT_DataObjectLevelIterator : public SUIT_DataObjectIterator
{
public:
  SUIT_DataObjectLevelIterator( SUIT_DataObject* root, const int start, const int end = 0,
                                  const bool LeftToRight = true );
  virtual void      operator++();

protected:
  int myStartLevel;
  int myEndLevel;
};

#endif
