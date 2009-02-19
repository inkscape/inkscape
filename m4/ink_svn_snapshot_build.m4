# Check for SVN snapshot build
# (c) 2009 Krzysztof Kosiński
# Released under GNU GPL; see the file COPYING for more information

AC_DEFUN([INK_SVN_SNAPSHOT_BUILD],
[
  AC_CACHE_CHECK([for SVN snapshot build], ink_cv_svn_snapshot_build,
                 [ink_cv_svn_snapshot_build=no
                  if which svn > /dev/null && test -e $srcdir/.svn/entries; then
                    ink_cv_svn_snapshot_build=yes
                  fi
  ])
  AM_CONDITIONAL([USE_SVN_VERSION], [test "x$ink_cv_svn_snapshot_build" = "xyes"])
])
