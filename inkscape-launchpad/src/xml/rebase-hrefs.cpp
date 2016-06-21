#include "xml/rebase-hrefs.h"
#include "dir-util.h"
#include "../document.h"  /* Unfortunately there's a separate xml/document.h. */
#include "io/sys.h"
#include "sp-object.h"
#include "streq.h"
#include "util/share.h"
#include "xml/attribute-record.h"
#include "xml/node.h"
#include <glib.h>
#include <glibmm/miscutils.h>
#include <glibmm/convert.h>
#include <glibmm/uriutils.h>

using Inkscape::XML::AttributeRecord;

/**
 * Determine if a href needs rebasing.
 */
static bool href_needs_rebasing(std::string const &href)
{
    bool ret = true;

    if ( href.empty() || (href[0] == '#') ) {
        ret = false;
        /* False (no change) is the right behaviour even when the base URI differs from the
         * document URI: RFC 3986 defines empty string relative URL as referring to the containing
         * document, rather than referring to the base URI. */
    } else {
        /* Don't change data or http hrefs. */
        std::string scheme = Glib::uri_parse_scheme(href);
        if ( !scheme.empty() ) {
            /* Assume it shouldn't be changed.  This is probably wrong if the scheme is `file'
             * (or if the scheme of the new base is non-file, though I believe that never
             * happens at the time of writing), but that's rare, and we won't try too hard to
             * handle this now: wait until after the freeze, then add liburiparser (or similar)
             * as a dependency and do it properly.  For now we'll just try to be simple (while
             * at least still correctly handling data hrefs). */
            ret = false;
        } else if (Glib::path_is_absolute(href)) {
            /* If absolute then keep it as is.
             *
             * Even in the following borderline cases:
             *
             *   - We keep it absolute even if it is in new_base (directly or indirectly).
             *
             *   - We assume that if xlink:href is absolute then we honour it in preference to
             *     sodipodi:absref even if sodipodi:absref points to an existing file while xlink:href
             *     doesn't.  This is because we aren't aware of any bugs in xlink:href handling when
             *     it's absolute, so we assume that it's the best value to use even in this case.)
             */
            /* No strong preference on what we do for sodipodi:absref.  Once we're
             * confident of our handling of xlink:href and xlink:base, we should clear it.
             * Though for the moment we do the simple thing: neither clear nor set it. */
            ret = false;
        }
    }

    return ret;
}

static std::string calc_abs_href(std::string const &abs_base_dir, std::string const &href,
                                 gchar const *const sp_absref)
{
    std::string ret = Glib::build_filename(abs_base_dir, href);

    if ( sp_absref
         && !Inkscape::IO::file_test(ret.c_str(),       G_FILE_TEST_EXISTS)
         &&  Inkscape::IO::file_test(sp_absref, G_FILE_TEST_EXISTS) )
    {
        /* sodipodi:absref points to an existing file while xlink:href doesn't.
         * This could mean that xlink:href is wrong, or it could mean that the user
         * intends to supply the missing file later.
         *
         * Given that we aren't sure what the right behaviour is, and given that a
         * wrong xlink:href value may mean a bug (as has occurred in the past), we
         * write a message to stderr. */
        g_warning("xlink:href points to non-existent file, so using sodipodi:absref instead");

        /* Currently, we choose to use sodipodi:absref in this situation (because we
         * aren't yet confident in xlink:href interpretation); though note that
         * honouring a foreign attribute in preference to standard SVG xlink:href and
         * xlink:base means that we're not a conformant SVG user agent, so eventually
         * we hope to have enough confidence in our xlink:href and xlink:base handling
         * to be able to disregard sodipodi:absref.
         *
         * effic: Once we no longer consult sodipodi:absref, we can do
         * `if (base unchanged) { return; }' at the start of rebase_hrefs.
         */
        ret = sp_absref;
    }

    return ret;
}

