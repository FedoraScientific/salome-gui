// Copyright (C) 2007-2015  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
// Author : Adrien Bruneton (CEA/DEN)
// Created on: 4 avr. 2013

#ifndef PYCONSOLE_ENHEDITORBASE_H_
#define PYCONSOLE_ENHEDITORBASE_H_

#include "PyConsoleBase.h"
#include "PyConsole_EditorBase.h"

#include <QObject>
#include <queue>

/**
 * Enhanced Python editor handling tab completion.
 */
class PYCONSOLEBASE_EXPORT PyConsole_EnhEditorBase : public PyConsole_EditorBase
{
  Q_OBJECT;

public:
  PyConsole_EnhEditorBase(PyConsole_Interp* interp, QWidget* parent = 0);
  virtual ~PyConsole_EnhEditorBase() {}

signals:
  /**
   * Signal emitted by the editor widget when the doc string should be updated.
   * @param doc a HTML block with the formatted doc string.
   * TODO: for now this signal is left uncaught.
   */
  void updateDoc(QString doc);

protected:
  /** List of separators identifying the last parsable token for completion */
  static const std::vector<QString> SEPARATORS;

  /** Maximum number of completions shown at once */
  static const int MAX_COMPLETIONS = 70;

  /** Are we in completion mode */
  bool _tab_mode;

  /** String on which the dir() comamnd is executed */
  QString _compl_before_point;
  /** String on which the results of the dir() are matched */
  QString _compl_after_point;

  /** Cursor position when <TAB> is hit */
  int _cursor_pos;

  /** Are we currently pasting several lines */
  bool _multi_line_paste;

  /** Queue of lines being pasted */
  std::queue<QString> _multi_line_content;

  // Overrides:
  virtual void   keyPressEvent ( QKeyEvent* event);
  virtual void   customEvent( QEvent* event);
  virtual void   mousePressEvent( QMouseEvent* event );
  virtual void   insertFromMimeData(const QMimeData* source);

  virtual PyInterp_Request* createTabRequest( const QString& input );
  virtual void handleTab();
  virtual void handleBackTab();
  virtual void clearCompletion();
  virtual void formatCompletion(const QStringList& matches, QString& result) const;
  virtual QString formatDocHTML(const QString & doc) const;

  virtual void multilinePaste(const QString & s);
  virtual void multiLineProcessNextLine();

private:
  void extractCommon(const QStringList& matches, QString& result) const;

};

#endif /* PYCONSOLE_ENHEDITORBASE_H_ */
