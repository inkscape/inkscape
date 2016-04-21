/**
 * @file
 * Unclumping objects.
 */
/* Authors:
 *   bulia byak
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2005 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <algorithm>
#include <map>
#include <2geom/transforms.h>
#include "sp-item.h"
#include "unclump.h"

#include <glib.h>

// Taking bbox of an item is an expensive operation, and we need to do it many times, so here we
// cache the centers, widths, and heights of items

//FIXME: make a class with these cashes as members instead of globals
std::map<const gchar *, Geom::Point> c_cache;
std::map<const gchar *, Geom::Point> wh_cache;

/**
Center of bbox of item
*/
static Geom::Point
unclump_center (SPItem *item)
{
    std::map<const gchar *, Geom::Point>::iterator i = c_cache.find(item->getId());
    if ( i != c_cache.end() ) {
        return i->second;
    }

    Geom::OptRect r = item->desktopVisualBounds();
    if (r) {
        Geom::Point const c = r->midpoint();
        c_cache[item->getId()] = c;
        return c;
    } else {
        // FIXME
        return Geom::Point(0, 0);
    }
}

static Geom::Point
unclump_wh (SPItem *item)
{
    Geom::Point wh;
    std::map<const gchar *, Geom::Point>::iterator i = wh_cache.find(item->getId());
    if ( i != wh_cache.end() ) {
        wh = i->second;
    } else {
        Geom::OptRect r = item->desktopVisualBounds();
        if (r) {
            wh = r->dimensions();
            wh_cache[item->getId()] = wh;
        } else {
            wh = Geom::Point(0, 0);
        }
    }

    return wh;
}

/**
Distance between "edges" of item1 and item2. An item is considered to be an ellipse inscribed into its w/h,
so its radius (distance from center to edge) depends on the w/h and the angle towards the other item.
May be negative if the edge of item1 is between the center and the edge of item2.
*/
static double
unclump_dist (SPItem *item1, SPItem *item2)
{
	Geom::Point c1 = unclump_center (item1);
	Geom::Point c2 = unclump_center (item2);

	Geom::Point wh1 = unclump_wh (item1);
	Geom::Point wh2 = unclump_wh (item2);

	// angle from each item's center to the other's, unsqueezed by its w/h, normalized to 0..pi/2
	double a1 = atan2 ((c2 - c1)[Geom::Y], (c2 - c1)[Geom::X] * wh1[Geom::Y]/wh1[Geom::X]);
	a1 = fabs (a1);
	if (a1 > M_PI/2) a1 = M_PI - a1;

	double a2 = atan2 ((c1 - c2)[Geom::Y], (c1 - c2)[Geom::X] * wh2[Geom::Y]/wh2[Geom::X]);
	a2 = fabs (a2);
	if (a2 > M_PI/2) a2 = M_PI - a2;

	// get the radius of each item for the given angle
	double r1 = 0.5 * (wh1[Geom::X] + (wh1[Geom::Y] - wh1[Geom::X]) * (a1/(M_PI/2)));
	double r2 = 0.5 * (wh2[Geom::X] + (wh2[Geom::Y] - wh2[Geom::X]) * (a2/(M_PI/2)));

	// dist between centers minus angle-adjusted radii
	double dist_r =  (Geom::L2 (c2 - c1) - r1 - r2);

	double stretch1 = wh1[Geom::Y]/wh1[Geom::X];
	double stretch2 = wh2[Geom::Y]/wh2[Geom::X];

	if ((stretch1 > 1.5 || stretch1 < 0.66) && (stretch2 > 1.5 || stretch2 < 0.66)) {

		std::vector<double> dists;
		dists.push_back (dist_r);

		// If both objects are not circle-like, find dists between four corners
		std::vector<Geom::Point> c1_points(2);
		{
			double y_closest;
			if (c2[Geom::Y] > c1[Geom::Y] + wh1[Geom::Y]/2) {
				y_closest = c1[Geom::Y] + wh1[Geom::Y]/2;
			} else if (c2[Geom::Y] < c1[Geom::Y] - wh1[Geom::Y]/2) {
				y_closest = c1[Geom::Y] - wh1[Geom::Y]/2;
			} else {
				y_closest = c2[Geom::Y];
			}
			c1_points[0] = Geom::Point (c1[Geom::X], y_closest);
			double x_closest;
			if (c2[Geom::X] > c1[Geom::X] + wh1[Geom::X]/2) {
				x_closest = c1[Geom::X] + wh1[Geom::X]/2;
			} else if (c2[Geom::X] < c1[Geom::X] - wh1[Geom::X]/2) {
				x_closest = c1[Geom::X] - wh1[Geom::X]/2;
			} else {
				x_closest = c2[Geom::X];
			}
			c1_points[1] = Geom::Point (x_closest, c1[Geom::Y]);
		}


		std::vector<Geom::Point> c2_points(2);
		{
			double y_closest;
			if (c1[Geom::Y] > c2[Geom::Y] + wh2[Geom::Y]/2) {
				y_closest = c2[Geom::Y] + wh2[Geom::Y]/2;
			} else if (c1[Geom::Y] < c2[Geom::Y] - wh2[Geom::Y]/2) {
				y_closest = c2[Geom::Y] - wh2[Geom::Y]/2;
			} else {
				y_closest = c1[Geom::Y];
			}
			c2_points[0] = Geom::Point (c2[Geom::X], y_closest);
			double x_closest;
			if (c1[Geom::X] > c2[Geom::X] + wh2[Geom::X]/2) {
				x_closest = c2[Geom::X] + wh2[Geom::X]/2;
			} else if (c1[Geom::X] < c2[Geom::X] - wh2[Geom::X]/2) {
				x_closest = c2[Geom::X] - wh2[Geom::X]/2;
			} else {
				x_closest = c1[Geom::X];
			}
			c2_points[1] = Geom::Point (x_closest, c2[Geom::Y]);
		}

		for (int i = 0; i < 2; i ++) {
			for (int j = 0; j < 2; j ++) {
				dists.push_back (Geom::L2 (c1_points[i] - c2_points[j]));
			}
		}

		// return the minimum of all dists
		return *std::min_element(dists.begin(), dists.end());
	} else {
		return dist_r;
	}
}

