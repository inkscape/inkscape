
 /** \file
 * PDF parsing using libpoppler
 *
 * Derived from poppler's Gfx.cc
 *
 * Copyright 1996-2003 Glyph & Cog, LLC
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_POPPLER

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

extern "C" {
        
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

}

#include "svg-builder.h"
#include "Gfx.h"
#include "pdf-parser.h"
#include "unit-constants.h"

#include "goo/gmem.h"
#include "goo/GooTimer.h"
#include "goo/GooHash.h"
#include "GlobalParams.h"
#include "CharTypes.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"
#include "GfxFont.h"
#include "GfxState.h"
#include "OutputDev.h"
#include "Page.h"
#include "Annot.h"
#include "Error.h"

// the MSVC math.h doesn't define this
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
// constants
//------------------------------------------------------------------------

// Default max delta allowed in any color component for a shading fill.
#define defaultShadingColorDelta (dblToCol( 1 / 2.0 ))

// Default max recursive depth for a shading fill.
#define defaultShadingMaxDepth 6

// Max number of operators kept in the history list.
#define maxOperatorHistoryDepth 16

//------------------------------------------------------------------------
// Operator table
//------------------------------------------------------------------------

#ifdef WIN32 // this works around a bug in the VC7 compiler
#  pragma optimize("",off)
#endif

PdfOperator PdfParser::opTab[] = {
  {"\"",  3, {tchkNum,    tchkNum,    tchkString},
          &PdfParser::opMoveSetShowText},
  {"'",   1, {tchkString},
          &PdfParser::opMoveShowText},
  {"B",   0, {tchkNone},
          &PdfParser::opFillStroke},
  {"B*",  0, {tchkNone},
          &PdfParser::opEOFillStroke},
  {"BDC", 2, {tchkName,   tchkProps},
          &PdfParser::opBeginMarkedContent},
  {"BI",  0, {tchkNone},
          &PdfParser::opBeginImage},
  {"BMC", 1, {tchkName},
          &PdfParser::opBeginMarkedContent},
  {"BT",  0, {tchkNone},
          &PdfParser::opBeginText},
  {"BX",  0, {tchkNone},
          &PdfParser::opBeginIgnoreUndef},
  {"CS",  1, {tchkName},
          &PdfParser::opSetStrokeColorSpace},
  {"DP",  2, {tchkName,   tchkProps},
          &PdfParser::opMarkPoint},
  {"Do",  1, {tchkName},
          &PdfParser::opXObject},
  {"EI",  0, {tchkNone},
          &PdfParser::opEndImage},
  {"EMC", 0, {tchkNone},
          &PdfParser::opEndMarkedContent},
  {"ET",  0, {tchkNone},
          &PdfParser::opEndText},
  {"EX",  0, {tchkNone},
          &PdfParser::opEndIgnoreUndef},
  {"F",   0, {tchkNone},
          &PdfParser::opFill},
  {"G",   1, {tchkNum},
          &PdfParser::opSetStrokeGray},
  {"ID",  0, {tchkNone},
          &PdfParser::opImageData},
  {"J",   1, {tchkInt},
          &PdfParser::opSetLineCap},
  {"K",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opSetStrokeCMYKColor},
  {"M",   1, {tchkNum},
          &PdfParser::opSetMiterLimit},
  {"MP",  1, {tchkName},
          &PdfParser::opMarkPoint},
  {"Q",   0, {tchkNone},
          &PdfParser::opRestore},
  {"RG",  3, {tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opSetStrokeRGBColor},
  {"S",   0, {tchkNone},
          &PdfParser::opStroke},
  {"SC",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opSetStrokeColor},
  {"SCN", -33, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN},
          &PdfParser::opSetStrokeColorN},
  {"T*",  0, {tchkNone},
          &PdfParser::opTextNextLine},
  {"TD",  2, {tchkNum,    tchkNum},
          &PdfParser::opTextMoveSet},
  {"TJ",  1, {tchkArray},
          &PdfParser::opShowSpaceText},
  {"TL",  1, {tchkNum},
          &PdfParser::opSetTextLeading},
  {"Tc",  1, {tchkNum},
          &PdfParser::opSetCharSpacing},
  {"Td",  2, {tchkNum,    tchkNum},
          &PdfParser::opTextMove},
  {"Tf",  2, {tchkName,   tchkNum},
          &PdfParser::opSetFont},
  {"Tj",  1, {tchkString},
          &PdfParser::opShowText},
  {"Tm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &PdfParser::opSetTextMatrix},
  {"Tr",  1, {tchkInt},
          &PdfParser::opSetTextRender},
  {"Ts",  1, {tchkNum},
          &PdfParser::opSetTextRise},
  {"Tw",  1, {tchkNum},
          &PdfParser::opSetWordSpacing},
  {"Tz",  1, {tchkNum},
          &PdfParser::opSetHorizScaling},
  {"W",   0, {tchkNone},
          &PdfParser::opClip},
  {"W*",  0, {tchkNone},
          &PdfParser::opEOClip},
  {"b",   0, {tchkNone},
          &PdfParser::opCloseFillStroke},
  {"b*",  0, {tchkNone},
          &PdfParser::opCloseEOFillStroke},
  {"c",   6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &PdfParser::opCurveTo},
  {"cm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &PdfParser::opConcat},
  {"cs",  1, {tchkName},
          &PdfParser::opSetFillColorSpace},
  {"d",   2, {tchkArray,  tchkNum},
          &PdfParser::opSetDash},
  {"d0",  2, {tchkNum,    tchkNum},
          &PdfParser::opSetCharWidth},
  {"d1",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &PdfParser::opSetCacheDevice},
  {"f",   0, {tchkNone},
          &PdfParser::opFill},
  {"f*",  0, {tchkNone},
          &PdfParser::opEOFill},
  {"g",   1, {tchkNum},
          &PdfParser::opSetFillGray},
  {"gs",  1, {tchkName},
          &PdfParser::opSetExtGState},
  {"h",   0, {tchkNone},
          &PdfParser::opClosePath},
  {"i",   1, {tchkNum},
          &PdfParser::opSetFlat},
  {"j",   1, {tchkInt},
          &PdfParser::opSetLineJoin},
  {"k",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opSetFillCMYKColor},
  {"l",   2, {tchkNum,    tchkNum},
          &PdfParser::opLineTo},
  {"m",   2, {tchkNum,    tchkNum},
          &PdfParser::opMoveTo},
  {"n",   0, {tchkNone},
          &PdfParser::opEndPath},
  {"q",   0, {tchkNone},
          &PdfParser::opSave},
  {"re",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opRectangle},
  {"rg",  3, {tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opSetFillRGBColor},
  {"ri",  1, {tchkName},
          &PdfParser::opSetRenderingIntent},
  {"s",   0, {tchkNone},
          &PdfParser::opCloseStroke},
  {"sc",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opSetFillColor},
  {"scn", -33, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN},
          &PdfParser::opSetFillColorN},
  {"sh",  1, {tchkName},
          &PdfParser::opShFill},
  {"v",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opCurveTo1},
  {"w",   1, {tchkNum},
          &PdfParser::opSetLineWidth},
  {"y",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &PdfParser::opCurveTo2}
};

#ifdef WIN32 // this works around a bug in the VC7 compiler
#  pragma optimize("",on)
#endif

#define numOps (sizeof(opTab) / sizeof(PdfOperator))

//------------------------------------------------------------------------
// PdfParser
//------------------------------------------------------------------------

PdfParser::PdfParser(XRef *xrefA, Inkscape::Extension::Internal::SvgBuilder *builderA,
                     int pageNum, int rotate, Dict *resDict,
                     PDFRectangle *box, PDFRectangle *cropBox) {

  int i;

  xref = xrefA;
  subPage = gFalse;
  printCommands = false;

  // start the resource stack
  res = new GfxResources(xref, resDict, NULL);

  // initialize
  state = new GfxState(72.0, 72.0, box, rotate, gTrue);
  clipHistory = new ClipHistoryEntry();
  setDefaultApproximationPrecision();
  fontChanged = gFalse;
  clip = clipNone;
  ignoreUndef = 0;
  operatorHistory = NULL;
  builder = builderA;
  builder->setDocumentSize(state->getPageWidth()*PX_PER_PT,
                           state->getPageHeight()*PX_PER_PT);

  double *ctm = state->getCTM();
  double scaledCTM[6];
  for (i = 0; i < 6; ++i) {
    baseMatrix[i] = ctm[i];
    scaledCTM[i] = PX_PER_PT * ctm[i];
  }
  saveState();
  builder->setTransform((double*)&scaledCTM);
  formDepth = 0;

  // set crop box
  if (cropBox) {
    if (printCommands)
        printf("cropBox: %f %f %f %f\n", cropBox->x1, cropBox->y1, cropBox->x2, cropBox->y2);
    // do not clip if it's not needed
    if (cropBox->x1 != 0.0 || cropBox->y1 != 0.0 ||
        cropBox->x2 != state->getPageWidth() || cropBox->y2 != state->getPageHeight()) {
        
        state->moveTo(cropBox->x1, cropBox->y1);
        state->lineTo(cropBox->x2, cropBox->y1);
        state->lineTo(cropBox->x2, cropBox->y2);
        state->lineTo(cropBox->x1, cropBox->y2);
        state->closePath();
        state->clip();
        clipHistory->setClip(state->getPath(), clipNormal);
        builder->setClipPath(state);
        state->clearPath();
    }
  }
  pushOperator("startPage");
}

PdfParser::PdfParser(XRef *xrefA, Inkscape::Extension::Internal::SvgBuilder *builderA,
                     Dict *resDict, PDFRectangle *box) {

  int i;

  xref = xrefA;
  subPage = gTrue;
  printCommands = false;

  // start the resource stack
  res = new GfxResources(xref, resDict, NULL);

  // initialize
  operatorHistory = NULL;
  builder = builderA;
  state = new GfxState(72, 72, box, 0, gFalse);
  clipHistory = new ClipHistoryEntry();
  setDefaultApproximationPrecision();
  
  fontChanged = gFalse;
  clip = clipNone;
  ignoreUndef = 0;
  for (i = 0; i < 6; ++i) {
    baseMatrix[i] = state->getCTM()[i];
  }
  formDepth = 0;
}

PdfParser::~PdfParser() {
  while (state->hasSaves()) {
    restoreState();
  }
  if (!subPage) {
    //out->endPage();
  }
  while (res) {
    popResources();
  }
  if (state) {
    delete state;
  }
  if (clipHistory) {
    delete clipHistory;
  }
}

void PdfParser::parse(Object *obj, GBool topLevel) {
  Object obj2;
  int i;

  if (obj->isArray()) {
    for (i = 0; i < obj->arrayGetLength(); ++i) {
      obj->arrayGet(i, &obj2);
      if (!obj2.isStream()) {
	error(-1, "Weird page contents");
	obj2.free();
	return;
      }
      obj2.free();
    }
  } else if (!obj->isStream()) {
    error(-1, "Weird page contents");
    return;
  }
  parser = new Parser(xref, new Lexer(xref, obj), gFalse);
  go(topLevel);
  delete parser;
  parser = NULL;
}

void PdfParser::go(GBool topLevel) {
  Object obj;
  Object args[maxArgs];
  int numArgs, i;
  int lastAbortCheck;

  // scan a sequence of objects
  numArgs = 0;
  parser->getObj(&obj);
  while (!obj.isEOF()) {

    // got a command - execute it
    if (obj.isCmd()) {
      if (printCommands) {
	obj.print(stdout);
	for (i = 0; i < numArgs; ++i) {
	  printf(" ");
	  args[i].print(stdout);
	}
	printf("\n");
	fflush(stdout);
      }

      // Run the operation
      execOp(&obj, args, numArgs);

      obj.free();
      for (i = 0; i < numArgs; ++i)
	args[i].free();
      numArgs = 0;

    // got an argument - save it
    } else if (numArgs < maxArgs) {
      args[numArgs++] = obj;

    // too many arguments - something is wrong
    } else {
      error(getPos(), "Too many args in content stream");
      if (printCommands) {
	printf("throwing away arg: ");
	obj.print(stdout);
	printf("\n");
	fflush(stdout);
      }
      obj.free();
    }

    // grab the next object
    parser->getObj(&obj);
  }
  obj.free();

  // args at end with no command
  if (numArgs > 0) {
    error(getPos(), "Leftover args in content stream");
    if (printCommands) {
      printf("%d leftovers:", numArgs);
      for (i = 0; i < numArgs; ++i) {
	printf(" ");
	args[i].print(stdout);
      }
      printf("\n");
      fflush(stdout);
    }
    for (i = 0; i < numArgs; ++i)
      args[i].free();
  }
}

void PdfParser::pushOperator(const char *name) {
    OpHistoryEntry *newEntry = new OpHistoryEntry;
    newEntry->name = name;
    newEntry->state = NULL;
    newEntry->depth = (operatorHistory != NULL ? (operatorHistory->depth+1) : 0);
    newEntry->next = operatorHistory;
    operatorHistory = newEntry;

    // Truncate list if needed
    if (operatorHistory->depth > maxOperatorHistoryDepth) {
        OpHistoryEntry *curr = operatorHistory;
        OpHistoryEntry *prev = NULL;
        while (curr && curr->next != NULL) {
            curr->depth--;
            prev = curr;
            curr = curr->next;
        }
        if (prev) {
            if (curr->state != NULL)
                delete curr->state;
            delete curr;
            prev->next = NULL;
        }
    }
}

const char *PdfParser::getPreviousOperator(unsigned int look_back) {
    OpHistoryEntry *prev = NULL;
    if (operatorHistory != NULL && look_back > 0) {
        prev = operatorHistory->next;
        while (--look_back > 0 && prev != NULL) {
            prev = prev->next;
        }
    }
    if (prev != NULL) {
        return prev->name;
    } else {
        return "";
    }
}

void PdfParser::execOp(Object *cmd, Object args[], int numArgs) {
  PdfOperator *op;
  char *name;
  Object *argPtr;
  int i;

  // find operator
  name = cmd->getCmd();
  if (!(op = findOp(name))) {
    if (ignoreUndef == 0)
      error(getPos(), "Unknown operator '%s'", name);
    return;
  }

  // type check args
  argPtr = args;
  if (op->numArgs >= 0) {
    if (numArgs < op->numArgs) {
      error(getPos(), "Too few (%d) args to '%s' operator", numArgs, name);
      return;
    }
    if (numArgs > op->numArgs) {
#if 0
      error(getPos(), "Too many (%d) args to '%s' operator", numArgs, name);
#endif
      argPtr += numArgs - op->numArgs;
      numArgs = op->numArgs;
    }
  } else {
    if (numArgs > -op->numArgs) {
      error(getPos(), "Too many (%d) args to '%s' operator",
	    numArgs, name);
      return;
    }
  }
  for (i = 0; i < numArgs; ++i) {
    if (!checkArg(&argPtr[i], op->tchk[i])) {
      error(getPos(), "Arg #%d to '%s' operator is wrong type (%s)",
	    i, name, argPtr[i].getTypeName());
      return;
    }
  }

  // add to history
  pushOperator((char*)&op->name);

  // do it
  (this->*op->func)(argPtr, numArgs);
}

PdfOperator *PdfParser::findOp(char *name) {
  int a, b, m, cmp;

  a = -1;
  b = numOps;
  // invariant: opTab[a] < name < opTab[b]
  while (b - a > 1) {
    m = (a + b) / 2;
    cmp = strcmp(opTab[m].name, name);
    if (cmp < 0)
      a = m;
    else if (cmp > 0)
      b = m;
    else
      a = b = m;
  }
  if (cmp != 0)
    return NULL;
  return &opTab[a];
}

GBool PdfParser::checkArg(Object *arg, TchkType type) {
  switch (type) {
  case tchkBool:   return arg->isBool();
  case tchkInt:    return arg->isInt();
  case tchkNum:    return arg->isNum();
  case tchkString: return arg->isString();
  case tchkName:   return arg->isName();
  case tchkArray:  return arg->isArray();
  case tchkProps:  return arg->isDict() || arg->isName();
  case tchkSCN:    return arg->isNum() || arg->isName();
  case tchkNone:   return gFalse;
  }
  return gFalse;
}

int PdfParser::getPos() {
  return parser ? parser->getPos() : -1;
}

//------------------------------------------------------------------------
// graphics state operators
//------------------------------------------------------------------------

void PdfParser::opSave(Object args[], int numArgs) {
  saveState();
}

void PdfParser::opRestore(Object args[], int numArgs) {
  restoreState();
}

void PdfParser::opConcat(Object args[], int numArgs) {
  state->concatCTM(args[0].getNum(), args[1].getNum(),
		   args[2].getNum(), args[3].getNum(),
		   args[4].getNum(), args[5].getNum());
  const char *prevOp = getPreviousOperator();
  double a0 = args[0].getNum();
  double a1 = args[1].getNum();
  double a2 = args[2].getNum();
  double a3 = args[3].getNum();
  double a4 = args[4].getNum();
  double a5 = args[5].getNum();
  if (!strcmp(prevOp, "q")) {
      builder->setTransform(a0, a1, a2, a3, a4, a5);
  } else if (!strcmp(prevOp, "cm") || !strcmp(prevOp, "startPage")) {
      // multiply it with the previous transform
      double otherMatrix[6];
      if (!builder->getTransform(otherMatrix)) { // invalid transform
          // construct identity matrix
          otherMatrix[0] = otherMatrix[3] = 1.0;
          otherMatrix[1] = otherMatrix[2] = otherMatrix[4] = otherMatrix[5] = 0.0;
      }
      double c0 = a0*otherMatrix[0] + a1*otherMatrix[2];
      double c1 = a0*otherMatrix[1] + a1*otherMatrix[3];
      double c2 = a2*otherMatrix[0] + a3*otherMatrix[2];
      double c3 = a2*otherMatrix[1] + a3*otherMatrix[3];
      double c4 = a4*otherMatrix[0] + a5*otherMatrix[2] + otherMatrix[4];
      double c5 = a4*otherMatrix[1] + a5*otherMatrix[3] + otherMatrix[5];
      builder->setTransform(c0, c1, c2, c3, c4, c5);
  } else {
      builder->pushGroup();
      builder->setTransform(a0, a1, a2, a3, a4, a5);
  }
  fontChanged = gTrue;
}

void PdfParser::opSetDash(Object args[], int numArgs) {
  Array *a;
  int length;
  Object obj;
  double *dash;
  int i;

  a = args[0].getArray();
  length = a->getLength();
  if (length == 0) {
    dash = NULL;
  } else {
    dash = (double *)gmallocn(length, sizeof(double));
    for (i = 0; i < length; ++i) {
      dash[i] = a->get(i, &obj)->getNum();
      obj.free();
    }
  }
  state->setLineDash(dash, length, args[1].getNum());
  builder->updateStyle(state);
}

void PdfParser::opSetFlat(Object args[], int numArgs) {
  state->setFlatness((int)args[0].getNum());
}

void PdfParser::opSetLineJoin(Object args[], int numArgs) {
  state->setLineJoin(args[0].getInt());
  builder->updateStyle(state);
}

void PdfParser::opSetLineCap(Object args[], int numArgs) {
  state->setLineCap(args[0].getInt());
  builder->updateStyle(state);
}

void PdfParser::opSetMiterLimit(Object args[], int numArgs) {
  state->setMiterLimit(args[0].getNum());
  builder->updateStyle(state);
}

void PdfParser::opSetLineWidth(Object args[], int numArgs) {
  state->setLineWidth(args[0].getNum());
  builder->updateStyle(state);
}

void PdfParser::opSetExtGState(Object args[], int numArgs) {
  Object obj1, obj2, obj3, obj4, obj5;
  GfxBlendMode mode;
  GBool haveFillOP;
  Function *funcs[4];
  GfxColor backdropColor;
  GBool haveBackdropColor;
  GfxColorSpace *blendingColorSpace;
  GBool alpha, isolated, knockout;
  int i;

  if (!res->lookupGState(args[0].getName(), &obj1)) {
    return;
  }
  if (!obj1.isDict()) {
    error(getPos(), "ExtGState '%s' is wrong type", args[0].getName());
    obj1.free();
    return;
  }
  if (printCommands) {
    printf("  gfx state dict: ");
    obj1.print();
    printf("\n");
  }

  // transparency support: blend mode, fill/stroke opacity
  if (!obj1.dictLookup("BM", &obj2)->isNull()) {
    if (state->parseBlendMode(&obj2, &mode)) {
      state->setBlendMode(mode);
    } else {
      error(getPos(), "Invalid blend mode in ExtGState");
    }
  }
  obj2.free();
  if (obj1.dictLookup("ca", &obj2)->isNum()) {
    state->setFillOpacity(obj2.getNum());
  }
  obj2.free();
  if (obj1.dictLookup("CA", &obj2)->isNum()) {
    state->setStrokeOpacity(obj2.getNum());
  }
  obj2.free();

  // fill/stroke overprint
  if ((haveFillOP = (obj1.dictLookup("op", &obj2)->isBool()))) {
    state->setFillOverprint(obj2.getBool());
  }
  obj2.free();
  if (obj1.dictLookup("OP", &obj2)->isBool()) {
    state->setStrokeOverprint(obj2.getBool());
    if (!haveFillOP) {
      state->setFillOverprint(obj2.getBool());
    }
  }
  obj2.free();

  // stroke adjust
  if (obj1.dictLookup("SA", &obj2)->isBool()) {
    state->setStrokeAdjust(obj2.getBool());
  }
  obj2.free();

  // transfer function
  if (obj1.dictLookup("TR2", &obj2)->isNull()) {
    obj2.free();
    obj1.dictLookup("TR", &obj2);
  }
  if (obj2.isName("Default") ||
      obj2.isName("Identity")) {
    funcs[0] = funcs[1] = funcs[2] = funcs[3] = NULL;
    state->setTransfer(funcs);
  } else if (obj2.isArray() && obj2.arrayGetLength() == 4) {
    for (i = 0; i < 4; ++i) {
      obj2.arrayGet(i, &obj3);
      funcs[i] = Function::parse(&obj3);
      obj3.free();
      if (!funcs[i]) {
	break;
      }
    }
    if (i == 4) {
      state->setTransfer(funcs);
    }
  } else if (obj2.isName() || obj2.isDict() || obj2.isStream()) {
    if ((funcs[0] = Function::parse(&obj2))) {
      funcs[1] = funcs[2] = funcs[3] = NULL;
      state->setTransfer(funcs);
    }
  } else if (!obj2.isNull()) {
    error(getPos(), "Invalid transfer function in ExtGState");
  }
  obj2.free();

  // soft mask
  if (!obj1.dictLookup("SMask", &obj2)->isNull()) {
    if (obj2.isName("None")) {
      builder->clearSoftMask(state);
    } else if (obj2.isDict()) {
      if (obj2.dictLookup("S", &obj3)->isName("Alpha")) {
	alpha = gTrue;
      } else { // "Luminosity"
	alpha = gFalse;
      }
      obj3.free();
      funcs[0] = NULL;
      if (!obj2.dictLookup("TR", &obj3)->isNull()) {
	funcs[0] = Function::parse(&obj3);
	if (funcs[0]->getInputSize() != 1 ||
	    funcs[0]->getOutputSize() != 1) {
	  error(getPos(),
		"Invalid transfer function in soft mask in ExtGState");
	  delete funcs[0];
	  funcs[0] = NULL;
	}
      }
      obj3.free();
      if ((haveBackdropColor = obj2.dictLookup("BC", &obj3)->isArray())) {
	for (i = 0; i < gfxColorMaxComps; ++i) {
	  backdropColor.c[i] = 0;
	}
	for (i = 0; i < obj3.arrayGetLength() && i < gfxColorMaxComps; ++i) {
	  obj3.arrayGet(i, &obj4);
	  if (obj4.isNum()) {
	    backdropColor.c[i] = dblToCol(obj4.getNum());
	  }
	  obj4.free();
	}
      }
      obj3.free();
      if (obj2.dictLookup("G", &obj3)->isStream()) {
	if (obj3.streamGetDict()->lookup("Group", &obj4)->isDict()) {
	  blendingColorSpace = NULL;
	  isolated = knockout = gFalse;
	  if (!obj4.dictLookup("CS", &obj5)->isNull()) {
	    blendingColorSpace = GfxColorSpace::parse(&obj5);
	  }
	  obj5.free();
	  if (obj4.dictLookup("I", &obj5)->isBool()) {
	    isolated = obj5.getBool();
	  }
	  obj5.free();
	  if (obj4.dictLookup("K", &obj5)->isBool()) {
	    knockout = obj5.getBool();
	  }
	  obj5.free();
	  if (!haveBackdropColor) {
	    if (blendingColorSpace) {
	      blendingColorSpace->getDefaultColor(&backdropColor);
	    } else {
	      //~ need to get the parent or default color space (?)
	      for (i = 0; i < gfxColorMaxComps; ++i) {
		backdropColor.c[i] = 0;
	      }
	    }
	  }
	  doSoftMask(&obj3, alpha, blendingColorSpace,
		     isolated, knockout, funcs[0], &backdropColor);
	  if (funcs[0]) {
	    delete funcs[0];
	  }
	} else {
	  error(getPos(), "Invalid soft mask in ExtGState - missing group");
	}
	obj4.free();
      } else {
	error(getPos(), "Invalid soft mask in ExtGState - missing group");
      }
      obj3.free();
    } else if (!obj2.isNull()) {
      error(getPos(), "Invalid soft mask in ExtGState");
    }
  }
  obj2.free();

  obj1.free();
}

void PdfParser::doSoftMask(Object *str, GBool alpha,
		     GfxColorSpace *blendingColorSpace,
		     GBool isolated, GBool knockout,
		     Function *transferFunc, GfxColor *backdropColor) {
  Dict *dict, *resDict;
  double m[6], bbox[4];
  Object obj1, obj2;
  int i;

  // check for excessive recursion
  if (formDepth > 20) {
    return;
  }

  // get stream dict
  dict = str->streamGetDict();

  // check form type
  dict->lookup("FormType", &obj1);
  if (!(obj1.isNull() || (obj1.isInt() && obj1.getInt() == 1))) {
    error(getPos(), "Unknown form type");
  }
  obj1.free();

  // get bounding box
  dict->lookup("BBox", &obj1);
  if (!obj1.isArray()) {
    obj1.free();
    error(getPos(), "Bad form bounding box");
    return;
  }
  for (i = 0; i < 4; ++i) {
    obj1.arrayGet(i, &obj2);
    bbox[i] = obj2.getNum();
    obj2.free();
  }
  obj1.free();

  // get matrix
  dict->lookup("Matrix", &obj1);
  if (obj1.isArray()) {
    for (i = 0; i < 6; ++i) {
      obj1.arrayGet(i, &obj2);
      m[i] = obj2.getNum();
      obj2.free();
    }
  } else {
    m[0] = 1; m[1] = 0;
    m[2] = 0; m[3] = 1;
    m[4] = 0; m[5] = 0;
  }
  obj1.free();

  // get resources
  dict->lookup("Resources", &obj1);
  resDict = obj1.isDict() ? obj1.getDict() : (Dict *)NULL;

  // draw it
  ++formDepth;
  doForm1(str, resDict, m, bbox, gTrue, gTrue,
	  blendingColorSpace, isolated, knockout,
	  alpha, transferFunc, backdropColor);
  --formDepth;

  if (blendingColorSpace) {
    delete blendingColorSpace;
  }
  obj1.free();
}

void PdfParser::opSetRenderingIntent(Object args[], int numArgs) {
}

//------------------------------------------------------------------------
// color operators
//------------------------------------------------------------------------

void PdfParser::opSetFillGray(Object args[], int numArgs) {
  GfxColor color;

  state->setFillPattern(NULL);
  state->setFillColorSpace(new GfxDeviceGrayColorSpace());
  color.c[0] = dblToCol(args[0].getNum());
  state->setFillColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetStrokeGray(Object args[], int numArgs) {
  GfxColor color;

  state->setStrokePattern(NULL);
  state->setStrokeColorSpace(new GfxDeviceGrayColorSpace());
  color.c[0] = dblToCol(args[0].getNum());
  state->setStrokeColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetFillCMYKColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  state->setFillPattern(NULL);
  state->setFillColorSpace(new GfxDeviceCMYKColorSpace());
  for (i = 0; i < 4; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setFillColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetStrokeCMYKColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  state->setStrokePattern(NULL);
  state->setStrokeColorSpace(new GfxDeviceCMYKColorSpace());
  for (i = 0; i < 4; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setStrokeColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetFillRGBColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  state->setFillPattern(NULL);
  state->setFillColorSpace(new GfxDeviceRGBColorSpace());
  for (i = 0; i < 3; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setFillColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetStrokeRGBColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  state->setStrokePattern(NULL);
  state->setStrokeColorSpace(new GfxDeviceRGBColorSpace());
  for (i = 0; i < 3; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setStrokeColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetFillColorSpace(Object args[], int numArgs) {
  Object obj;
  GfxColorSpace *colorSpace;
  GfxColor color;

  state->setFillPattern(NULL);
  res->lookupColorSpace(args[0].getName(), &obj);
  if (obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&args[0]);
  } else {
    colorSpace = GfxColorSpace::parse(&obj);
  }
  obj.free();
  if (colorSpace) {
    state->setFillColorSpace(colorSpace);
    colorSpace->getDefaultColor(&color);
    state->setFillColor(&color);
    builder->updateStyle(state);
  } else {
    error(getPos(), "Bad color space (fill)");
  }
}

void PdfParser::opSetStrokeColorSpace(Object args[], int numArgs) {
  Object obj;
  GfxColorSpace *colorSpace;
  GfxColor color;

  state->setStrokePattern(NULL);
  res->lookupColorSpace(args[0].getName(), &obj);
  if (obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&args[0]);
  } else {
    colorSpace = GfxColorSpace::parse(&obj);
  }
  obj.free();
  if (colorSpace) {
    state->setStrokeColorSpace(colorSpace);
    colorSpace->getDefaultColor(&color);
    state->setStrokeColor(&color);
    builder->updateStyle(state);
  } else {
    error(getPos(), "Bad color space (stroke)");
  }
}

void PdfParser::opSetFillColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  if (numArgs != state->getFillColorSpace()->getNComps()) {
    error(getPos(), "Incorrect number of arguments in 'sc' command");
    return;
  }
  state->setFillPattern(NULL);
  for (i = 0; i < numArgs; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setFillColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetStrokeColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  if (numArgs != state->getStrokeColorSpace()->getNComps()) {
    error(getPos(), "Incorrect number of arguments in 'SC' command");
    return;
  }
  state->setStrokePattern(NULL);
  for (i = 0; i < numArgs; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setStrokeColor(&color);
  builder->updateStyle(state);
}

void PdfParser::opSetFillColorN(Object args[], int numArgs) {
  GfxColor color;
  GfxPattern *pattern;
  int i;

  if (state->getFillColorSpace()->getMode() == csPattern) {
    if (numArgs > 1) {
      if (!((GfxPatternColorSpace *)state->getFillColorSpace())->getUnder() ||
	  numArgs - 1 != ((GfxPatternColorSpace *)state->getFillColorSpace())
	                     ->getUnder()->getNComps()) {
	error(getPos(), "Incorrect number of arguments in 'scn' command");
	return;
      }
      for (i = 0; i < numArgs - 1 && i < gfxColorMaxComps; ++i) {
	if (args[i].isNum()) {
	  color.c[i] = dblToCol(args[i].getNum());
	}
      }
      state->setFillColor(&color);
      builder->updateStyle(state);
    }
    if (args[numArgs-1].isName() &&
	(pattern = res->lookupPattern(args[numArgs-1].getName()))) {
      state->setFillPattern(pattern);
      builder->updateStyle(state);
    }

  } else {
    if (numArgs != state->getFillColorSpace()->getNComps()) {
      error(getPos(), "Incorrect number of arguments in 'scn' command");
      return;
    }
    state->setFillPattern(NULL);
    for (i = 0; i < numArgs && i < gfxColorMaxComps; ++i) {
      if (args[i].isNum()) {
	color.c[i] = dblToCol(args[i].getNum());
      }
    }
    state->setFillColor(&color);
    builder->updateStyle(state);
  }
}

void PdfParser::opSetStrokeColorN(Object args[], int numArgs) {
  GfxColor color;
  GfxPattern *pattern;
  int i;

  if (state->getStrokeColorSpace()->getMode() == csPattern) {
    if (numArgs > 1) {
      if (!((GfxPatternColorSpace *)state->getStrokeColorSpace())
	       ->getUnder() ||
	  numArgs - 1 != ((GfxPatternColorSpace *)state->getStrokeColorSpace())
	                     ->getUnder()->getNComps()) {
	error(getPos(), "Incorrect number of arguments in 'SCN' command");
	return;
      }
      for (i = 0; i < numArgs - 1 && i < gfxColorMaxComps; ++i) {
	if (args[i].isNum()) {
	  color.c[i] = dblToCol(args[i].getNum());
	}
      }
      state->setStrokeColor(&color);
      builder->updateStyle(state);
    }
    if (args[numArgs-1].isName() &&
	(pattern = res->lookupPattern(args[numArgs-1].getName()))) {
      state->setStrokePattern(pattern);
      builder->updateStyle(state);
    }

  } else {
    if (numArgs != state->getStrokeColorSpace()->getNComps()) {
      error(getPos(), "Incorrect number of arguments in 'SCN' command");
      return;
    }
    state->setStrokePattern(NULL);
    for (i = 0; i < numArgs && i < gfxColorMaxComps; ++i) {
      if (args[i].isNum()) {
	color.c[i] = dblToCol(args[i].getNum());
      }
    }
    state->setStrokeColor(&color);
    builder->updateStyle(state);
  }
}

//------------------------------------------------------------------------
// path segment operators
//------------------------------------------------------------------------

void PdfParser::opMoveTo(Object args[], int numArgs) {
  state->moveTo(args[0].getNum(), args[1].getNum());
}

void PdfParser::opLineTo(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    error(getPos(), "No current point in lineto");
    return;
  }
  state->lineTo(args[0].getNum(), args[1].getNum());
}

void PdfParser::opCurveTo(Object args[], int numArgs) {
  double x1, y1, x2, y2, x3, y3;

  if (!state->isCurPt()) {
    error(getPos(), "No current point in curveto");
    return;
  }
  x1 = args[0].getNum();
  y1 = args[1].getNum();
  x2 = args[2].getNum();
  y2 = args[3].getNum();
  x3 = args[4].getNum();
  y3 = args[5].getNum();
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void PdfParser::opCurveTo1(Object args[], int numArgs) {
  double x1, y1, x2, y2, x3, y3;

  if (!state->isCurPt()) {
    error(getPos(), "No current point in curveto1");
    return;
  }
  x1 = state->getCurX();
  y1 = state->getCurY();
  x2 = args[0].getNum();
  y2 = args[1].getNum();
  x3 = args[2].getNum();
  y3 = args[3].getNum();
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void PdfParser::opCurveTo2(Object args[], int numArgs) {
  double x1, y1, x2, y2, x3, y3;

  if (!state->isCurPt()) {
    error(getPos(), "No current point in curveto2");
    return;
  }
  x1 = args[0].getNum();
  y1 = args[1].getNum();
  x2 = args[2].getNum();
  y2 = args[3].getNum();
  x3 = x2;
  y3 = y2;
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void PdfParser::opRectangle(Object args[], int numArgs) {
  double x, y, w, h;

  x = args[0].getNum();
  y = args[1].getNum();
  w = args[2].getNum();
  h = args[3].getNum();
  state->moveTo(x, y);
  state->lineTo(x + w, y);
  state->lineTo(x + w, y + h);
  state->lineTo(x, y + h);
  state->closePath();
}

void PdfParser::opClosePath(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    error(getPos(), "No current point in closepath");
    return;
  }
  state->closePath();
}

//------------------------------------------------------------------------
// path painting operators
//------------------------------------------------------------------------

void PdfParser::opEndPath(Object args[], int numArgs) {
  doEndPath();
}

void PdfParser::opStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in stroke");
    return;
  }
  if (state->isPath()) {
    if (state->getStrokeColorSpace()->getMode() == csPattern &&
        !builder->isPatternTypeSupported(state->getStrokePattern())) {
          doPatternStrokeFallback();
    } else {
      builder->addPath(state, false, true);
    }
  }
  doEndPath();
}

void PdfParser::opCloseStroke(Object * /*args[]*/, int /*numArgs*/) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in closepath/stroke");
    return;
  }
  state->closePath();
  if (state->isPath()) {
    if (state->getStrokeColorSpace()->getMode() == csPattern &&
        !builder->isPatternTypeSupported(state->getStrokePattern())) {
      doPatternStrokeFallback();
    } else {
      builder->addPath(state, false, true);
    }
  }
  doEndPath();
}

