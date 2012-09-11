dnl $Id: config.m4 213 2007-03-11 20:37:47Z j $
dnl config.m4 for extension riakc

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(riakc, for riakc support,
[  --with-riakc[=DIR]             Include riakc support])

if test "$PHP_RIAKC" != "no"; then
  for i in $PHP_RIAKC /usr/local /usr; do
    echo $i
    if test -r "$i/bin/riakc"; then
      RIAKC_DIR=$i
      break
    fi
  done

  PHP_REQUIRE_CXX()

dnl  RIAKC_LIBS=$($RIAKC_DIR/lib --libs)
dnl  RIAKC_INCS=$($RIAKC_DIR/include --cflags)
  
dnl  PHP_EVAL_LIBLINE($RIAKC_LIBS, RIAKC_SHARED_LIBADD)
dnl  PHP_EVAL_INCLINE($RIAKC_INCS)  

  if test -z "$RIAKC_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Could not find the riak-cxx-client header files: $RIAKC_DIR)
  fi

  PHP_ADD_INCLUDE($RIAKC_DIR/include)

  PHP_SUBST(RIAKC_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, RIAKC_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(riak_client, $RIAKC_DIR/lib, RIAKC_SHARED_LIBADD)
  PHP_NEW_EXTENSION(riakc, riakc.cpp, $ext_shared)
fi