Inkscape::Util::List<AttributeRecord const>
Inkscape::XML::rebase_href_attrs(gchar const *const old_abs_base,
                                 gchar const *const new_abs_base,
                                 Inkscape::Util::List<AttributeRecord const> attributes)
{
    using Inkscape::Util::List;
    using Inkscape::Util::cons;
    using Inkscape::Util::ptr_shared;
    using Inkscape::Util::share_string;


    if (old_abs_base == new_abs_base) {
        return attributes;
    }

    GQuark const href_key = g_quark_from_static_string("xlink:href");
    GQuark const absref_key = g_quark_from_static_string("sodipodi:absref");

    /* First search attributes for xlink:href and sodipodi:absref, putting the rest in ret.
     *
     * However, if we find that xlink:href doesn't need rebasing, then return immediately
     * with no change to attributes. */
    ptr_shared<char> old_href;
    ptr_shared<char> sp_absref;
    List<AttributeRecord const> ret;
    {
        for (List<AttributeRecord const> ai(attributes); ai; ++ai) {
            if (ai->key == href_key) {
                old_href = ai->value;
                if (!href_needs_rebasing(static_cast<char const *>(old_href))) {
                    return attributes;
                }
            } else if (ai->key == absref_key) {
                sp_absref = ai->value;
            } else {
                ret = cons(AttributeRecord(ai->key, ai->value), ret);
            }
        }
    }

    if (!old_href) {
        return attributes;
        /* We could instead return ret in this case, i.e. ensure that sodipodi:absref is cleared if
         * no xlink:href attribute.  However, retaining it might be more cautious.
         *
         * (For the usual case of not present, attributes and ret will be the same except
         * reversed.) */
    }

    std::string abs_href = calc_abs_href(old_abs_base, static_cast<char const *>(old_href), sp_absref);
    std::string new_href = sp_relative_path_from_path(abs_href, new_abs_base);
    ret = cons(AttributeRecord(href_key, share_string(new_href.c_str())), ret); // Check if this is safe/copied or if it is only held.
    if (sp_absref) {
        /* We assume that if there wasn't previously a sodipodi:absref attribute
         * then we shouldn't create one. */
        ret = cons(AttributeRecord(absref_key, ( streq(abs_href.c_str(), sp_absref)
                                                 ? sp_absref
                                                 : share_string(abs_href.c_str()) )),
                   ret);
    }

    return ret;
}

// std::string Inkscape::XML::rebase_href_attrs( std::string const &oldAbsBase, std::string const &newAbsBase, gchar const * /*href*/, gchar const */*absref*/ )
// {
//     std::string ret;
//     //g_message( "XX  need to flip from [%s] to [%s]", oldAbsBase.c_str(), newAbsBase.c_str() );

//     if ( oldAbsBase != newAbsBase ) {
//     }

//     return ret;
// }

std::string Inkscape::XML::calc_abs_doc_base(gchar const *doc_base)
{
    /* Note that we don't currently try to handle the case of doc_base containing
     * `..' or `.' path components.  This non-handling means that sometimes
     * sp_relative_path_from_path will needlessly give an absolute path.
     *
     * It's probably not worth trying to address this until we're using proper
     * relative URL/IRI href processing (with liburiparser).
     *
     * (Note that one possibile difficulty with `..' is symlinks.) */
    std::string ret;

    if (!doc_base) {
        ret = Glib::get_current_dir();
    } else if (Glib::path_is_absolute(doc_base)) {
        ret = doc_base;
    } else {
        ret = Glib::build_filename( Glib::get_current_dir(), doc_base );
    }

    return ret;
}