void PdfParser::opFill(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in fill");
    return;
  }
  if (state->isPath()) {
    if (state->getFillColorSpace()->getMode() == csPattern &&
        !builder->isPatternTypeSupported(state->getFillPattern())) {
      doPatternFillFallback(gFalse);
    } else {
      builder->addPath(state, true, false);
    }
  }
  doEndPath();
}

void PdfParser::opEOFill(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in eofill");
    return;
  }
  if (state->isPath()) {
    if (state->getFillColorSpace()->getMode() == csPattern &&
        !builder->isPatternTypeSupported(state->getFillPattern())) {
      doPatternFillFallback(gTrue);
    } else {
      builder->addPath(state, true, false, true);
    }
  }
  doEndPath();
}

void PdfParser::opFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in fill/stroke");
    return;
  }
  if (state->isPath()) {
    doFillAndStroke(gFalse);
  } else {
    builder->addPath(state, true, true);
  }
  doEndPath();
}

void PdfParser::opCloseFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in closepath/fill/stroke");
    return;
  }
  if (state->isPath()) {
    state->closePath();
    doFillAndStroke(gFalse);
  }
  doEndPath();
}

void PdfParser::opEOFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in eofill/stroke");
    return;
  }
  if (state->isPath()) {
    doFillAndStroke(gTrue);
  }
  doEndPath();
}