/**
Average unclump_dist from item to others
*/
static double unclump_average (SPItem *item, std::list<SPItem*> &others)
{
    int n = 0;
    double sum = 0;
    for (std::list<SPItem*>::const_iterator i = others.begin(); i != others.end();++i) {
        SPItem *other = *i;

        if (other == item)
            continue;

        n++;
        sum += unclump_dist (item, other);
    }

    if (n != 0)
        return sum/n;
    else
        return 0;
}

/**
Closest to item among others
 */
static SPItem *unclump_closest (SPItem *item, std::list<SPItem*> &others)
{
    double min = HUGE_VAL;
    SPItem *closest = NULL;

    for (std::list<SPItem*>::const_iterator i = others.begin(); i != others.end();++i) {
    	SPItem *other = *i;

        if (other == item)
            continue;

        double dist = unclump_dist (item, other);
        if (dist < min && fabs (dist) < 1e6) {
            min = dist;
            closest = other;
        }
    }

    return closest;
}

/**
Most distant from item among others
 */
static SPItem *unclump_farest (SPItem *item, std::list<SPItem*> &others)
{
    double max = -HUGE_VAL;
    SPItem *farest = NULL;
    for (std::list<SPItem*>::const_iterator i = others.begin(); i != others.end();++i) {
        SPItem *other = *i;

        if (other == item)
            continue;

        double dist = unclump_dist (item, other);
        if (dist > max && fabs (dist) < 1e6) {
            max = dist;
            farest = other;
        }
    }

    return farest;
}

/**
Removes from the \a rest list those items that are "behind" \a closest as seen from \a item,
i.e. those on the other side of the line through \a closest perpendicular to the direction from \a
item to \a closest. Returns a newly created list which must be freed.
 */
static std::vector<SPItem*>
unclump_remove_behind (SPItem *item, SPItem *closest, std::list<SPItem*> &rest)
{
    Geom::Point it = unclump_center (item);
    Geom::Point p1 = unclump_center (closest);

    // perpendicular through closest to the direction to item:
    Geom::Point perp = Geom::rot90(it - p1);
    Geom::Point p2 = p1 + perp;

    // get the standard Ax + By + C = 0 form for p1-p2:
    double A = p1[Geom::Y] - p2[Geom::Y];
    double B = p2[Geom::X] - p1[Geom::X];
    double C = p2[Geom::Y] * p1[Geom::X] - p1[Geom::Y] * p2[Geom::X];

    // substitute the item into it:
    double val_item = A * it[Geom::X] + B * it[Geom::Y] + C;

    std::vector<SPItem*> out;
    for (std::list<SPItem*>::const_reverse_iterator i = rest.rbegin(); i != rest.rend();++i) {
        SPItem *other = *i;

        if (other == item)
            continue;

        Geom::Point o = unclump_center (other);
        double val_other = A * o[Geom::X] + B * o[Geom::Y] + C;

        if (val_item * val_other <= 1e-6) {
            // different signs, which means item and other are on the different sides of p1-p2 line; skip
        } else {
            out.push_back(other);
        }
    }

    return out;
}

