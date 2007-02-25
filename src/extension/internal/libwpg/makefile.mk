PRJ=..$/..$/..$/..$/..$/..

PRJNAME=libwpg
TARGET=wpglib
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

SLOFILES= \
	$(SLO)$/WPGraphics.obj \
	$(SLO)$/WPGPen.obj \
	$(SLO)$/WPGGradient.obj \
	$(SLO)$/WPGPoint.obj \
	$(SLO)$/WPGPath.obj \
	$(SLO)$/WPGHeader.obj \
	$(SLO)$/WPGXParser.obj \
	$(SLO)$/WPG1Parser.obj \
	$(SLO)$/WPG2Parser.obj \
	$(SLO)$/WPGOLEStream.obj \
	$(SLO)$/WPGStreamImplementation.obj

LIB1ARCHIV=$(LB)$/libwpglib.a
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1OBJFILES= $(SLOFILES)

.INCLUDE :  target.mk
