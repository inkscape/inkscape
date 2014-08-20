/**
    \file clear-n_.h
 
    A way to clear the N_ macro, which is defined as an inline function.
	Unfortunately, this makes it so it is hard to use in static strings
	where you only want to translate a small part.  Including this
	turns it back into a a macro.
*/
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef N_
#undef N_
#endif
#define N_(x) x

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