void PdfParser::opCloseEOFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(getPos(), "No path in closepath/eofill/stroke");
    return;
  }
  if (state->isPath()) {
    state->closePath();
    doFillAndStroke(gTrue);
  }
  doEndPath();
}

void PdfParser::doFillAndStroke(GBool eoFill) {
    GBool fillOk = gTrue, strokeOk = gTrue;
    if (state->getFillColorSpace()->getMode() == csPattern &&
        !builder->isPatternTypeSupported(state->getFillPattern())) {
        fillOk = gFalse;
    }
    if (state->getStrokeColorSpace()->getMode() == csPattern &&
        !builder->isPatternTypeSupported(state->getStrokePattern())) {
        strokeOk = gFalse;
    }
    if (fillOk && strokeOk) {
        builder->addPath(state, true, true, eoFill);
    } else {
        doPatternFillFallback(eoFill);
        doPatternStrokeFallback();
    }
}

void PdfParser::doPatternFillFallback(GBool eoFill) {
  GfxPattern *pattern;

  if (!(pattern = state->getFillPattern())) {
    return;
  }
  switch (pattern->getType()) {
  case 1:
    break;
  case 2:
    doShadingPatternFillFallback((GfxShadingPattern *)pattern, gFalse, eoFill);
    break;
  default:
    error(getPos(), "Unimplemented pattern type (%d) in fill",
	  pattern->getType());
    break;
  }
}

