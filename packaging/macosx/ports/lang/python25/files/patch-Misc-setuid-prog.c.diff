--- Misc/setuid-prog.c.orig	Sat Dec 11 14:29:22 2004
+++ Misc/setuid-prog.c	Sat Dec 11 14:30:13 2004
@@ -70,6 +70,12 @@
 #define environ _environ
 #endif
 
+#if defined(__APPLE__)
+#include <sys/time.h>
+#include <crt_externs.h>
+#define environ (*_NSGetEnviron())
+#endif
+
 /* don't change def_IFS */
 char def_IFS[] = "IFS= \t\n";
 /* you may want to change def_PATH, but you should really change it in */

