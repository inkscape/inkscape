/*  This file is part of the libdepixelize project
    Copyright (C) 2013 Vinícius dos Santos Oliveira <vini.ipsmaker@gmail.com>

    GNU Lesser General Public License Usage
    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation; either version 2.1 of the License, or (at your
    option) any later version.
    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.

    GNU General Public License Usage
    Alternatively, this library may be used under the terms of the GNU General
    Public License as published by the Free Software Foundation, either version
    2 of the License, or (at your option) any later version.
    You should have received a copy of the GNU General Public License along with
    this library.  If not, see <http://www.gnu.org/licenses/>.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

#ifndef LIBDEPIXELIZE_TRACER_KOPFTRACER2011_H
#define LIBDEPIXELIZE_TRACER_KOPFTRACER2011_H

#include <string>

#include <glibmm.h>
// Contains exception definitions
#include <glibmm/fileutils.h>
#include <gdkmm/pixbuf.h>
#include "splines.h"

namespace Tracer {

class PixelGraph;

class Kopf2011
{
public:
    struct Options
    {
        enum Defaults {
            CURVES_MULTIPLIER        = 1,
            ISLANDS_WEIGHT           = 5,
            SPARSE_PIXELS_MULTIPLIER = 1,
            SPARSE_PIXELS_RADIUS     = 4
        };

        Options() :
            curvesMultiplier(CURVES_MULTIPLIER),
            islandsWeight(ISLANDS_WEIGHT),
            sparsePixelsMultiplier(SPARSE_PIXELS_MULTIPLIER),
            sparsePixelsRadius(SPARSE_PIXELS_RADIUS),
            optimize(true),
            nthreads(1)
        {}

        // Heuristics
        double curvesMultiplier;
        int islandsWeight;
        double sparsePixelsMultiplier;
        unsigned sparsePixelsRadius;

        // Other options
        bool optimize;
        int nthreads;
    };

    /**
     * # Exceptions
     *
     * \p options.optimize and options.nthreads will be ignored
     *
     * Glib::FileError
     * Gdk::PixbufError
     */
    static Splines to_voronoi(const std::string &filename,
                              const Options &options = Options());

    /*
     * \p options.optimize and options.nthreads will be ignored
     */
    static Splines to_voronoi(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
                              const Options &options = Options());

    /**
     * # Exceptions
     *
     * \p options.optimize and options.nthreads will be ignored
     *
     * Glib::FileError
     * Gdk::PixbufError
     */
    static Splines to_grouped_voronoi(const std::string &filename,
                                      const Options &options = Options());

    /*
     * \p options.optimize and options.nthreads will be ignored
     */
    static Splines to_grouped_voronoi(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
                                      const Options &options = Options());

    /**
     * # Exceptions
     *
     * Glib::FileError
     * Gdk::PixbufError
     */
    static Splines to_splines(const std::string &filename,
                              const Options &options = Options());

    static Splines to_splines(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
                              const Options &options = Options());

private:
    typedef Geom::Coord Precision;

    template<class T, bool adjust_splines>
    static SimplifiedVoronoi<T, adjust_splines>
    _voronoi(const Glib::RefPtr<Gdk::Pixbuf const> &buf,
             const Options &options);

    static void _disconnect_neighbors_with_dissimilar_colors(PixelGraph &graph);

    // here, T/template is only used as an easy way to not expose internal
    // symbols
    template<class T>
    static void _remove_crossing_edges_safe(T &container);
    template<class T>
    static void _remove_crossing_edges_unsafe(PixelGraph &graph, T &edges,
                                              const Options &options);
};

} // namespace Tracer

#endif // LIBDEPIXELIZE_TRACER_KOPFTRACER2011_H

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
