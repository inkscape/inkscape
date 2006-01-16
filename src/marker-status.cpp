
void marker_status(char const *format, ...)
{
    /* Don't bother inlining this.  Not called often, and eventually all
       calls will be removed anyway. */
#if 0 /* Bryce sets this to 1. */
    va_list args;
    va_start(args, format);
    g_logv(marker_status_INITIAL_ARGS, format, args);
    va_end(args);
#endif
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
