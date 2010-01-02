# Check for BZR snapshot build
# (c) 2009 Krzysztof Kosiński
# Released under GNU GPL; see the file COPYING for more information

AC_DEFUN([INK_BZR_SNAPSHOT_BUILD],
[
  AC_CACHE_CHECK([for BZR snapshot build], ink_cv_bzr_snapshot_build,
                 [ink_cv_bzr_snapshot_build=no
                  if which bzr > /dev/null && test -e $srcdir/.bzr/branch/last-revision; then
                    ink_cv_bzr_snapshot_build=yes
                  fi
  ])
  AM_CONDITIONAL([USE_BZR_VERSION], [test "x$ink_cv_bzr_snapshot_build" = "xyes"])
])