/**
Moves \a what away from \a from by \a dist
 */
static void
unclump_push (SPItem *from, SPItem *what, double dist)
{
    Geom::Point it = unclump_center (what);
    Geom::Point p = unclump_center (from);
    Geom::Point by = dist * Geom::unit_vector (- (p - it));

    Geom::Affine move = Geom::Translate (by);

    std::map<const gchar *, Geom::Point>::iterator i = c_cache.find(what->getId());
    if ( i != c_cache.end() ) {
        i->second *= move;
    }

    //g_print ("push %s at %g,%g from %g,%g by %g,%g, dist %g\n", what->getId(), it[Geom::X],it[Geom::Y], p[Geom::X],p[Geom::Y], by[Geom::X],by[Geom::Y], dist);

    what->set_i2d_affine(what->i2dt_affine() * move);
    what->doWriteTransform(what->getRepr(), what->transform, NULL);
}

/**
Moves \a what towards \a to by \a dist
 */
static void
unclump_pull (SPItem *to, SPItem *what, double dist)
{
    Geom::Point it = unclump_center (what);
    Geom::Point p = unclump_center (to);
    Geom::Point by = dist * Geom::unit_vector (p - it);

    Geom::Affine move = Geom::Translate (by);

    std::map<const gchar *, Geom::Point>::iterator i = c_cache.find(what->getId());
    if ( i != c_cache.end() ) {
        i->second *= move;
    }

    //g_print ("pull %s at %g,%g to %g,%g by %g,%g, dist %g\n", what->getId(), it[Geom::X],it[Geom::Y], p[Geom::X],p[Geom::Y], by[Geom::X],by[Geom::Y], dist);

    what->set_i2d_affine(what->i2dt_affine() * move);
    what->doWriteTransform(what->getRepr(), what->transform, NULL);
}


/**
Unclumps the items in \a items, reducing local unevenness in their distribution. Produces an effect
similar to "engraver dots". The only distribution which is unchanged by unclumping is a hexagonal
grid. May be called repeatedly for stronger effect.
 */
void
unclump (std::vector<SPItem*> &items)
{
    c_cache.clear();
    wh_cache.clear();

    for (std::vector<SPItem*>::const_iterator i = items.begin(); i != items.end();++i) { //  for each original/clone x:
        SPItem *item = *i;

        std::list<SPItem*> nei;

        std::list<SPItem*> rest;
        for (int i=0; i < static_cast<int>(items.size()); i++) {
            rest.push_front(items[items.size() - i - 1]);
        }
        rest.remove(item);

        while (!rest.empty()) {
            SPItem *closest = unclump_closest (item, rest);
            if (closest) {
                nei.push_front(closest);
                rest.remove(closest);
                std::vector<SPItem*> new_rest = unclump_remove_behind (item, closest, rest);
                rest.clear();
                for (int i=0; i < static_cast<int>(new_rest.size()); i++) {
                    rest.push_front(new_rest[new_rest.size() - i - 1]);
                }
            } else {
                break;
            }
        }

        if ( (nei.size()) >= 2) {
            double ave = unclump_average (item, nei);

            SPItem *closest = unclump_closest (item, nei);
            SPItem *farest = unclump_farest (item, nei);

            double dist_closest = unclump_dist (closest, item);
            double dist_farest = unclump_dist (farest, item);

            //g_print ("NEI %d for item %s    closest %s at %g  farest %s at %g  ave %g\n", g_slist_length(nei), item->getId(), closest->getId(), dist_closest, farest->getId(), dist_farest, ave);

            if (fabs (ave) < 1e6 && fabs (dist_closest) < 1e6 && fabs (dist_farest) < 1e6) { // otherwise the items are bogus
                // increase these coefficients to make unclumping more aggressive and less stable
                // the pull coefficient is a bit bigger to counteract the long-term expansion trend
                unclump_push (closest, item, 0.3 * (ave - dist_closest));
                unclump_pull (farest, item, 0.35 * (dist_farest - ave));
            }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
