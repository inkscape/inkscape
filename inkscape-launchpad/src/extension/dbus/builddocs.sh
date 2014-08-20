xsltproc doc/spec-to-docbook.xsl application-interface.xml > doc/org.inkscape.application.ref.xml &&
xsltproc doc/spec-to-docbook.xsl document-interface.xml > doc/org.inkscape.document.ref.xml &&
xsltproc doc/spec-to-docbook.xsl proposed-interface.xml > doc/org.inkscape.proposed.ref.xml &&
xmlto --skip-validation xhtml-nochunks -o doc -m doc/config.xsl doc/inkscapeDbusRef.xml &&
firefox doc/inkscapeDbusRef.html

