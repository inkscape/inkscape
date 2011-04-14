/* Copyright (C) 2001-2010 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* $Id: trace.h 227 2010-12-16 05:47:19Z selinger $ */

#ifndef TRACE_H
#define TRACE_H

#include "potracelib.h"
#include "progress.h"
#include "curve.h"

int process_path(path_t *plist, const potrace_param_t *param, progress_t *progress);

#endif /* TRACE_H */
