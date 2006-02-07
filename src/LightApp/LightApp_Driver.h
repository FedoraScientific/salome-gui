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
#ifndef LIGHTAPP_DRIVER_H
#define LIGHTAPP_DRIVER_H

#include <LightApp.h>

#include "string"
#include "vector"
#include "map"

#ifdef WIN32
#pragma warning( disable:4251 )
#endif

/*!Description : Driver can save to file and read from file list of files for light modules*/

class LIGHTAPP_EXPORT LightApp_Driver
{
public:
  LightApp_Driver();
  virtual ~LightApp_Driver();


  typedef std::vector<std::string> ListOfFiles;

  virtual bool        SaveDatasInFile   (const char* theFileName, bool isMultiFile);
  virtual bool        ReadDatasFromFile (const char* theFileName, bool isMultiFile);
  virtual std::string GetTmpDir      (const char* theURL, const bool isMultiFile);

  ListOfFiles         GetListOfFiles (const char* theModuleName);
  virtual void        SetListOfFiles (const char* theModuleName, const ListOfFiles theListOfFiles);
  virtual void        RemoveTemporaryFiles(const char* theModuleName, const bool IsDirDeleted);
  void                RemoveFiles( const ListOfFiles& theFiles, const bool IsDirDeleted);

  virtual void        ClearDriverContents();

protected:
  void                PutFilesToStream(const std::string& theModuleName, unsigned char*& theBuffer,
                                       long& theBufferSize, bool theNamesOnly = false);
  ListOfFiles         PutStreamToFiles(const unsigned char* theBuffer,
                                       const long theBufferSize, bool theNamesOnly = false);

  std::string GetTmpDir();
  std::string GetDirFromPath(const std::string& thePath);

  void                SetIsTemporary( bool theFlag ) { myIsTemp = theFlag; }
  bool                IsTemporary() const { return myIsTemp; }

protected:
  typedef std::map<std::string, ListOfFiles> MapOfListOfFiles;
  MapOfListOfFiles                           myMap;
  std::string                                myTmpDir;

private:
  bool                                       myIsTemp;
};

#endif 