void PdfParser::doPatternStrokeFallback() {
  GfxPattern *pattern;

  if (!(pattern = state->getStrokePattern())) {
    return;
  }
  switch (pattern->getType()) {
  case 1:
    break;
  case 2:
    doShadingPatternFillFallback((GfxShadingPattern *)pattern, gTrue, gFalse);
    break;
  default:
    error(getPos(), "Unimplemented pattern type (%d) in stroke",
	  pattern->getType());
    break;
  }
}

void PdfParser::doShadingPatternFillFallback(GfxShadingPattern *sPat,
                                             GBool stroke, GBool eoFill) {
  GfxShading *shading;
  GfxPath *savedPath;
  double *ctm, *btm, *ptm;
  double m[6], ictm[6], m1[6];
  double xMin, yMin, xMax, yMax;
  double det;

  shading = sPat->getShading();

  // save current graphics state
  savedPath = state->getPath()->copy();
  saveState();

  // clip to bbox
  if (0 ){//shading->getHasBBox()) {
    shading->getBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMax, yMin);
    state->lineTo(xMax, yMax);
    state->lineTo(xMin, yMax);
    state->closePath();
    state->clip();
    //builder->clip(state);
    state->setPath(savedPath->copy());
  }

  // clip to current path
  if (stroke) {
    state->clipToStrokePath();
    //out->clipToStrokePath(state);
  } else {
    state->clip();
    if (eoFill) {
      builder->setClipPath(state, true);
    } else {
      builder->setClipPath(state);
    }
  }

  // set the color space
  state->setFillColorSpace(shading->getColorSpace()->copy());

  // background color fill
  if (shading->getHasBackground()) {
    state->setFillColor(shading->getBackground());
    builder->addPath(state, true, false);
  }
  state->clearPath();

  // construct a (pattern space) -> (current space) transform matrix
  ctm = state->getCTM();
  btm = baseMatrix;
  ptm = sPat->getMatrix();
  // iCTM = invert CTM
  det = 1 / (ctm[0] * ctm[3] - ctm[1] * ctm[2]);
  ictm[0] = ctm[3] * det;
  ictm[1] = -ctm[1] * det;
  ictm[2] = -ctm[2] * det;
  ictm[3] = ctm[0] * det;
  ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
  ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;
  // m1 = PTM * BTM = PTM * base transform matrix
  m1[0] = ptm[0] * btm[0] + ptm[1] * btm[2];
  m1[1] = ptm[0] * btm[1] + ptm[1] * btm[3];
  m1[2] = ptm[2] * btm[0] + ptm[3] * btm[2];
  m1[3] = ptm[2] * btm[1] + ptm[3] * btm[3];
  m1[4] = ptm[4] * btm[0] + ptm[5] * btm[2] + btm[4];
  m1[5] = ptm[4] * btm[1] + ptm[5] * btm[3] + btm[5];
  // m = m1 * iCTM = (PTM * BTM) * (iCTM)
  m[0] = m1[0] * ictm[0] + m1[1] * ictm[2];
  m[1] = m1[0] * ictm[1] + m1[1] * ictm[3];
  m[2] = m1[2] * ictm[0] + m1[3] * ictm[2];
  m[3] = m1[2] * ictm[1] + m1[3] * ictm[3];
  m[4] = m1[4] * ictm[0] + m1[5] * ictm[2] + ictm[4];
  m[5] = m1[4] * ictm[1] + m1[5] * ictm[3] + ictm[5];

  // set the new matrix
  state->concatCTM(m[0], m[1], m[2], m[3], m[4], m[5]);
  builder->setTransform(m[0], m[1], m[2], m[3], m[4], m[5]);

  // do shading type-specific operations
  switch (shading->getType()) {
  case 1:
    doFunctionShFill((GfxFunctionShading *)shading);
    break;
  case 2:
  case 3:
    // no need to implement these
    break;
  case 4:
  case 5:
    doGouraudTriangleShFill((GfxGouraudTriangleShading *)shading);
    break;
  case 6:
  case 7:
    doPatchMeshShFill((GfxPatchMeshShading *)shading);
    break;
  }

  // restore graphics state
  restoreState();
  state->setPath(savedPath);
}

void PdfParser::opShFill(Object args[], int numArgs) {
  GfxShading *shading;
  GfxPath *savedPath;
  double xMin, yMin, xMax, yMax;
  double gradientTransform[6];
  double *matrix = NULL;
  GBool savedState = gFalse;

  if (!(shading = res->lookupShading(args[0].getName()))) {
    return;
  }

  // save current graphics state
  if (shading->getType() != 2 && shading->getType() != 3) {
    savedPath = state->getPath()->copy();
    saveState();
    savedState = gTrue;
  } else {  // get gradient transform if possible
      // check proper operator sequence
      // first there should be one W(*) and then one 'cm' somewhere before 'sh'
      GBool seenClip, seenConcat;
      seenClip = (clipHistory->getClipPath() != NULL);
      seenConcat = gFalse;
      int i = 1;
      while (i <= maxOperatorHistoryDepth) {
        const char *opName = getPreviousOperator(i);
        if (!strcmp(opName, "cm")) {
          if (seenConcat) {   // more than one 'cm'
            break;
          } else {
            seenConcat = gTrue;
          }
        }
        i++;
      }

      if (seenConcat && seenClip) {
        if (builder->getTransform(gradientTransform)) {
          matrix = (double*)&gradientTransform;
          builder->setTransform(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);  // remove transform
        }
      }
  }

  // clip to bbox
  if (shading->getHasBBox()) {
    shading->getBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMax, yMin);
    state->lineTo(xMax, yMax);
    state->lineTo(xMin, yMax);
    state->closePath();
    state->clip();
    if (savedState)
      builder->setClipPath(state);
    else
      builder->clip(state);
    state->clearPath();
  }

  // set the color space
  if (savedState)
    state->setFillColorSpace(shading->getColorSpace()->copy());

  // do shading type-specific operations
  switch (shading->getType()) {
  case 1:
    doFunctionShFill((GfxFunctionShading *)shading);
    break;
  case 2:
  case 3:
    if (clipHistory->getClipPath()) {
      builder->addShadedFill(shading, matrix, clipHistory->getClipPath(),
                             clipHistory->getClipType() == clipEO ? true : false);
    }
    break;
  case 4:
  case 5:
    doGouraudTriangleShFill((GfxGouraudTriangleShading *)shading);
    break;
  case 6:
  case 7:
    doPatchMeshShFill((GfxPatchMeshShading *)shading);
    break;
  }

  // restore graphics state
  if (savedState) {
    restoreState();
    state->setPath(savedPath);
  }

  delete shading;
}

void PdfParser::doFunctionShFill(GfxFunctionShading *shading) {
  double x0, y0, x1, y1;
  GfxColor colors[4];

  shading->getDomain(&x0, &y0, &x1, &y1);
  shading->getColor(x0, y0, &colors[0]);
  shading->getColor(x0, y1, &colors[1]);
  shading->getColor(x1, y0, &colors[2]);
  shading->getColor(x1, y1, &colors[3]);
  doFunctionShFill1(shading, x0, y0, x1, y1, colors, 0);
}

