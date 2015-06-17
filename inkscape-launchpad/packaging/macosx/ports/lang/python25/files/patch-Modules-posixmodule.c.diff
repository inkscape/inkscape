--- Modules/posixmodule.c.orig	Sat Dec 11 14:27:52 2004
+++ Modules/posixmodule.c	Sat Dec 11 14:28:17 2004
@@ -339,7 +339,7 @@
 #endif
 
 /* Return a dictionary corresponding to the POSIX environment table */
-#ifdef WITH_NEXT_FRAMEWORK
+#ifdef __APPLE__
 /* On Darwin/MacOSX a shared library or framework has no access to
 ** environ directly, we must obtain it with _NSGetEnviron().
 */
@@ -357,7 +357,7 @@
 	d = PyDict_New();
 	if (d == NULL)
 		return NULL;
-#ifdef WITH_NEXT_FRAMEWORK
+#ifdef __APPLE__
 	if (environ == NULL)
 		environ = *_NSGetEnviron();
 #endif

