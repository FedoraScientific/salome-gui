# Copyright (C) 2012-2015  CEA/DEN, EDF R&D, OPEN CASCADE
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
# Author: Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

INCLUDE(UseSIP)

####################################################################
#
# _PYQT_WRAP_GET_UNIQUE_TARGET_NAME: internal function
# 
# Used to generate unique custom target name for usage in
# PYQT_WRAP_UIC macro.
#
# USAGE: _PYQT_WRAP_GET_UNIQUE_TARGET_NAME(prefix unique_name)
#
# ARGUMENTS:
#   prefix [in] prefix for the name
#   unique_name [out] unique name generated by function
#
####################################################################
FUNCTION(_PYQT_WRAP_GET_UNIQUE_TARGET_NAME name unique_name)
   SET(_propertyName "_PYQT_WRAP_UNIQUE_COUNTER_${name}")
   GET_PROPERTY(_currentCounter GLOBAL PROPERTY "${_propertyName}")
   IF(NOT _currentCounter)
      SET(_currentCounter 1)
   ENDIF()
   SET(${unique_name} "${name}_${_currentCounter}" PARENT_SCOPE)
   MATH(EXPR _currentCounter "${_currentCounter} + 1")
   SET_PROPERTY(GLOBAL PROPERTY ${_propertyName} ${_currentCounter} )
ENDFUNCTION()

####################################################################
#
# PYQT_WRAP_UIC macro
#
# Create Python modules by processing input *.ui (Qt designer) files with
# PyQt pyuic tool.
#
# USAGE: PYQT_WRAP_UIC(output_files pyuic_files)
#
# ARGUMENTS:
#   output_files [out] variable where output file names are listed to
#   pyuic_files  [in]  list of *.ui files
# 
# NOTES:
#   - Input files are considered relative to the current source directory.
#   - Output files are generated in the current build directory.
#   - Macro automatically adds custom build target to generate output files
# 
####################################################################
MACRO(PYQT_WRAP_UIC outfiles)

 IF(NOT WIN32)

  FOREACH(_input ${ARGN})
    GET_FILENAME_COMPONENT(_input_name ${_input} NAME)
    STRING(REPLACE ".ui" "_ui.py" _input_name ${_input_name})
    SET(_output ${CMAKE_CURRENT_BINARY_DIR}/${_input_name})
    ADD_CUSTOM_COMMAND(
      OUTPUT ${_output}
      COMMAND ${PYQT_PYUIC_PATH} -o ${_output} ${CMAKE_CURRENT_SOURCE_DIR}/${_input}
      MAIN_DEPENDENCY ${_input}
      )
    SET(${outfiles} ${${outfiles}} ${_output})
  ENDFOREACH()
  _PYQT_WRAP_GET_UNIQUE_TARGET_NAME(BUILD_UI_PY_FILES _uniqueTargetName)
  ADD_CUSTOM_TARGET(${_uniqueTargetName} ALL DEPENDS ${${outfiles}})

 ELSE(NOT WIN32)

####
# ANA: Workaround for the Microsoft Visual Studio 2010. Seems there is a bug in 
# the Microsoft Visual Studio 2010 or CMake 2.8.10.2: custom target doesn't work 
# for the list of the dependencies. It works only for the first dependency in the 
# list. So generate separate target for the each input file. This problem will be 
#investigated in the future.
####

  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
  _PYQT_WRAP_GET_UNIQUE_TARGET_NAME(BUILD_UI_PY_FILES _uniqueTargetName)
  ADD_CUSTOM_TARGET(${_uniqueTargetName} ALL)
  FOREACH(_input ${ARGN})
    GET_FILENAME_COMPONENT(_input_name ${_input} NAME)
    STRING(REPLACE ".ui" "_ui.py" _input_name ${_input_name})
    SET(_output ${CMAKE_CURRENT_BINARY_DIR}/${_input_name})
    _PYQT_WRAP_GET_UNIQUE_TARGET_NAME(BUILD_UI_PY_FILES _TgName)
    ADD_CUSTOM_TARGET(${_TgName} ${PYQT_PYUIC_PATH} -o ${_output} ${CMAKE_CURRENT_SOURCE_DIR}/${_input}
      DEPENDS ${_input}
      )
    SET_TARGET_PROPERTIES(${_TgName} PROPERTIES FOLDER PYQT_WRAP_UIC_TARGETS)
    ADD_DEPENDENCIES(${_uniqueTargetName} DEPEND ${_TgName})
    SET(${outfiles} ${${outfiles}} ${_output})
  ENDFOREACH()
 ENDIF(NOT WIN32)
ENDMACRO(PYQT_WRAP_UIC)

####################################################################
#
# PYQT_WRAP_SIP macro
#
# Generate C++ wrappings for *.sip files by processing them with sip.
#
# USAGE: PYQT_WRAP_SIP(output_files sip_file [sip_file...] [OPTIONS options] [SOURCES sources])
#
# NOTES: See SIP_WRAP_SIP macro from UseSIP.cmake for the usage description.
# 
####################################################################
MACRO(PYQT_WRAP_SIP outfiles)
  SIP_WRAP_SIP(${outfiles} ${ARGN} OPTIONS ${PYQT_SIPFLAGS})
ENDMACRO(PYQT_WRAP_SIP)

####################################################################
#
# PYQT_WRAP_QRC macro
#
# Generate Python wrappings for *.qrc files by processing them with pyrcc5.
#
# USAGE: PYQT_WRAP_QRC(output_files qrc_files)
#
# ARGUMENTS:
#   output_files [out] variable where output file names are listed to
#   qrc_files  [in]  list of *.qrc files
# 
# NOTES:
#   - Input files are considered relative to the current source directory.
#   - Output files are generated in the current build directory.
#   - Macro automatically adds custom build target to generate output files
# 
####################################################################

MACRO(PYQT_WRAP_QRC outfiles)
  FOREACH(_input ${ARGN})
    GET_FILENAME_COMPONENT(_input_name ${_input} NAME)
    STRING(REPLACE ".qrc" "_qrc.py" _input_name ${_input_name})
    SET(_output ${CMAKE_CURRENT_BINARY_DIR}/${_input_name})
    ADD_CUSTOM_COMMAND(
      OUTPUT ${_output}
      COMMAND ${PYQT_PYRCC_PATH} -o ${_output} ${CMAKE_CURRENT_SOURCE_DIR}/${_input}
      MAIN_DEPENDENCY ${_input}
      )
    SET(${outfiles} ${${outfiles}} ${_output})
  ENDFOREACH()
  _PYQT_WRAP_GET_UNIQUE_TARGET_NAME(BUILD_QRC_PY_FILES _uniqueTargetName)
  ADD_CUSTOM_TARGET(${_uniqueTargetName} ALL DEPENDS ${${outfiles}})
ENDMACRO(PYQT_WRAP_QRC)