void PdfParser::doFunctionShFill1(GfxFunctionShading *shading,
			    double x0, double y0,
			    double x1, double y1,
			    GfxColor *colors, int depth) {
  GfxColor fillColor;
  GfxColor color0M, color1M, colorM0, colorM1, colorMM;
  GfxColor colors2[4];
  double functionColorDelta = colorDeltas[pdfFunctionShading-1];
  double *matrix;
  double xM, yM;
  int nComps, i, j;

  nComps = shading->getColorSpace()->getNComps();
  matrix = shading->getMatrix();

  // compare the four corner colors
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < nComps; ++j) {
      if (abs(colors[i].c[j] - colors[(i+1)&3].c[j]) > functionColorDelta) {
	break;
      }
    }
    if (j < nComps) {
      break;
    }
  }

  // center of the rectangle
  xM = 0.5 * (x0 + x1);
  yM = 0.5 * (y0 + y1);

  // the four corner colors are close (or we hit the recursive limit)
  // -- fill the rectangle; but require at least one subdivision
  // (depth==0) to avoid problems when the four outer corners of the
  // shaded region are the same color
  if ((i == 4 && depth > 0) || depth == maxDepths[pdfFunctionShading-1]) {

    // use the center color
    shading->getColor(xM, yM, &fillColor);
    state->setFillColor(&fillColor);

    // fill the rectangle
    state->moveTo(x0 * matrix[0] + y0 * matrix[2] + matrix[4],
		  x0 * matrix[1] + y0 * matrix[3] + matrix[5]);
    state->lineTo(x1 * matrix[0] + y0 * matrix[2] + matrix[4],
		  x1 * matrix[1] + y0 * matrix[3] + matrix[5]);
    state->lineTo(x1 * matrix[0] + y1 * matrix[2] + matrix[4],
		  x1 * matrix[1] + y1 * matrix[3] + matrix[5]);
    state->lineTo(x0 * matrix[0] + y1 * matrix[2] + matrix[4],
		  x0 * matrix[1] + y1 * matrix[3] + matrix[5]);
    state->closePath();
    builder->addPath(state, true, false);
    state->clearPath();

  // the four corner colors are not close enough -- subdivide the
  // rectangle
  } else {

    // colors[0]       colorM0       colors[2]
    //   (x0,y0)       (xM,y0)       (x1,y0)
    //         +----------+----------+
    //         |          |          |
    //         |    UL    |    UR    |
    // color0M |       colorMM       | color1M
    // (x0,yM) +----------+----------+ (x1,yM)
    //         |       (xM,yM)       |
    //         |    LL    |    LR    |
    //         |          |          |
    //         +----------+----------+
    // colors[1]       colorM1       colors[3]
    //   (x0,y1)       (xM,y1)       (x1,y1)

    shading->getColor(x0, yM, &color0M);
    shading->getColor(x1, yM, &color1M);
    shading->getColor(xM, y0, &colorM0);
    shading->getColor(xM, y1, &colorM1);
    shading->getColor(xM, yM, &colorMM);

    // upper-left sub-rectangle
    colors2[0] = colors[0];
    colors2[1] = color0M;
    colors2[2] = colorM0;
    colors2[3] = colorMM;
    doFunctionShFill1(shading, x0, y0, xM, yM, colors2, depth + 1);
    
    // lower-left sub-rectangle
    colors2[0] = color0M;
    colors2[1] = colors[1];
    colors2[2] = colorMM;
    colors2[3] = colorM1;
    doFunctionShFill1(shading, x0, yM, xM, y1, colors2, depth + 1);
    
    // upper-right sub-rectangle
    colors2[0] = colorM0;
    colors2[1] = colorMM;
    colors2[2] = colors[2];
    colors2[3] = color1M;
    doFunctionShFill1(shading, xM, y0, x1, yM, colors2, depth + 1);

    // lower-right sub-rectangle
    colors2[0] = colorMM;
    colors2[1] = colorM1;
    colors2[2] = color1M;
    colors2[3] = colors[3];
    doFunctionShFill1(shading, xM, yM, x1, y1, colors2, depth + 1);
  }
}

void PdfParser::doGouraudTriangleShFill(GfxGouraudTriangleShading *shading) {
  double x0, y0, x1, y1, x2, y2;
  GfxColor color0, color1, color2;
  int i;

  for (i = 0; i < shading->getNTriangles(); ++i) {
    shading->getTriangle(i, &x0, &y0, &color0,
			 &x1, &y1, &color1,
			 &x2, &y2, &color2);
    gouraudFillTriangle(x0, y0, &color0, x1, y1, &color1, x2, y2, &color2,
			shading->getColorSpace()->getNComps(), 0);
  }
}

void PdfParser::gouraudFillTriangle(double x0, double y0, GfxColor *color0,
			      double x1, double y1, GfxColor *color1,
			      double x2, double y2, GfxColor *color2,
			      int nComps, int depth) {
  double x01, y01, x12, y12, x20, y20;
  double gouraudColorDelta = colorDeltas[pdfGouraudTriangleShading-1];
  GfxColor color01, color12, color20;
  int i;

  for (i = 0; i < nComps; ++i) {
    if (abs(color0->c[i] - color1->c[i]) > gouraudColorDelta ||
       abs(color1->c[i] - color2->c[i]) > gouraudColorDelta) {
      break;
    }
  }
  if (i == nComps || depth == maxDepths[pdfGouraudTriangleShading-1]) {
    state->setFillColor(color0);
    state->moveTo(x0, y0);
    state->lineTo(x1, y1);
    state->lineTo(x2, y2);
    state->closePath();
    builder->addPath(state, true, false);
    state->clearPath();
  } else {
    x01 = 0.5 * (x0 + x1);
    y01 = 0.5 * (y0 + y1);
    x12 = 0.5 * (x1 + x2);
    y12 = 0.5 * (y1 + y2);
    x20 = 0.5 * (x2 + x0);
    y20 = 0.5 * (y2 + y0);
    //~ if the shading has a Function, this should interpolate on the
    //~ function parameter, not on the color components
    for (i = 0; i < nComps; ++i) {
      color01.c[i] = (color0->c[i] + color1->c[i]) / 2;
      color12.c[i] = (color1->c[i] + color2->c[i]) / 2;
      color20.c[i] = (color2->c[i] + color0->c[i]) / 2;
    }
    gouraudFillTriangle(x0, y0, color0, x01, y01, &color01,
			x20, y20, &color20, nComps, depth + 1);
    gouraudFillTriangle(x01, y01, &color01, x1, y1, color1,
			x12, y12, &color12, nComps, depth + 1);
    gouraudFillTriangle(x01, y01, &color01, x12, y12, &color12,
			x20, y20, &color20, nComps, depth + 1);
    gouraudFillTriangle(x20, y20, &color20, x12, y12, &color12,
			x2, y2, color2, nComps, depth + 1);
  }
}

void PdfParser::doPatchMeshShFill(GfxPatchMeshShading *shading) {
  int start, i;

  if (shading->getNPatches() > 128) {
    start = 3;
  } else if (shading->getNPatches() > 64) {
    start = 2;
  } else if (shading->getNPatches() > 16) {
    start = 1;
  } else {
    start = 0;
  }
  for (i = 0; i < shading->getNPatches(); ++i) {
    fillPatch(shading->getPatch(i), shading->getColorSpace()->getNComps(),
	      start);
  }
}

void PdfParser::fillPatch(GfxPatch *patch, int nComps, int depth) {
  GfxPatch patch00, patch01, patch10, patch11;
  double xx[4][8], yy[4][8];
  double xxm, yym;
  double patchColorDelta = colorDeltas[pdfPatchMeshShading-1];
  int i;

  for (i = 0; i < nComps; ++i) {
    if (abs(patch->color[0][0].c[i] - patch->color[0][1].c[i])
	  > patchColorDelta ||
	abs(patch->color[0][1].c[i] - patch->color[1][1].c[i])
	  > patchColorDelta ||
	abs(patch->color[1][1].c[i] - patch->color[1][0].c[i])
	  > patchColorDelta ||
	abs(patch->color[1][0].c[i] - patch->color[0][0].c[i])
	  > patchColorDelta) {
      break;
    }
  }
  if (i == nComps || depth == maxDepths[pdfPatchMeshShading-1]) {
    state->setFillColor(&patch->color[0][0]);
    state->moveTo(patch->x[0][0], patch->y[0][0]);
    state->curveTo(patch->x[0][1], patch->y[0][1],
		   patch->x[0][2], patch->y[0][2],
		   patch->x[0][3], patch->y[0][3]);
    state->curveTo(patch->x[1][3], patch->y[1][3],
		   patch->x[2][3], patch->y[2][3],
		   patch->x[3][3], patch->y[3][3]);
    state->curveTo(patch->x[3][2], patch->y[3][2],
		   patch->x[3][1], patch->y[3][1],
		   patch->x[3][0], patch->y[3][0]);
    state->curveTo(patch->x[2][0], patch->y[2][0],
		   patch->x[1][0], patch->y[1][0],
		   patch->x[0][0], patch->y[0][0]);
    state->closePath();
    builder->addPath(state, true, false);
    state->clearPath();
  } else {
    for (i = 0; i < 4; ++i) {
      xx[i][0] = patch->x[i][0];
      yy[i][0] = patch->y[i][0];
      xx[i][1] = 0.5 * (patch->x[i][0] + patch->x[i][1]);
      yy[i][1] = 0.5 * (patch->y[i][0] + patch->y[i][1]);
      xxm = 0.5 * (patch->x[i][1] + patch->x[i][2]);
      yym = 0.5 * (patch->y[i][1] + patch->y[i][2]);
      xx[i][6] = 0.5 * (patch->x[i][2] + patch->x[i][3]);
      yy[i][6] = 0.5 * (patch->y[i][2] + patch->y[i][3]);
      xx[i][2] = 0.5 * (xx[i][1] + xxm);
      yy[i][2] = 0.5 * (yy[i][1] + yym);
      xx[i][5] = 0.5 * (xxm + xx[i][6]);
      yy[i][5] = 0.5 * (yym + yy[i][6]);
      xx[i][3] = xx[i][4] = 0.5 * (xx[i][2] + xx[i][5]);
      yy[i][3] = yy[i][4] = 0.5 * (yy[i][2] + yy[i][5]);
      xx[i][7] = patch->x[i][3];
      yy[i][7] = patch->y[i][3];
    }
    for (i = 0; i < 4; ++i) {
      patch00.x[0][i] = xx[0][i];
      patch00.y[0][i] = yy[0][i];
      patch00.x[1][i] = 0.5 * (xx[0][i] + xx[1][i]);
      patch00.y[1][i] = 0.5 * (yy[0][i] + yy[1][i]);
      xxm = 0.5 * (xx[1][i] + xx[2][i]);
      yym = 0.5 * (yy[1][i] + yy[2][i]);
      patch10.x[2][i] = 0.5 * (xx[2][i] + xx[3][i]);
      patch10.y[2][i] = 0.5 * (yy[2][i] + yy[3][i]);
      patch00.x[2][i] = 0.5 * (patch00.x[1][i] + xxm);
      patch00.y[2][i] = 0.5 * (patch00.y[1][i] + yym);
      patch10.x[1][i] = 0.5 * (xxm + patch10.x[2][i]);
      patch10.y[1][i] = 0.5 * (yym + patch10.y[2][i]);
      patch00.x[3][i] = 0.5 * (patch00.x[2][i] + patch10.x[1][i]);
      patch00.y[3][i] = 0.5 * (patch00.y[2][i] + patch10.y[1][i]);
      patch10.x[0][i] = patch00.x[3][i];
      patch10.y[0][i] = patch00.y[3][i];
      patch10.x[3][i] = xx[3][i];
      patch10.y[3][i] = yy[3][i];
    }
    for (i = 4; i < 8; ++i) {
      patch01.x[0][i-4] = xx[0][i];
      patch01.y[0][i-4] = yy[0][i];
      patch01.x[1][i-4] = 0.5 * (xx[0][i] + xx[1][i]);
      patch01.y[1][i-4] = 0.5 * (yy[0][i] + yy[1][i]);
      xxm = 0.5 * (xx[1][i] + xx[2][i]);
      yym = 0.5 * (yy[1][i] + yy[2][i]);
      patch11.x[2][i-4] = 0.5 * (xx[2][i] + xx[3][i]);
      patch11.y[2][i-4] = 0.5 * (yy[2][i] + yy[3][i]);
      patch01.x[2][i-4] = 0.5 * (patch01.x[1][i-4] + xxm);
      patch01.y[2][i-4] = 0.5 * (patch01.y[1][i-4] + yym);
      patch11.x[1][i-4] = 0.5 * (xxm + patch11.x[2][i-4]);
      patch11.y[1][i-4] = 0.5 * (yym + patch11.y[2][i-4]);
      patch01.x[3][i-4] = 0.5 * (patch01.x[2][i-4] + patch11.x[1][i-4]);
      patch01.y[3][i-4] = 0.5 * (patch01.y[2][i-4] + patch11.y[1][i-4]);
      patch11.x[0][i-4] = patch01.x[3][i-4];
      patch11.y[0][i-4] = patch01.y[3][i-4];
      patch11.x[3][i-4] = xx[3][i];
      patch11.y[3][i-4] = yy[3][i];
    }
    //~ if the shading has a Function, this should interpolate on the
    //~ function parameter, not on the color components
    for (i = 0; i < nComps; ++i) {
      patch00.color[0][0].c[i] = patch->color[0][0].c[i];
      patch00.color[0][1].c[i] = (patch->color[0][0].c[i] +
				  patch->color[0][1].c[i]) / 2;
      patch01.color[0][0].c[i] = patch00.color[0][1].c[i];
      patch01.color[0][1].c[i] = patch->color[0][1].c[i];
      patch01.color[1][1].c[i] = (patch->color[0][1].c[i] +
				  patch->color[1][1].c[i]) / 2;
      patch11.color[0][1].c[i] = patch01.color[1][1].c[i];
      patch11.color[1][1].c[i] = patch->color[1][1].c[i];
      patch11.color[1][0].c[i] = (patch->color[1][1].c[i] +
				  patch->color[1][0].c[i]) / 2;
      patch10.color[1][1].c[i] = patch11.color[1][0].c[i];
      patch10.color[1][0].c[i] = patch->color[1][0].c[i];
      patch10.color[0][0].c[i] = (patch->color[1][0].c[i] +
				  patch->color[0][0].c[i]) / 2;
      patch00.color[1][0].c[i] = patch10.color[0][0].c[i];
      patch00.color[1][1].c[i] = (patch00.color[1][0].c[i] +
				  patch01.color[1][1].c[i]) / 2;
      patch01.color[1][0].c[i] = patch00.color[1][1].c[i];
      patch11.color[0][0].c[i] = patch00.color[1][1].c[i];
      patch10.color[0][1].c[i] = patch00.color[1][1].c[i];
    }
    fillPatch(&patch00, nComps, depth + 1);
    fillPatch(&patch10, nComps, depth + 1);
    fillPatch(&patch01, nComps, depth + 1);
    fillPatch(&patch11, nComps, depth + 1);
  }
}

