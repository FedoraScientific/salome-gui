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
#ifndef CAF_STUDY_H
#define CAF_STUDY_H

#include "CAF.h"

#include "SUIT_Study.h"

#include <qobject.h>

#include <TDocStd_Document.hxx>
#include <TDocStd_Application.hxx>

class CAF_Application;

#if defined WNT
#pragma warning ( disable: 4251 )
#endif

/*!
  \class CAF_Study
  Represents study for using in CAF, contains reference
  to OCAF std document and allows to use OCAF services.
  Provides necessary functionality for OCC transactions management.
*/
class CAF_EXPORT CAF_Study : public SUIT_Study
{
  Q_OBJECT

public:
	CAF_Study( SUIT_Application* theApp );
	CAF_Study( SUIT_Application* theApp, Handle(TDocStd_Document)& aStdDoc );
	virtual ~CAF_Study();

  virtual void                createDocument();
  virtual void                closeDocument( bool = true );
  virtual bool                openDocument( const QString& );

  virtual bool                saveDocumentAs( const QString& );

  bool                        isSaved() const;
	bool                        isModified() const;
	void                        doModified( bool = true );
	void                        undoModified();
	void                        clearModified();

  bool                        undo();
	bool                        redo();
	bool                        canUndo() const;
	bool                        canRedo() const;
	QStringList                 undoNames() const;
	QStringList                 redoNames() const;

  Handle(TDocStd_Document)    stdDoc() const;

protected:
  Handle(TDocStd_Application) stdApp() const;
  CAF_Application*            cafApplication() const;

  virtual bool                openTransaction();
  virtual bool                abortTransaction();
  virtual bool                hasTransaction() const;
  virtual bool                commitTransaction( const QString& = QString::null );

  virtual void                setStdDoc( Handle(TDocStd_Document)& );

private:
	Handle(TDocStd_Document)    myStdDoc;
	int                         myModifiedCnt;

  friend class CAF_Operation;
};

#if defined WNT
#pragma warning ( default: 4251 )
#endif

#endif