void Inkscape::XML::rebase_hrefs(SPDocument *const doc, gchar const *const new_base, bool const spns)
{
    if (!doc->getBase()) {
        return;
    }

    std::string old_abs_base = calc_abs_doc_base(doc->getBase());
    std::string new_abs_base = calc_abs_doc_base(new_base);

    /* TODO: Should handle not just image but also:
     *
     *    a, altGlyph, animElementAttrs, animate, animateColor, animateMotion, animateTransform,
     *    animation, audio, color-profile, cursor, definition-src, discard, feImage, filter,
     *    font-face-uri, foreignObject, glyphRef, handler, linearGradient, mpath, pattern,
     *    prefetch, radialGradient, script, set, textPath, tref, use, video
     *
     * (taken from the union of the xlink:href elements listed at
     * http://www.w3.org/TR/SVG11/attindex.html and
     * http://www.w3.org/TR/SVGMobile12/attributeTable.html).
     *
     * Also possibly some other attributes of type <URI> or <IRI> or list-thereof, or types like
     * <paint> that can include an IRI/URI, and stylesheets and style attributes.  (xlink:base is a
     * special case.  xlink:role and xlink:arcrole can be assumed to be already absolute, based on
     * http://www.w3.org/TR/SVG11/struct.html#xlinkRefAttrs .)
     *
     * Note that it may not useful to set sodipodi:absref for anything other than image.
     *
     * Note also that Inkscape only supports fragment hrefs (href="#pattern257") for many of these
     * cases. */
    std::set<SPObject *> images = doc->getResourceList("image");
    for (std::set<SPObject *>::const_iterator it = images.begin(); it != images.end(); ++it) {
        Inkscape::XML::Node *ir = (*it)->getRepr();

        std::string uri;
        {
            gchar const *tmp = ir->attribute("xlink:href");
            if ( !tmp ) {
                continue;
            }
            uri = tmp;
        }
        if ( uri.substr(0, 7) == "file://" ) {
            uri = Glib::filename_from_uri(uri);
        }
        // The following two cases are for absolute hrefs that can be converted to relative.
        // Imported images, first time rebased, need an old base.
        std::string href = uri;
        if ( Glib::path_is_absolute(href) ) {
            href = sp_relative_path_from_path(uri, old_abs_base);
        }
        // Files moved from a absolute path need a new one.
        if ( Glib::path_is_absolute(href) ) {
            href = sp_relative_path_from_path(uri, new_abs_base);
        }
        // Other bitmaps are either really absolute, or already relative.

#ifdef WIN32
        /* Windows relative path needs their native separators before we
         * compare it to native baserefs. */
        if ( !Glib::path_is_absolute(href) ) {
            std::replace(href.begin(), href.end(), '/', '\\');
        }
#endif

        /* TODO: Most of this function currently treats href as if it were a simple filename
         * (e.g. passing it to g_path_is_absolute, g_build_filename or IO::file_test, or avoiding
         * changing non-file hrefs), which breaks if href starts with a scheme or if href contains
         * any escaping. */

        if ( href_needs_rebasing(href) ) {
            std::string abs_href = calc_abs_href(old_abs_base, href, ir->attribute("sodipodi:absref"));

            /* todo: One difficult case once we support writing to non-file locations is where
             * existing hrefs in the document point to local files.  In this case, we should
             * probably copy those referenced files to the new location at the same time.  It's
             * less clear what to do when copying from one non-file location to another.  We may
             * need to ask the user in some way (even if it's as a checkbox), but we'd like to
             * bother the user as little as possible yet also want to warn the user about the case
             * of file hrefs. */

            std::string new_href = sp_relative_path_from_path(abs_href, new_abs_base);
            ir->setAttribute("sodipodi:absref", ( spns
                                                  ? abs_href.c_str()
                                                  : NULL ));
            if (!Glib::path_is_absolute(new_href)) {
#ifdef WIN32
                /* Native Windows path separators are replaced with / so that the href
                 * also works on Gnu/Linux and OSX */
                std::replace(href.begin(), href.end(), '\\', '/');
#endif
                ir->setAttribute("xlink:href", new_href.c_str());
            } else {
                ir->setAttribute("xlink:href", g_filename_to_uri(new_href.c_str(), NULL, NULL));
            }

            /* impl: I assume that if !spns then any existing sodipodi:absref is about to get
             * cleared (or is already cleared) anyway, in which case it doesn't matter whether we
             * clear or leave any existing sodipodi:absref value.  If that assumption turns out to
             * be wrong, then leaving it means risking leaving the wrong value (if xlink:href
             * referred to a different file than sodipodi:absref) while clearing it means risking
             * losing information. */
        }
    }
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
// vi: set autoindent shiftwidth=4 tabstop=8 filetype=cpp expandtab softtabstop=4 encoding=utf-8 textwidth=99 :