void PdfParser::doEndPath() {
  if (state->isCurPt() && clip != clipNone) {
    state->clip();
    if (clip == clipNormal) {
      clipHistory->setClip(state->getPath(), clipNormal);
      builder->clip(state);
    } else {
      clipHistory->setClip(state->getPath(), clipEO);
      builder->clip(state, true);
    }
  }
  clip = clipNone;
  state->clearPath();
}

//------------------------------------------------------------------------
// path clipping operators
//------------------------------------------------------------------------

void PdfParser::opClip(Object args[], int numArgs) {
  clip = clipNormal;
}

void PdfParser::opEOClip(Object args[], int numArgs) {
  clip = clipEO;
}

//------------------------------------------------------------------------
// text object operators
//------------------------------------------------------------------------

void PdfParser::opBeginText(Object args[], int numArgs) {
  state->setTextMat(1, 0, 0, 1, 0, 0);
  state->textMoveTo(0, 0);
  builder->updateTextPosition(0.0, 0.0);
  fontChanged = gTrue;
  builder->beginTextObject(state);
}

void PdfParser::opEndText(Object args[], int numArgs) {
  builder->endTextObject(state);
}

//------------------------------------------------------------------------
// text state operators
//------------------------------------------------------------------------

void PdfParser::opSetCharSpacing(Object args[], int numArgs) {
  state->setCharSpace(args[0].getNum());
}

void PdfParser::opSetFont(Object args[], int numArgs) {
  GfxFont *font;

  if (!(font = res->lookupFont(args[0].getName()))) {
    // unsetting the font (drawing no text) is better than using the
    // previous one and drawing random glyphs from it
    state->setFont(NULL, args[1].getNum());
    fontChanged = gTrue;
    return;
  }
  if (printCommands) {
    printf("  font: tag=%s name='%s' %g\n",
	   font->getTag()->getCString(),
	   font->getName() ? font->getName()->getCString() : "???",
	   args[1].getNum());
    fflush(stdout);
  }

  font->incRefCnt();
  state->setFont(font, args[1].getNum());
  fontChanged = gTrue;
}

void PdfParser::opSetTextLeading(Object args[], int numArgs) {
  state->setLeading(args[0].getNum());
}

void PdfParser::opSetTextRender(Object args[], int numArgs) {
  state->setRender(args[0].getInt());
  builder->updateStyle(state);
}

void PdfParser::opSetTextRise(Object args[], int numArgs) {
  state->setRise(args[0].getNum());
}

void PdfParser::opSetWordSpacing(Object args[], int numArgs) {
  state->setWordSpace(args[0].getNum());
}

void PdfParser::opSetHorizScaling(Object args[], int numArgs) {
  state->setHorizScaling(args[0].getNum());
  builder->updateTextMatrix(state);
  fontChanged = gTrue;
}

//------------------------------------------------------------------------
// text positioning operators
//------------------------------------------------------------------------

void PdfParser::opTextMove(Object args[], int numArgs) {
  double tx, ty;

  tx = state->getLineX() + args[0].getNum();
  ty = state->getLineY() + args[1].getNum();
  state->textMoveTo(tx, ty);
  builder->updateTextPosition(tx, ty);
}

void PdfParser::opTextMoveSet(Object args[], int numArgs) {
  double tx, ty;

  tx = state->getLineX() + args[0].getNum();
  ty = args[1].getNum();
  state->setLeading(-ty);
  ty += state->getLineY();
  state->textMoveTo(tx, ty);
  builder->updateTextPosition(tx, ty);
}

void PdfParser::opSetTextMatrix(Object args[], int numArgs) {
  state->setTextMat(args[0].getNum(), args[1].getNum(),
		    args[2].getNum(), args[3].getNum(),
		    args[4].getNum(), args[5].getNum());
  state->textMoveTo(0, 0);
  builder->updateTextMatrix(state);
  builder->updateTextPosition(0.0, 0.0);
  fontChanged = gTrue;
}

void PdfParser::opTextNextLine(Object args[], int numArgs) {
  double tx, ty;

  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  builder->updateTextPosition(tx, ty);
}

//------------------------------------------------------------------------
// text string operators
//------------------------------------------------------------------------

void PdfParser::opShowText(Object args[], int numArgs) {
  if (!state->getFont()) {
    error(getPos(), "No font in show");
    return;
  }
  if (fontChanged) {
    builder->updateFont(state);
    fontChanged = gFalse;
  }
  doShowText(args[0].getString());
}

void PdfParser::opMoveShowText(Object args[], int numArgs) {
  double tx, ty;

  if (!state->getFont()) {
    error(getPos(), "No font in move/show");
    return;
  }
  if (fontChanged) {
    builder->updateFont(state);
    fontChanged = gFalse;
  }
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  builder->updateTextPosition(tx, ty);
  doShowText(args[0].getString());
}

void PdfParser::opMoveSetShowText(Object args[], int numArgs) {
  double tx, ty;

  if (!state->getFont()) {
    error(getPos(), "No font in move/set/show");
    return;
  }
  if (fontChanged) {
    builder->updateFont(state);
    fontChanged = gFalse;
  }
  state->setWordSpace(args[0].getNum());
  state->setCharSpace(args[1].getNum());
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  builder->updateTextPosition(tx, ty);
  doShowText(args[2].getString());
}

void PdfParser::opShowSpaceText(Object args[], int numArgs) {
  Array *a;
  Object obj;
  int wMode;
  int i;

  if (!state->getFont()) {
    error(getPos(), "No font in show/space");
    return;
  }
  if (fontChanged) {
    builder->updateFont(state);
    fontChanged = gFalse;
  }
  wMode = state->getFont()->getWMode();
  a = args[0].getArray();
  for (i = 0; i < a->getLength(); ++i) {
    a->get(i, &obj);
    if (obj.isNum()) {
      // this uses the absolute value of the font size to match
      // Acrobat's behavior
      if (wMode) {
	state->textShift(0, -obj.getNum() * 0.001 *
			    fabs(state->getFontSize()));
      } else {
	state->textShift(-obj.getNum() * 0.001 *
			 fabs(state->getFontSize()), 0);
      }
      builder->updateTextShift(state, obj.getNum());
    } else if (obj.isString()) {
      doShowText(obj.getString());
    } else {
      error(getPos(), "Element of show/space array must be number or string");
    }
    obj.free();
  }
}



/*
 * The `POPPLER_NEW_GFXFONT' stuff is for the change to GfxFont's getNextChar() call.
 * Thanks to tsdgeos for the fix.
 * Miklos, does this look ok?
 */   

