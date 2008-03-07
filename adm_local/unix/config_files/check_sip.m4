dnl Copyright (C) 2003  CEA/DEN, EDF R&D

AC_DEFUN([CHECK_SIP],[
AC_REQUIRE([CHECK_PYTHON])dnl
AC_REQUIRE([CHECK_QT])dnl
AC_REQUIRE([AC_LINKER_OPTIONS])dnl

sip_ok=yes

AC_ARG_WITH(sip,
    [  --with-sip=EXEC sip executable ],
    [SIP="$withval"
      AC_MSG_RESULT("select $withval as sip executable")
    ], [
      AC_PATH_PROG(SIP, sip)
    ])

if test "x$SIP" = "x" ; then
    sip_ok=no
    AC_MSG_RESULT(sip not in PATH variable)
else
    version=`$SIP -V`
    AC_MSG_RESULT(sip version is $version)
    case "$version" in
         3.2*)
           SIP_VERS=v3_old ;;
         3.3*)
           SIP_VERS=v3_old ;;
         3.4*)
           SIP_VERS=v3_old ;;
         3.5*)
           SIP_VERS=v3_old ;;
         3.6*)
           SIP_VERS=v3_old ;;
         3.7*)
           SIP_VERS=v3_old ;;
         3.8*)
           SIP_VERS=v3_old ;;
         3.9*)
           SIP_VERS=v3_old ;;
         3.10*)
           SIP_VERS=v3_new ;;
         4.1*)
           SIP_VERS=v4_old ;;
         4.2*)
           SIP_VERS=v4_new ;;
         4.3*)
           SIP_VERS=v4_new ;;
         4.4*)
           SIP_VERS=v4_new ;;
         4.5*)
           SIP_VERS=v4_new ;;
	 4.6*)
           SIP_VERS=v4_new ;;
	 4.7*)
           SIP_VERS=v4_new ;;
         *)
           AC_MSG_RESULT(sip version $version not supported)
           SIP_VERS=unsupported ;;
    esac

    if test "x$SIPDIR" = "x" ; then
        SIPDIR=`dirname $SIP`
        SIPDIR=`dirname $SIPDIR`
    fi

    sip_ok=no
    dnl Search sip.h file
    if test "x$SIPDIR" != "x" ; then
        dnl look for sip.h in ${SIPDIR}
        AC_CHECK_FILE(${SIPDIR}/sip.h,sip_ok=yes,sip_ok=no)
	if test "x$sip_ok" == "xyes" ; then
	    SIP_INCLUDES="-I${SIPDIR}"
	fi
        if test "x$sip_ok" == "xno" ; then
            dnl look for sip.h in ${SIPDIR}/include/python${PYTHON_VERSION}
	    if test -d ${SIPDIR}/include/python${PYTHON_VERSION} ; then
		AC_CHECK_FILE(${SIPDIR}/include/python${PYTHON_VERSION}/sip.h,sip_ok=yes,sip_ok=no)
      	        if test "x$sip_ok" == "xyes" ; then
	            SIP_INCLUDES="-I${SIPDIR}/include/python${PYTHON_VERSION}"
		fi
	    fi
	fi
    fi
    if test "x$sip_ok" == "xno" ; then
        dnl look for sip.h in ${PYTHON_PREFIX}/include/python${PYTHON_VERSION}
	if test -d ${PYTHON_PREFIX}/include/python${PYTHON_VERSION} ; then
            AC_CHECK_FILE(${PYTHON_PREFIX}/include/python${PYTHON_VERSION}/sip.h,sip_ok=yes,sip_ok=no)
	    if test "x$sip_ok" == "xyes" ; then
	        SIP_INCLUDES="${PYTHON_INCLUDES}"
            fi
	fi
    fi
    if test "x$sip_ok" == "xno" ; then
        dnl look for sip.h in /usr/include/python${PYTHON_VERSION}
	if test -d /usr/include/python${PYTHON_VERSION} ; then
            AC_CHECK_FILE(/usr/include/python${PYTHON_VERSION}/sip.h,sip_ok=yes,sip_ok=no)
	    if test "x$sip_ok" == "xyes" ; then
	        SIP_INCLUDES="/usr/include/python${PYTHON_VERSION}"
            fi
	fi
    fi
	
    dnl Search (lib)sip.so file
    sip_lib_ok=no
    if test "x$SIPDIR" != "x" ; then
        dnl look for (lib)sip.so in ${SIPDIR}
        AC_CHECK_FILE(${SIPDIR}/libsip.so,sip_lib_ok=yes,sip_lib_ok=no)
	if test "x$sip_lib_ok" == "xyes" ; then
            if test "x${SIPDIR}" = "x/usr/lib"
            then
              SIP_LIBS="-lsip"
            else
              SIP_LIBS="-L${SIPDIR} -lsip"
            fi
	else
	    AC_CHECK_FILE(${SIPDIR}/sip.so,sip_lib_ok=yes,sip_lib_ok=no)
	    if test "x$sip_lib_ok" == "xyes" ; then
              if test "x${SIPDIR}" = "x/usr/lib"
              then
                SIP_LIBS=""
              else
                SIP_LIBS="-L${SIPDIR}"
              fi
	    fi
	fi

        if test "x$sip_lib_ok" == "xno" ; then
            dnl look for (lib)sip.so in ${SIPDIR}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages
	    if test -d ${SIPDIR}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages ; then
		AC_CHECK_FILE(${SIPDIR}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages/libsip.so,sip_lib_ok=yes,sip_lib_ok=no)
      	        if test "x$sip_lib_ok" == "xyes" ; then
	            SIP_LIBS="-L${SIPDIR}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages -lsip"
	        else
	            AC_CHECK_FILE(${SIPDIR}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages/sip.so,sip_lib_ok=yes,sip_lib_ok=no)
	            if test "x$sip_lib_ok" == "xyes" ; then
	                SIP_LIBS="-L${SIPDIR}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages"
	            fi
		fi
	    fi
	fi
    fi
    if test "x$sip_lib_ok" == "xno" ; then
        dnl look for (lib)sip.so in ${PYTHON_PREFIX}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages
	if test -d ${PYTHON_PREFIX}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages ; then
            AC_CHECK_FILE(${PYTHON_PREFIX}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages/libsip.so,sip_lib_ok=yes,sip_lib_ok=no)
	    if test "x$sip_lib_ok" == "xyes" ; then
	        SIP_LIBS="-L${PYTHON_PREFIX}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages -lsip"
	    else
	        AC_CHECK_FILE(${PYTHON_PREFIX}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages/sip.so,sip_lib_ok=yes,sip_lib_ok=no)
                if test "x$sip_lib_ok" == "xyes" ; then
	            SIP_LIBS="-L${PYTHON_PREFIX}/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages"
                fi
	    fi
	fi
    fi
    if test "x$sip_lib_ok" == "xno" ; then
        dnl look for (lib)sip.so in /usr/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages
	if test -d /usr/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages ; then
            AC_CHECK_FILE(/usr/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages/libsip.so,sip_lib_ok=yes,sip_lib_ok=no)
	    if test "x$sip_lib_ok" == "xyes" ; then
	        SIP_LIBS="-L$/usr/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages -lsip"
	    else
	        AC_CHECK_FILE(/usr/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages/sip.so,sip_lib_ok=yes,sip_lib_ok=no)
                if test "x$sip_lib_ok" == "xyes" ; then
	            SIP_LIBS="-L/usr/lib${LIB_LOCATION_SUFFIX}/python${PYTHON_VERSION}/site-packages"
	        fi
            fi
	fi
    fi
    if test "x$sip_lib_ok" == "xno" ; then
        sip_ok=no
    fi
fi

AC_SUBST(SIP)
AC_SUBST(SIP_INCLUDES)
AC_SUBST(SIP_LIBS)
AC_SUBST(SIP_VERS)

AM_CONDITIONAL(IS_SIP_V4, [test $SIP_VERS == v4_old || test $SIP_VERS == v4_new])
AM_CONDITIONAL(IS_SIP_NEW, [test $SIP_VERS == v3_new || test $SIP_VERS == v4_new])

AC_MSG_RESULT(for sip: $sip_ok)

])dnl
dnl
