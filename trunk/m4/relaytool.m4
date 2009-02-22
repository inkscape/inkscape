dnl  Usage: RELAYTOOL(LIBRARY_NAME, LIBS, CFLAGS, ACTION-IF-WEAK-LINK-IS-POSSIBLE)

dnl  Example:
dnl  RELAYTOOL("gtkspell", GTKSPELL_LIBS, GTKSPELL_CFLAGS, gtkspell_weak=yes)
dnl  Will modify GTKSPELL_LIBS to include a call to relaytool if available
dnl  or if not, will modify GTKSPELL_CFLAGS to include -D switches to define
dnl  libgtkspell_is_present=1 and libgtkspell_symbol_is_present=1

AC_DEFUN([RELAYTOOL], [
    if test -z "$RELAYTOOL_PROG"; then
        AC_PATH_PROG(RELAYTOOL_PROG, relaytool, no)
    fi

    AC_MSG_CHECKING(whether we can weak link $1)

    _RELAYTOOL_PROCESSED_NAME=`echo "$1" | sed 's/-/_/g;s/\./_/g;'`
    _RELAYTOOL_UPPER_NAME=`echo $_RELAYTOOL_PROCESSED_NAME | tr '[[:lower:]]' '[[:upper:]]'`

    if test "$RELAYTOOL_PROG" = "no"; then
        AC_MSG_RESULT(no)
        $3="-DRELAYTOOL_${_RELAYTOOL_UPPER_NAME}='static const int lib${_RELAYTOOL_PROCESSED_NAME}_is_present = 1; static int __attribute__((unused)) lib${_RELAYTOOL_PROCESSED_NAME}_symbol_is_present(char *m) { return 1; }' $$3"
    else
        AC_MSG_RESULT(yes)
        $2=`echo $$2|sed 's/\`/\\\\\`/g;'`
        $2="-Wl,--gc-sections \`relaytool --relay $1 $$2\`"
	$3="-DRELAYTOOL_${_RELAYTOOL_UPPER_NAME}='extern int lib${_RELAYTOOL_PROCESSED_NAME}_is_present; extern int lib${_RELAYTOOL_PROCESSED_NAME}_symbol_is_present(char *s);' $$3"
        $4
    fi
])

