#!/bin/sh
# Copyright (C) 2010-2014  CEA/DEN, EDF R&D, OPEN CASCADE
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

#
# ==================================================================
#_debut_usage
#
# USAGE:
#
#      dlgfactory.sh -n className [-t classType]
#
#
# DESCRIPTION:
#
# This script generates a set of files to initiate a dialog qt window
# (i.e. MyDialog.ui, MyDialog.h and MyDialog.cxx files).
#
# The dialog window can be a self-consistent class (i.e. depends only
# on Qt classes) or a classe that inherits from the class
# GenericDialog whose implementation is provided in this package and
# whose design is defined by the GenericDialog.ui file (editable using
# the qt designer).
#
# The -t option let you choose between the two possibilities (specify
# "-t qdialog" for the first case, "-t gdialog" otherwise).
#
# OPTION:
#          -n : specify the name of the class (default is TestDialog)
#
#          -t : specify the type of the class (default is qdialog)
#
#_fin_usage
# ==================================================================
#

thisScript=$(which $0)
TOOLDIRNAME=$(dirname $0)

displayUsage()
{
  cat $thisScript | sed -e '/^#_debut_usage/,/^#_fin_usage/!d' \
   -e '/^#_debut_usage/d' \
   -e '/^#_fin_usage/d' \
   -e 's/^#//g'
}

#
# Read the options on the command line
#
className=TestDialog
classType=dialog
if [ $# -eq 0 ]; then
    displayUsage
    exit 1
else
    while [ $# -ne 0 ]; do
        case $1 in
            -n)
                shift; className=$1; shift ;;
            -t)
                shift; classType=$1; shift ;;
            *)
                displayUsage;;
        esac
    done
fi

if [ "$classType" = "qdialog" ]; then
    sed s/__CLASSNAME__/$className/g $TOOLDIRNAME/__QDIALOG__.ui > $className.ui
    sed s/__CLASSNAME__/$className/g $TOOLDIRNAME/__QDIALOG__.h > $className.h
    sed s/__CLASSNAME__/$className/g $TOOLDIRNAME/__QDIALOG__.cxx > $className.cxx
else
    sed s/__CLASSNAME__/$className/g $TOOLDIRNAME/__GDIALOG__.ui > $className.ui
    sed s/__CLASSNAME__/$className/g $TOOLDIRNAME/__GDIALOG__.h > $className.h
    sed s/__CLASSNAME__/$className/g $TOOLDIRNAME/__GDIALOG__.cxx > $className.cxx    
fi

displayMessage()
{
  cat $thisScript | sed -e '/^#_debut_message/,/^#_fin_message/!d' \
   -e '/^#_debut_message/d' \
   -e '/^#_fin_message/d' \
   -e 's/^#//g' \
   -e "s/__CLASSNAME__/$className/g"
}

#_debut_message
##
## ---------------------------------------------------------
## Generation rules to create moc files from QObject headers
## and form source files from ui files
## ---------------------------------------------------------
##
#%_moc.cxx: %.h
#	$(MOC) $< -o $@
#
#ui_%.h: %.ui
#	$(UIC) -o $@ $<
#
##
## ---------------------------------------------------------
## Declaration of form files generated by UIC and MOC files
## as BUILT_SOURCES to be used in the building process.
## ---------------------------------------------------------
##
#UIC_FILES = \
#	ui___CLASSNAME__.h
##
#MOC_FILES = \
#	__CLASSNAME___moc.cxx
#
#BUILT_SOURCES = $(UIC_FILES)
#
##
## ---------------------------------------------------------
## Declaration of sources files to the building process
## ---------------------------------------------------------
## MOC files and UIC files should be added to the list of undistributed
## source files with something like (where <MyLibrary> should be
## replaced by the name of the product declared by the directive
## lib_LTLIBRARIES):
##
#nodist_<MyLibrary>_la_SOURCES += $(MOC_FILES) $(UIC_FILES)
#
#dist_<MyLibrary>_la_SOURCES += __CLASSNAME__.cxx
#salomeinclude_HEADERS       += __CLASSNAME__.h
#
#<MyLibrary>_la_CPPFLAGS = \
#	$(QT_CXXFLAGS)
#
#<MyLibrary>_la_LDFLAGS = \
#        $(QT_LIBS)

#_fin_message

echo "Note that the following directives should be present in your CMakeLists.txt (or something like that):"
echo ""
displayMessage