void PdfParser::doShowText(GooString *s) {
  GfxFont *font;
  int wMode;
  double riseX, riseY;
  CharCode code;
#ifdef POPPLER_NEW_GFXFONT
  Unicode *u = NULL;
#else
  Unicode u[8];
#endif
  double x, y, dx, dy, dx2, dy2, curX, curY, tdx, tdy, lineX, lineY;
  double originX, originY, tOriginX, tOriginY;
  double oldCTM[6], newCTM[6];
  double *mat;
  Object charProc;
  Dict *resDict;
  Parser *oldParser;
  char *p;
  int len, n, uLen, nChars, nSpaces, i;

  font = state->getFont();
  wMode = font->getWMode();

  builder->beginString(state, s);

  // handle a Type 3 char
  if (font->getType() == fontType3 && 0) {//out->interpretType3Chars()) {
    mat = state->getCTM();
    for (i = 0; i < 6; ++i) {
      oldCTM[i] = mat[i];
    }
    mat = state->getTextMat();
    newCTM[0] = mat[0] * oldCTM[0] + mat[1] * oldCTM[2];
    newCTM[1] = mat[0] * oldCTM[1] + mat[1] * oldCTM[3];
    newCTM[2] = mat[2] * oldCTM[0] + mat[3] * oldCTM[2];
    newCTM[3] = mat[2] * oldCTM[1] + mat[3] * oldCTM[3];
    mat = font->getFontMatrix();
    newCTM[0] = mat[0] * newCTM[0] + mat[1] * newCTM[2];
    newCTM[1] = mat[0] * newCTM[1] + mat[1] * newCTM[3];
    newCTM[2] = mat[2] * newCTM[0] + mat[3] * newCTM[2];
    newCTM[3] = mat[2] * newCTM[1] + mat[3] * newCTM[3];
    newCTM[0] *= state->getFontSize();
    newCTM[1] *= state->getFontSize();
    newCTM[2] *= state->getFontSize();
    newCTM[3] *= state->getFontSize();
    newCTM[0] *= state->getHorizScaling();
    newCTM[2] *= state->getHorizScaling();
    state->textTransformDelta(0, state->getRise(), &riseX, &riseY);
    curX = state->getCurX();
    curY = state->getCurY();
    lineX = state->getLineX();
    lineY = state->getLineY();
    oldParser = parser;
    p = s->getCString();
    len = s->getLength();
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
#ifdef POPPLER_NEW_GFXFONT
			    &u, &uLen,  /* TODO: This looks like a memory leak for u. */
#else
			    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
#endif
			    &dx, &dy, &originX, &originY);
      dx = dx * state->getFontSize() + state->getCharSpace();
      if (n == 1 && *p == ' ') {
	dx += state->getWordSpace();
      }
      dx *= state->getHorizScaling();
      dy *= state->getFontSize();
      state->textTransformDelta(dx, dy, &tdx, &tdy);
      state->transform(curX + riseX, curY + riseY, &x, &y);
      saveState();
      state->setCTM(newCTM[0], newCTM[1], newCTM[2], newCTM[3], x, y);
      //~ the CTM concat values here are wrong (but never used)
      //out->updateCTM(state, 1, 0, 0, 1, 0, 0);
      if (0){ /*!out->beginType3Char(state, curX + riseX, curY + riseY, tdx, tdy,
			       code, u, uLen)) {*/
	((Gfx8BitFont *)font)->getCharProc(code, &charProc);
	if ((resDict = ((Gfx8BitFont *)font)->getResources())) {
	  pushResources(resDict);
	}
	if (charProc.isStream()) {
	  //parse(&charProc, gFalse); // TODO: parse into SVG font
	} else {
	  error(getPos(), "Missing or bad Type3 CharProc entry");
	}
	//out->endType3Char(state);
	if (resDict) {
	  popResources();
	}
	charProc.free();
      }
      restoreState();
      // GfxState::restore() does *not* restore the current position,
      // so we deal with it here using (curX, curY) and (lineX, lineY)
      curX += tdx;
      curY += tdy;
      state->moveTo(curX, curY);
      state->textSetPos(lineX, lineY);
      p += n;
      len -= n;
    }
    parser = oldParser;

  } else {
    state->textTransformDelta(0, state->getRise(), &riseX, &riseY);
    p = s->getCString();
    len = s->getLength();
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
#ifdef POPPLER_NEW_GFXFONT
			    &u, &uLen,  /* TODO: This looks like a memory leak for u. */
#else
			    u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
#endif
			    &dx, &dy, &originX, &originY);
      
      if (wMode) {
	dx *= state->getFontSize();
	dy = dy * state->getFontSize() + state->getCharSpace();
	if (n == 1 && *p == ' ') {
	  dy += state->getWordSpace();
	}
      } else {
	dx = dx * state->getFontSize() + state->getCharSpace();
	if (n == 1 && *p == ' ') {
	  dx += state->getWordSpace();
	}
	dx *= state->getHorizScaling();
	dy *= state->getFontSize();
      }
      state->textTransformDelta(dx, dy, &tdx, &tdy);
      originX *= state->getFontSize();
      originY *= state->getFontSize();
      state->textTransformDelta(originX, originY, &tOriginX, &tOriginY);
      builder->addChar(state, state->getCurX() + riseX, state->getCurY() + riseY,
                       dx, dy, tOriginX, tOriginY, code, n, u, uLen);
      state->shift(tdx, tdy);
      p += n;
      len -= n;
    }
  }

  builder->endString(state);
}


//------------------------------------------------------------------------
// XObject operators
//------------------------------------------------------------------------

void PdfParser::opXObject(Object args[], int numArgs) {
  char *name;
  Object obj1, obj2, obj3, refObj;

  name = args[0].getName();
  if (!res->lookupXObject(name, &obj1)) {
    return;
  }
  if (!obj1.isStream()) {
    error(getPos(), "XObject '%s' is wrong type", name);
    obj1.free();
    return;
  }
  obj1.streamGetDict()->lookup("Subtype", &obj2);
  if (obj2.isName("Image")) {
    res->lookupXObjectNF(name, &refObj);
    doImage(&refObj, obj1.getStream(), gFalse);
    refObj.free();
  } else if (obj2.isName("Form")) {
    doForm(&obj1);
  } else if (obj2.isName("PS")) {
    obj1.streamGetDict()->lookup("Level1", &obj3);
/*    out->psXObject(obj1.getStream(),
    		   obj3.isStream() ? obj3.getStream() : (Stream *)NULL);*/
  } else if (obj2.isName()) {
    error(getPos(), "Unknown XObject subtype '%s'", obj2.getName());
  } else {
    error(getPos(), "XObject subtype is missing or wrong type");
  }
  obj2.free();
  obj1.free();
}

