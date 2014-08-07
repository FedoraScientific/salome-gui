# Copyright (C) 2007-2014  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

INCLUDE(FindQt4)
INCLUDE(UseQt4)
SET(QT_INCLUDE_DIRS ${QT_INCLUDES})
SET(DIRS)
FOREACH(D ${QT_INCLUDES})
SET(DIRS ${DIRS} -I${D})
ENDFOREACH(D ${QT_INCLUDES})
SET(QT_INCLUDES ${DIRS}) # to be removed
SET(QT_INCLUDES ${QT_INCLUDES} -DQT_THREAD_SUPPORT) # to be removed
SET(QT_DEFINITIONS -DQT_THREAD_SUPPORT)
SET(QT_MT_LIBS ${QT_LIBRARIES} ${QT_QTXML_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTWEBKIT_LIBRARY})
SET(QT_LIBS ${QT_MT_LIBS})
SET(qt4_ldflags ${QT_MT_LIBS})
SET(QT_LIBS ${QT_MT_LIBS})

FIND_PROGRAM(QT_LRELEASE_EXECUTABLE lrelease)
