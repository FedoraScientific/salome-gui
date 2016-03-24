# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

FILE(TO_CMAKE_PATH "$ENV{QWTHOME}" QWTHOME)
IF(NOT QWTHOME)
  SET(QWTHOME /usr)
ENDIF(NOT QWTHOME)

FIND_PATH(QWT_INCLUDE_DIR qwt_plot.h PATHS ${QWTHOME}/include ${QWTHOME}/include/qwt)
SET(QWT_INCLUDES -I${QWT_INCLUDE_DIR}) # to remove
SET(QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR})
IF(WINDOWS)
SET(QWT_INCLUDES ${QWT_INCLUDES} -DQWT_DLL) # to remove
SET(QWT_DEFINITIONS "-DQWT_DLL")
ENDIF(WINDOWS)

IF(WINDOWS)
IF(CMAKE_BUILD_TYPE STREQUAL Debug)
FIND_LIBRARY(QWT_LIBS qwtd5 ${QWTHOME}/lib)
ELSE(CMAKE_BUILD_TYPE STREQUAL Debug)
FIND_LIBRARY(QWT_LIBS qwt5 ${QWTHOME}/lib)
ENDIF(CMAKE_BUILD_TYPE STREQUAL Debug)
ELSE(WINDOWS)
FIND_LIBRARY(QWT_LIBS qwt HINTS ${QWTHOME} PATH_SUFFIXES lib lib64)
ENDIF(WINDOWS)