void PdfParser::doImage(Object *ref, Stream *str, GBool inlineImg) {
  Dict *dict, *maskDict;
  int width, height;
  int bits, maskBits;
  StreamColorSpaceMode csMode;
  GBool mask;
  GBool invert;
  GfxColorSpace *colorSpace, *maskColorSpace;
  GfxImageColorMap *colorMap, *maskColorMap;
  Object maskObj, smaskObj;
  GBool haveColorKeyMask, haveExplicitMask, haveSoftMask;
  int maskColors[2*gfxColorMaxComps];
  int maskWidth, maskHeight;
  GBool maskInvert;
  Stream *maskStr;
  Object obj1, obj2;
  int i;

  // get info from the stream
  bits = 0;
  csMode = streamCSNone;
  str->getImageParams(&bits, &csMode);

  // get stream dict
  dict = str->getDict();

  // get size
  dict->lookup("Width", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("W", &obj1);
  }
  if (obj1.isInt())
    width = obj1.getInt();
  else if (obj1.isReal())
    width = (int)obj1.getReal();
  else
    goto err2;
  obj1.free();
  dict->lookup("Height", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("H", &obj1);
  }
  if (obj1.isInt())
    height = obj1.getInt();
  else if (obj1.isReal())
    height = (int)obj1.getReal();
  else
    goto err2;
  obj1.free();

  // image or mask?
  dict->lookup("ImageMask", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("IM", &obj1);
  }
  mask = gFalse;
  if (obj1.isBool())
    mask = obj1.getBool();
  else if (!obj1.isNull())
    goto err2;
  obj1.free();

  // bit depth
  if (bits == 0) {
    dict->lookup("BitsPerComponent", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("BPC", &obj1);
    }
    if (obj1.isInt()) {
      bits = obj1.getInt();
    } else if (mask) {
      bits = 1;
    } else {
      goto err2;
    }
    obj1.free();
  }

  // display a mask
  if (mask) {

    // check for inverted mask
    if (bits != 1)
      goto err1;
    invert = gFalse;
    dict->lookup("Decode", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("D", &obj1);
    }
    if (obj1.isArray()) {
      obj1.arrayGet(0, &obj2);
      if (obj2.isInt() && obj2.getInt() == 1)
	invert = gTrue;
      obj2.free();
    } else if (!obj1.isNull()) {
      goto err2;
    }
    obj1.free();

    // draw it
    builder->addImageMask(state, str, width, height, invert);

  } else {

    // get color space and color map
    dict->lookup("ColorSpace", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("CS", &obj1);
    }
    if (obj1.isName()) {
      res->lookupColorSpace(obj1.getName(), &obj2);
      if (!obj2.isNull()) {
	obj1.free();
	obj1 = obj2;
      } else {
	obj2.free();
      }
    }
    if (!obj1.isNull()) {
      colorSpace = GfxColorSpace::parse(&obj1);
    } else if (csMode == streamCSDeviceGray) {
      colorSpace = new GfxDeviceGrayColorSpace();
    } else if (csMode == streamCSDeviceRGB) {
      colorSpace = new GfxDeviceRGBColorSpace();
    } else if (csMode == streamCSDeviceCMYK) {
      colorSpace = new GfxDeviceCMYKColorSpace();
    } else {
      colorSpace = NULL;
    }
    obj1.free();
    if (!colorSpace) {
      goto err1;
    }
    dict->lookup("Decode", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("D", &obj1);
    }
    colorMap = new GfxImageColorMap(bits, &obj1, colorSpace);
    obj1.free();
    if (!colorMap->isOk()) {
      delete colorMap;
      goto err1;
    }

    // get the mask
    haveColorKeyMask = haveExplicitMask = haveSoftMask = gFalse;
    maskStr = NULL; // make gcc happy
    maskWidth = maskHeight = 0; // make gcc happy
    maskInvert = gFalse; // make gcc happy
    maskColorMap = NULL; // make gcc happy
    dict->lookup("Mask", &maskObj);
    dict->lookup("SMask", &smaskObj);
    if (smaskObj.isStream()) {
      // soft mask
      if (inlineImg) {
	goto err1;
      }
      maskStr = smaskObj.getStream();
      maskDict = smaskObj.streamGetDict();
      maskDict->lookup("Width", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("W", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskWidth = obj1.getInt();
      obj1.free();
      maskDict->lookup("Height", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("H", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskHeight = obj1.getInt();
      obj1.free();
      maskDict->lookup("BitsPerComponent", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("BPC", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskBits = obj1.getInt();
      obj1.free();
      maskDict->lookup("ColorSpace", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("CS", &obj1);
      }
      if (obj1.isName()) {
	res->lookupColorSpace(obj1.getName(), &obj2);
	if (!obj2.isNull()) {
	  obj1.free();
	  obj1 = obj2;
	} else {
	  obj2.free();
	}
      }
      maskColorSpace = GfxColorSpace::parse(&obj1);
      obj1.free();
      if (!maskColorSpace || maskColorSpace->getMode() != csDeviceGray) {
	goto err1;
      }
      maskDict->lookup("Decode", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("D", &obj1);
      }
      maskColorMap = new GfxImageColorMap(maskBits, &obj1, maskColorSpace);
      obj1.free();
      if (!maskColorMap->isOk()) {
	delete maskColorMap;
	goto err1;
      }
      //~ handle the Matte entry
      haveSoftMask = gTrue;
    } else if (maskObj.isArray()) {
      // color key mask
      for (i = 0;
	   i < maskObj.arrayGetLength() && i < 2*gfxColorMaxComps;
	   ++i) {
	maskObj.arrayGet(i, &obj1);
	maskColors[i] = obj1.getInt();
	obj1.free();
      }
      haveColorKeyMask = gTrue;
    } else if (maskObj.isStream()) {
      // explicit mask
      if (inlineImg) {
	goto err1;
      }
      maskStr = maskObj.getStream();
      maskDict = maskObj.streamGetDict();
      maskDict->lookup("Width", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("W", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskWidth = obj1.getInt();
      obj1.free();
      maskDict->lookup("Height", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("H", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskHeight = obj1.getInt();
      obj1.free();
      maskDict->lookup("ImageMask", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("IM", &obj1);
      }
      if (!obj1.isBool() || !obj1.getBool()) {
	goto err2;
      }
      obj1.free();
      maskInvert = gFalse;
      maskDict->lookup("Decode", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("D", &obj1);
      }
      if (obj1.isArray()) {
	obj1.arrayGet(0, &obj2);
	if (obj2.isInt() && obj2.getInt() == 1) {
	  maskInvert = gTrue;
	}
	obj2.free();
      } else if (!obj1.isNull()) {
	goto err2;
      }
      obj1.free();
      haveExplicitMask = gTrue;
    }

    // draw it
    if (haveSoftMask) {
        builder->addSoftMaskedImage(state, str, width, height, colorMap,
                                    maskStr, maskWidth, maskHeight, maskColorMap);
      delete maskColorMap;
    } else if (haveExplicitMask) {
        builder->addMaskedImage(state, str, width, height, colorMap,
                                maskStr, maskWidth, maskHeight, maskInvert);
    } else {
      builder->addImage(state, str, width, height, colorMap,
		        haveColorKeyMask ? maskColors : (int *)NULL);
    }
    delete colorMap;

    maskObj.free();
    smaskObj.free();
  }

  return;

 err2:
  obj1.free();
 err1:
  error(getPos(), "Bad image parameters");
}

void PdfParser::doForm(Object *str) {
  Dict *dict;
  GBool transpGroup, isolated, knockout;
  GfxColorSpace *blendingColorSpace;
  Object matrixObj, bboxObj;
  double m[6], bbox[4];
  Object resObj;
  Dict *resDict;
  Object obj1, obj2, obj3;
  int i;

  // check for excessive recursion
  if (formDepth > 20) {
    return;
  }

  // get stream dict
  dict = str->streamGetDict();

  // check form type
  dict->lookup("FormType", &obj1);
  if (!(obj1.isNull() || (obj1.isInt() && obj1.getInt() == 1))) {
    error(getPos(), "Unknown form type");
  }
  obj1.free();

  // get bounding box
  dict->lookup("BBox", &bboxObj);
  if (!bboxObj.isArray()) {
    bboxObj.free();
    error(getPos(), "Bad form bounding box");
    return;
  }
  for (i = 0; i < 4; ++i) {
    bboxObj.arrayGet(i, &obj1);
    bbox[i] = obj1.getNum();
    obj1.free();
  }
  bboxObj.free();

  // get matrix
  dict->lookup("Matrix", &matrixObj);
  if (matrixObj.isArray()) {
    for (i = 0; i < 6; ++i) {
      matrixObj.arrayGet(i, &obj1);
      m[i] = obj1.getNum();
      obj1.free();
    }
  } else {
    m[0] = 1; m[1] = 0;
    m[2] = 0; m[3] = 1;
    m[4] = 0; m[5] = 0;
  }
  matrixObj.free();

  // get resources
  dict->lookup("Resources", &resObj);
  resDict = resObj.isDict() ? resObj.getDict() : (Dict *)NULL;

  // check for a transparency group
  transpGroup = isolated = knockout = gFalse;
  blendingColorSpace = NULL;
  if (dict->lookup("Group", &obj1)->isDict()) {
    if (obj1.dictLookup("S", &obj2)->isName("Transparency")) {
      transpGroup = gTrue;
      if (!obj1.dictLookup("CS", &obj3)->isNull()) {
	blendingColorSpace = GfxColorSpace::parse(&obj3);
      }
      obj3.free();
      if (obj1.dictLookup("I", &obj3)->isBool()) {
	isolated = obj3.getBool();
      }
      obj3.free();
      if (obj1.dictLookup("K", &obj3)->isBool()) {
	knockout = obj3.getBool();
      }
      obj3.free();
    }
    obj2.free();
  }
  obj1.free();

  // draw it
  ++formDepth;
  doForm1(str, resDict, m, bbox,
	  transpGroup, gFalse, blendingColorSpace, isolated, knockout);
  --formDepth;

  if (blendingColorSpace) {
    delete blendingColorSpace;
  }
  resObj.free();
}

void PdfParser::doForm1(Object *str, Dict *resDict, double *matrix, double *bbox,
		  GBool transpGroup, GBool softMask,
		  GfxColorSpace *blendingColorSpace,
		  GBool isolated, GBool knockout,
		  GBool alpha, Function *transferFunc,
		  GfxColor *backdropColor) {
  Parser *oldParser;
  double oldBaseMatrix[6];
  int i;

  // push new resources on stack
  pushResources(resDict);

  // save current graphics state
  saveState();

  // kill any pre-existing path
  state->clearPath();

  if (softMask || transpGroup) {
    builder->clearSoftMask(state);
    builder->pushTransparencyGroup(state, bbox, blendingColorSpace,
                                   isolated, knockout, softMask);
  }

  // save current parser
  oldParser = parser;

  // set form transformation matrix
  state->concatCTM(matrix[0], matrix[1], matrix[2],
		   matrix[3], matrix[4], matrix[5]);
  builder->setTransform(matrix[0], matrix[1], matrix[2],
                        matrix[3], matrix[4], matrix[5]);

  // set form bounding box
  state->moveTo(bbox[0], bbox[1]);
  state->lineTo(bbox[2], bbox[1]);
  state->lineTo(bbox[2], bbox[3]);
  state->lineTo(bbox[0], bbox[3]);
  state->closePath();
  state->clip();
  clipHistory->setClip(state->getPath());
  builder->clip(state);
  state->clearPath();

  if (softMask || transpGroup) {
    if (state->getBlendMode() != gfxBlendNormal) {
      state->setBlendMode(gfxBlendNormal);
    }
    if (state->getFillOpacity() != 1) {
      builder->setGroupOpacity(state->getFillOpacity());
      state->setFillOpacity(1);
    }
    if (state->getStrokeOpacity() != 1) {
      state->setStrokeOpacity(1);
    }
  }

  // set new base matrix
  for (i = 0; i < 6; ++i) {
    oldBaseMatrix[i] = baseMatrix[i];
    baseMatrix[i] = state->getCTM()[i];
  }

  // draw the form
  parse(str, gFalse);

  // restore base matrix
  for (i = 0; i < 6; ++i) {
    baseMatrix[i] = oldBaseMatrix[i];
  }

  // restore parser
  parser = oldParser;

  if (softMask || transpGroup) {
      builder->popTransparencyGroup(state);
  }

  // restore graphics state
  restoreState();

  // pop resource stack
  popResources();

  if (softMask) {
    builder->setSoftMask(state, bbox, alpha, transferFunc, backdropColor);
  } else if (transpGroup) {
    builder->paintTransparencyGroup(state, bbox);
  }

  return;
}

//------------------------------------------------------------------------
// in-line image operators
//------------------------------------------------------------------------

void PdfParser::opBeginImage(Object args[], int numArgs) {
  Stream *str;
  int c1, c2;

  // build dict/stream
  str = buildImageStream();

  // display the image
  if (str) {
    doImage(NULL, str, gTrue);
  
    // skip 'EI' tag
    c1 = str->getUndecodedStream()->getChar();
    c2 = str->getUndecodedStream()->getChar();
    while (!(c1 == 'E' && c2 == 'I') && c2 != EOF) {
      c1 = c2;
      c2 = str->getUndecodedStream()->getChar();
    }
    delete str;
  }
}

Stream *PdfParser::buildImageStream() {
  Object dict;
  Object obj;
  char *key;
  Stream *str;

  // build dictionary
  dict.initDict(xref);
  parser->getObj(&obj);
  while (!obj.isCmd("ID") && !obj.isEOF()) {
    if (!obj.isName()) {
      error(getPos(), "Inline image dictionary key must be a name object");
      obj.free();
    } else {
      key = copyString(obj.getName());
      obj.free();
      parser->getObj(&obj);
      if (obj.isEOF() || obj.isError()) {
	gfree(key);
	break;
      }
      dict.dictAdd(key, &obj);
    }
    parser->getObj(&obj);
  }
  if (obj.isEOF()) {
    error(getPos(), "End of file in inline image");
    obj.free();
    dict.free();
    return NULL;
  }
  obj.free();

  // make stream
  str = new EmbedStream(parser->getStream(), &dict, gFalse, 0);
  str = str->addFilters(&dict);

  return str;
}

void PdfParser::opImageData(Object args[], int numArgs) {
  error(getPos(), "Internal: got 'ID' operator");
}

void PdfParser::opEndImage(Object args[], int numArgs) {
  error(getPos(), "Internal: got 'EI' operator");
}

//------------------------------------------------------------------------
// type 3 font operators
//------------------------------------------------------------------------

void PdfParser::opSetCharWidth(Object args[], int numArgs) {
}

void PdfParser::opSetCacheDevice(Object args[], int numArgs) {
}

//------------------------------------------------------------------------
// compatibility operators
//------------------------------------------------------------------------

void PdfParser::opBeginIgnoreUndef(Object args[], int numArgs) {
  ++ignoreUndef;
}

void PdfParser::opEndIgnoreUndef(Object args[], int numArgs) {
  if (ignoreUndef > 0)
    --ignoreUndef;
}

//------------------------------------------------------------------------
// marked content operators
//------------------------------------------------------------------------

void PdfParser::opBeginMarkedContent(Object args[], int numArgs) {
  if (printCommands) {
    printf("  marked content: %s ", args[0].getName());
    if (numArgs == 2)
      args[2].print(stdout);
    printf("\n");
    fflush(stdout);
  }

  if(numArgs == 2) {
    //out->beginMarkedContent(args[0].getName(),args[1].getDict());
  } else {
    //out->beginMarkedContent(args[0].getName());
  }
}

void PdfParser::opEndMarkedContent(Object args[], int numArgs) {
  //out->endMarkedContent();
}

void PdfParser::opMarkPoint(Object args[], int numArgs) {
  if (printCommands) {
    printf("  mark point: %s ", args[0].getName());
    if (numArgs == 2)
      args[2].print(stdout);
    printf("\n");
    fflush(stdout);
  }

  if(numArgs == 2) {
    //out->markPoint(args[0].getName(),args[1].getDict());
  } else {
    //out->markPoint(args[0].getName());
  }

}

//------------------------------------------------------------------------
// misc
//------------------------------------------------------------------------

void PdfParser::saveState() {
  builder->saveState();
  state = state->save();
  clipHistory = clipHistory->save();
}

void PdfParser::restoreState() {
  clipHistory = clipHistory->restore();
  state = state->restore();
  builder->restoreState();
}

void PdfParser::pushResources(Dict *resDict) {
  res = new GfxResources(xref, resDict, res);
}

void PdfParser::popResources() {
  GfxResources *resPtr;

  resPtr = res->getNext();
  delete res;
  res = resPtr;
}

void PdfParser::setDefaultApproximationPrecision() {
  int i;

  for (i = 1; i <= pdfNumShadingTypes; ++i) {
    setApproximationPrecision(i, defaultShadingColorDelta, defaultShadingMaxDepth);
  }
}

void PdfParser::setApproximationPrecision(int shadingType, double colorDelta,
                                          int maxDepth) {

  if (shadingType > pdfNumShadingTypes || shadingType < 1) {
    return;
  }
  colorDeltas[shadingType-1] = dblToCol(colorDelta);
  maxDepths[shadingType-1] = maxDepth;
}

//------------------------------------------------------------------------
// ClipHistoryEntry
//------------------------------------------------------------------------

ClipHistoryEntry::ClipHistoryEntry(GfxPath *clipPathA, GfxClipType clipTypeA) {
    if (clipPathA) {
        clipPath = clipPathA->copy();
    } else {
        clipPath = NULL;
    }
    clipType = clipTypeA;
    saved = NULL;
}

ClipHistoryEntry::~ClipHistoryEntry() {
    if (clipPath) {
        delete clipPath;
    }
}

void ClipHistoryEntry::setClip(GfxPath *clipPathA, GfxClipType clipTypeA) {
    // Free previous clip path
    if (clipPath) {
        delete clipPath;
    }
    if (clipPathA) {
        clipPath = clipPathA->copy();
        clipType = clipTypeA;
    } else {
        clipPath = NULL;
    }
}

ClipHistoryEntry *ClipHistoryEntry::save() {
    ClipHistoryEntry *newEntry = new ClipHistoryEntry(this);
    newEntry->saved = this;

    return newEntry;
}

ClipHistoryEntry *ClipHistoryEntry::restore() {
    ClipHistoryEntry *oldEntry;

    if (saved) {
        oldEntry = saved;
        saved = NULL;
        delete this;
    } else {
        oldEntry = this;
    }

    return oldEntry;
}

ClipHistoryEntry::ClipHistoryEntry(ClipHistoryEntry *other) {
    if (other->clipPath) {
        this->clipPath = other->clipPath->copy();
        this->clipType = other->clipType;
    } else {
        this->clipPath = NULL;
    }
    saved = NULL;
}

#endif /* HAVE_POPPLER */
