/*
 * Transforming single items
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@gmail.com>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Abhishek Sharma
 *   Diederik van Lierop <mail@diedenrezi.nl>
 *
 * Copyright (C) 1999-2011 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/transforms.h>
#include "sp-item.h"
#include "sp-item-transform.h"

#include <glib.h>

void sp_item_rotate_rel(SPItem *item, Geom::Rotate const &rotation)
{
    Geom::Point center = item->getCenter();
    Geom::Translate const s(item->getCenter());
    Geom::Affine affine = Geom::Affine(s).inverse() * Geom::Affine(rotation) * Geom::Affine(s);

    // Rotate item.
    item->set_i2d_affine(item->i2dt_affine() * (Geom::Affine)affine);
    // Use each item's own transform writer, consistent with sp_selection_apply_affine()
    item->doWriteTransform(item->getRepr(), item->transform);

    // Restore the center position (it's changed because the bbox center changed)
    if (item->isCenterSet()) {
        item->setCenter(center * affine);
        item->updateRepr();
    }
}

void sp_item_scale_rel(SPItem *item, Geom::Scale const &scale)
{
    Geom::OptRect bbox = item->desktopVisualBounds();
    if (bbox) {
        Geom::Translate const s(bbox->midpoint()); // use getCenter?
        item->set_i2d_affine(item->i2dt_affine() * s.inverse() * scale * s);
        item->doWriteTransform(item->getRepr(), item->transform);
    }
}

void sp_item_skew_rel(SPItem *item, double skewX, double skewY)
{
    Geom::Point center = item->getCenter();
    Geom::Translate const s(item->getCenter());

    Geom::Affine const skew(1, skewY, skewX, 1, 0, 0);
    Geom::Affine affine = Geom::Affine(s).inverse() * skew * Geom::Affine(s);

    item->set_i2d_affine(item->i2dt_affine() * affine);
    item->doWriteTransform(item->getRepr(), item->transform);

    // Restore the center position (it's changed because the bbox center changed)
    if (item->isCenterSet()) {
        item->setCenter(center * affine);
        item->updateRepr();
    }
}

void sp_item_move_rel(SPItem *item, Geom::Translate const &tr)
{
    item->set_i2d_affine(item->i2dt_affine() * tr);

    item->doWriteTransform(item->getRepr(), item->transform);
}

/**
 * Calculate the affine transformation required to transform one visual bounding box into another, accounting for a uniform strokewidth.
 *
 * PS: This function will only return accurate results for the visual bounding box of a selection of one or more objects, all having
 * the same strokewidth. If the stroke width varies from object to object in this selection, then the function
 * get_scale_transform_for_variable_stroke() should be called instead
 *
 * When scaling or stretching an object using the selector, e.g. by dragging the handles or by entering a value, we will
 * need to calculate the affine transformation for the old dimensions to the new dimensions. When using a geometric bounding
 * box this is very straightforward, but when using a visual bounding box this become more tricky as we need to account for
 * the strokewidth, which is either constant or scales width the area of the object. This function takes care of the calculation
 * of the affine transformation:
 * @param bbox_visual Current visual bounding box
 * @param stroke_x Apparent strokewidth in horizontal direction
 * @param stroke_y Apparent strokewidth in vertical direction
 * @param transform_stroke If true then the stroke will be scaled proportional to the square root of the area of the geometric bounding box
 * @param preserve If true then the transform element will be preserved in XML, and evaluated after stroke is applied
 * @param x0 Coordinate of the target visual bounding box
 * @param y0 Coordinate of the target visual bounding box
 * @param x1 Coordinate of the target visual bounding box
 * @param y1 Coordinate of the target visual bounding box
 *   PS: we have to pass each coordinate individually, to find out if we are mirroring the object; Using a Geom::Rect() instead is
 *   not possible here because it will only allow for a positive width and height, and therefore cannot mirror
 * @return
 */
Geom::Affine get_scale_transform_for_uniform_stroke(Geom::Rect const &bbox_visual, gdouble stroke_x, gdouble stroke_y, bool transform_stroke, bool preserve, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
    Geom::Affine p2o = Geom::Translate (-bbox_visual.min());
    Geom::Affine o2n = Geom::Translate (x0, y0);

    Geom::Affine scale = Geom::Scale (1, 1);
    Geom::Affine unbudge = Geom::Translate (0, 0); // moves the object(s) to compensate for the drift caused by stroke width change

    // 1) We start with a visual bounding box (w0, h0) which we want to transfer into another visual bounding box (w1, h1)
    // 2) The stroke is r0, equal for all edges, if preserve transforms is false
    // 3) Given this visual bounding box we can calculate the geometric bounding box by subtracting half the stroke from each side;
    // -> The width and height of the geometric bounding box will therefore be (w0 - 2*0.5*r0) and (h0 - 2*0.5*r0)
    // 4) If preserve transforms is true, then stroke_x != stroke_y, since these are the apparent stroke widths, after transforming

    if ((stroke_x == Geom::infinity()) || (fabs(stroke_x) < 1e-6)) stroke_x = 0;
    if ((stroke_y == Geom::infinity()) || (fabs(stroke_y) < 1e-6)) stroke_y = 0;

    gdouble w0 = bbox_visual.width(); // will return a value >= 0, as required further down the road
    gdouble h0 = bbox_visual.height();

    // We also know the width and height of the new visual bounding box
    gdouble w1 = x1 - x0; // can have any sign
    gdouble h1 = y1 - y0;
    // The new visual bounding box will have a stroke r1

    // Here starts the calculation you've been waiting for; first do some preparation
    int flip_x = (w1 > 0) ? 1 : -1;
    int flip_y = (h1 > 0) ? 1 : -1;
    
    // w1 and h1 will be negative when mirroring, but if so then e.g. w1-r0 won't make sense
    // Therefore we will use the absolute values from this point on
    w1 = fabs(w1);
    h1 = fabs(h1);
    // w0 and h0 will always be positive due to the definition of the width() and height() methods.

    // Check whether the stroke is negative; i.e. the geometric bounding box is larger than the visual bounding box, which
    // occurs for example for clipped objects (see launchpad bug #811819)
    if (stroke_x < 0 || stroke_y < 0) {
        Geom::Affine direct = Geom::Scale(flip_x * w1 / w0, flip_y* h1 / h0); // Scaling of the visual bounding box
        // How should we handle the stroke width scaling of clipped object? I don't know if we can/should handle this,
        // so for now we simply return the direct scaling
        return (p2o * direct * o2n);
    }
    gdouble r0 = sqrt(stroke_x*stroke_y); // r0 is redundant, used only for those cases where stroke_x = stroke_y

    // We will now try to calculate the affine transformation required to transform the first visual bounding box into
    // the second one, while accounting for strokewidth

    if ((fabs(w0 - stroke_x) < 1e-6) && (fabs(h0 - stroke_y) < 1e-6)) {
        return Geom::Affine();
    }

    gdouble scale_x = 1;
    gdouble scale_y = 1;
    gdouble r1;

    if ((fabs(w0 - stroke_x) < 1e-6) || w1 == 0) { // We have a vertical line at hand
        scale_y = h1/h0;
        scale_x = transform_stroke ? 1 : scale_y;
        unbudge *= Geom::Translate (-flip_x * 0.5 * (scale_x - 1.0) * w0, 0);
        unbudge *= Geom::Translate ( flip_x * 0.5 * (w1 - w0), 0); // compensate for the fact that this operation cannot be performed
    } else if ((fabs(h0 - stroke_y) < 1e-6) || h1 == 0) { // We have a horizontal line at hand
        scale_x = w1/w0;
        scale_y = transform_stroke ? 1 : scale_x;
        unbudge *= Geom::Translate (0, -flip_y * 0.5 * (scale_y - 1.0) * h0);
        unbudge *= Geom::Translate (0,  flip_y * 0.5 * (h1 - h0)); // compensate for the fact that this operation cannot be performed
    } else { // We have a true 2D object at hand
        if (transform_stroke && !preserve) {
            /* Initial area of the geometric bounding box: A0 = (w0-r0)*(h0-r0)
             * Desired area of the geometric bounding box: A1 = (w1-r1)*(h1-r1)
             * This is how the stroke should scale: r1^2 / A1 = r0^2 / A0
             * So therefore we will need to solve this equation:
             *
             * r1^2 * (w0-r0) * (h0-r0) = r0^2 * (w1-r1) * (h1-r1)
             *
             * This is a quadratic equation in r1, of which the roots can be found using the ABC formula
             * */
            gdouble A = -w0*h0 + r0*(w0 + h0);
            gdouble B = -(w1 + h1) * r0*r0;
            gdouble C = w1 * h1 * r0*r0;
            if (B*B - 4*A*C < 0) {
                g_message("stroke scaling error : %d, %f, %f, %f, %f, %f", preserve, r0, w0, h0, w1, h1);
            } else {
                r1 = -C/B;
                if (!Geom::are_near(A*C/B/B, 0.0, Geom::EPSILON))
                    r1 = fabs((-B - sqrt(B*B - 4*A*C))/(2*A));
                // If w1 < 0 then the scale will be wrong if we just assume that scale_x = (w1 - r1)/(w0 - r0);
                // Therefore we here need the absolute values of w0, w1, h0, h1, and r0, as taken care of earlier
                scale_x = (w1 - r1)/(w0 - r0);
                scale_y = (h1 - r1)/(h0 - r0);
                // Make sure that the lower-left corner of the visual bounding box stays where it is, even though the stroke width has changed
                unbudge *= Geom::Translate (-flip_x * 0.5 * (r0 * scale_x - r1), -flip_y * 0.5 * (r0 * scale_y - r1));
            }
        } else if (!transform_stroke && !preserve) { // scale the geometric bbox with constant stroke
            scale_x = (w1 - r0) / (w0 - r0);
            scale_y = (h1 - r0) / (h0 - r0);
            unbudge *= Geom::Translate (-flip_x * 0.5 * r0 * (scale_x - 1), -flip_y * 0.5 * r0 * (scale_y - 1));
        } else if (!transform_stroke) { // 'Preserve Transforms' was chosen.
            // geometric mean of stroke_x and stroke_y will be preserved
            // new_stroke_x = stroke_x*sqrt(scale_x/scale_y)
            // new_stroke_y = stroke_y*sqrt(scale_y/scale_x)
            // scale_x = (w1 - new_stroke_x)/(w0 - stroke_x)
            // scale_y = (h1 - new_stroke_y)/(h0 - stroke_y)
            gdouble A = h1*(w0 - stroke_x);
            gdouble B = (h0*stroke_x - w0*stroke_y);
            gdouble C = -w1*(h0 - stroke_y);
            gdouble Sx_div_Sy;          // Sx_div_Sy = sqrt(scale_x/scale_y)
            if (B*B - 4*A*C < 0) {
                g_message("stroke scaling error : %d, %f, %f, %f, %f, %f, %f", preserve, stroke_x, stroke_y, w0, h0, w1, h1);
            } else {
                Sx_div_Sy = (-B + sqrt(B*B - 4*A*C))/2/A;
                scale_x = (w1 - stroke_x*Sx_div_Sy)/(w0 - stroke_x);
                scale_y = (h1 - stroke_y/Sx_div_Sy)/(h0 - stroke_y);
                unbudge *= Geom::Translate (-flip_x * 0.5 * stroke_x * scale_x * (1.0 - sqrt(1.0/scale_x/scale_y)), -flip_y * 0.5 * stroke_y * scale_y * (1.0 - sqrt(1.0/scale_x/scale_y)));
            }
        } else {                        // 'Preserve Transforms' was chosen, and stroke is scaled
            scale_x = w1 / w0;
            scale_y = h1 / h0;
        }
    }

    // Now we account for mirroring by flipping if needed
    scale *= Geom::Scale(flip_x * scale_x, flip_y * scale_y);

    return (p2o * scale * unbudge * o2n);
}

/**
 * Calculate the affine transformation required to transform one visual bounding box into another, accounting for a VARIABLE strokewidth.
 *
 * Note: Please try to understand get_scale_transform_for_uniform_stroke() first, and read all it's comments carefully. This function
 * (get_scale_transform_for_variable_stroke) is a bit different because it will allow for a strokewidth that's different for each
 * side of the visual bounding box. Such a situation will arise when transforming the visual bounding box of a selection of objects,
 * each having a different stroke width. In fact this function is a generalized version of get_scale_transform_for_uniform_stroke(), but
 * will not (yet) replace it because it has not been tested as carefully, and because the old function is can serve as an introduction to
 * understand the new one.
 *
 * When scaling or stretching an object using the selector, e.g. by dragging the handles or by entering a value, we will
 * need to calculate the affine transformation for the old dimensions to the new dimensions. When using a geometric bounding
 * box this is very straightforward, but when using a visual bounding box this become more tricky as we need to account for
 * the strokewidth, which is either constant or scales width the area of the object. This function takes care of the calculation
 * of the affine transformation:
 *
 * @param bbox_visual Current visual bounding box
 * @param bbox_geometric Current geometric bounding box (allows for calculating the strokewidth of each edge)
 * @param transform_stroke If true then the stroke will be scaled proportional to the square root of the area of the geometric bounding box
 * @param preserve If true then the transform element will be preserved in XML, and evaluated after stroke is applied
 * @param x0 Coordinate of the target visual bounding box
 * @param y0 Coordinate of the target visual bounding box
 * @param x1 Coordinate of the target visual bounding box
 * @param y1 Coordinate of the target visual bounding box
 *    PS: we have to pass each coordinate individually, to find out if we are mirroring the object; Using a Geom::Rect() instead is
 *    not possible here because it will only allow for a positive width and height, and therefore cannot mirror
 * @return
 */
Geom::Affine get_scale_transform_for_variable_stroke(Geom::Rect const &bbox_visual, Geom::Rect const &bbox_geom, bool transform_stroke, bool preserve, gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
    Geom::Affine p2o = Geom::Translate (-bbox_visual.min());
    Geom::Affine o2n = Geom::Translate (x0, y0);

    Geom::Affine scale = Geom::Scale (1, 1);
    Geom::Affine unbudge = Geom::Translate (0, 0);  // moves the object(s) to compensate for the drift caused by stroke width change

    // 1) We start with a visual bounding box (w0, h0) which we want to transfer into another visual bounding box (w1, h1)
    // 2) We will also know the geometric bounding box, which can be used to calculate the strokewidth. The strokewidth will however
    //    be different for each of the four sides (left/right/top/bottom: r0l, r0r, r0t, r0b)

    gdouble w0 = bbox_visual.width(); // will return a value >= 0, as required further down the road
    gdouble h0 = bbox_visual.height();

    // We also know the width and height of the new visual bounding box
    gdouble w1 = x1 - x0; // can have any sign
    gdouble h1 = y1 - y0;
    // The new visual bounding box will have strokes r1l, r1r, r1t, and r1b

    // We will now try to calculate the affine transformation required to transform the first visual bounding box into
    // the second one, while accounting for strokewidth
    gdouble r0w = w0 - bbox_geom.width();  // r0w is the average strokewidth of the left and right edges, i.e. 0.5*(r0l + r0r)
    gdouble r0h = h0 - bbox_geom.height(); // r0h is the average strokewidth of the top and bottom edges, i.e. 0.5*(r0t + r0b)
    if ((r0w == Geom::infinity()) || (fabs(r0w) < 1e-6)) r0w = 0;
    if ((r0h == Geom::infinity()) || (fabs(r0h) < 1e-6)) r0h = 0;

    int flip_x = (w1 > 0) ? 1 : -1;
    int flip_y = (h1 > 0) ? 1 : -1;

    // w1 and h1 will be negative when mirroring, but if so then e.g. w1-r0 won't make sense
    // Therefore we will use the absolute values from this point on
    w1 = fabs(w1);
    h1 = fabs(h1);
    // w0 and h0 will always be positive due to the definition of the width() and height() methods.

    if ((fabs(w0 - r0w) < 1e-6) && (fabs(h0 - r0h) < 1e-6)) {
        return Geom::Affine();
    }

    // Check whether the stroke is negative; i.e. the geometric bounding box is larger than the visual bounding box, which
    // occurs for example for clipped objects (see launchpad bug #811819)
    if (r0w < 0 || r0h < 0) {
        Geom::Affine direct = Geom::Scale(flip_x * w1 / w0, flip_y* h1 / h0); // Scaling of the visual bounding box
        // How should we handle the stroke width scaling of clipped object? I don't know if we can/should handle this,
        // so for now we simply return the direct scaling
        return (p2o * direct * o2n);
    }

    // The calculation of the new strokewidth will only use the average stroke for each of the dimensions; To find the new stroke for each
    // of the edges individually though, we will use the boundary condition that the ratio of the left/right strokewidth will not change due to the
    // scaling. The same holds for the ratio of the top/bottom strokewidth.
    gdouble stroke_ratio_w = fabs(r0w) < 1e-6 ? 1 : (bbox_geom[Geom::X].min() - bbox_visual[Geom::X].min())/r0w;
    gdouble stroke_ratio_h = fabs(r0h) < 1e-6 ? 1 : (bbox_geom[Geom::Y].min() - bbox_visual[Geom::Y].min())/r0h;

    gdouble scale_x = 1;
    gdouble scale_y = 1;
    gdouble r1h;
    gdouble r1w;

    if ((fabs(w0 - r0w) < 1e-6) || w1 == 0) { // We have a vertical line at hand
        scale_y = h1/h0;
        scale_x = transform_stroke ? 1 : scale_y;
        unbudge *= Geom::Translate (-flip_x * 0.5 * (scale_x - 1.0) * w0, 0);
        unbudge *= Geom::Translate ( flip_x * 0.5 * (w1 - w0), 0); // compensate for the fact that this operation cannot be performed
    } else if ((fabs(h0 - r0h) < 1e-6) || h1 == 0) { // We have a horizontal line at hand
        scale_x = w1/w0;
        scale_y = transform_stroke ? 1 : scale_x;
        unbudge *= Geom::Translate (0, -flip_y * 0.5 * (scale_y - 1.0) * h0);
        unbudge *= Geom::Translate (0,  flip_y * 0.5 * (h1 - h0)); // compensate for the fact that this operation cannot be performed
    } else { // We have a true 2D object at hand
        if (transform_stroke && !preserve) {
            /* Initial area of the geometric bounding box: A0 = (w0-r0w)*(h0-r0h)
             * Desired area of the geometric bounding box: A1 = (w1-r1w)*(h1-r1h)
             * This is how the stroke should scale:     r1w^2 = A1/A0 * r0w^2, AND
             *                                          r1h^2 = A1/A0 * r0h^2
             * These can be re-expressed as : r1w/r0w = r1h/r0h
             * and : r1w*r1w*(w0 - r0w)*(h0 - r0h) = r0w*r0w*(w1 - r1w)*(h1 - r1h)
             * This leads to a quadratic equation in r1w, solved as follows:
             * */

            gdouble A = w0*h0 - r0h*w0 - r0w*h0;
            gdouble B = r0h*w1 + r0w*h1;
            gdouble C = -w1*h1;

            if (B*B - 4*A*C < 0) {
                g_message("variable stroke scaling error : %d, %d, %f, %f, %f, %f, %f, %f", transform_stroke, preserve, r0w, r0h, w0, h0, w1, h1);
            } else {
                gdouble det = -C/B;
                if (!Geom::are_near(A*C/B/B, 0.0, Geom::EPSILON))
                    det = (-B + sqrt(B*B - 4*A*C))/(2*A);
                r1w = r0w*det;
                r1h = r0h*det;
                // If w1 < 0 then the scale will be wrong if we just assume that scale_x = (w1 - r1)/(w0 - r0);
                // Therefore we here need the absolute values of w0, w1, h0, h1, and r0, as taken care of earlier
                scale_x = (w1 - r1w)/(w0 - r0w);
                scale_y = (h1 - r1h)/(h0 - r0h);
                // Make sure that the lower-left corner of the visual bounding box stays where it is, even though the stroke width has changed
                unbudge *= Geom::Translate (-flip_x * stroke_ratio_w * (r0w * scale_x - r1w), -flip_y * stroke_ratio_h * (r0h * scale_y - r1h));
            }
        } else if (!transform_stroke && !preserve) { // scale the geometric bbox with constant stroke
            scale_x = (w1 - r0w) / (w0 - r0w);
            scale_y = (h1 - r0h) / (h0 - r0h);
            unbudge *= Geom::Translate (-flip_x * stroke_ratio_w * r0w * (scale_x - 1), -flip_y * stroke_ratio_h * r0h * (scale_y - 1));
        } else if (!transform_stroke) { // 'Preserve Transforms' was chosen.
            // geometric mean of r0w and r0h will be preserved
            // new_r0w = r0w*sqrt(scale_x/scale_y)
            // new_r0h = r0h*sqrt(scale_y/scale_x)
            // scale_x = (w1 - new_r0w)/(w0 - r0w)
            // scale_y = (h1 - new_r0h)/(h0 - r0h)
            gdouble A = h1*(w0 - r0w);
            gdouble B = (h0*r0w - w0*r0h);
            gdouble C = -w1*(h0 - r0h);
            gdouble Sx_div_Sy;          // Sx_div_Sy = sqrt(scale_x/scale_y)
            if (B*B - 4*A*C < 0) {
                g_message("variable stroke scaling error : %d, %d, %f, %f, %f, %f, %f, %f", transform_stroke, preserve, r0w, r0h, w0, h0, w1, h1);
            } else {
                Sx_div_Sy = (-B + sqrt(B*B - 4*A*C))/2/A;
                scale_x = (w1 - r0w*Sx_div_Sy)/(w0 - r0w);
                scale_y = (h1 - r0h/Sx_div_Sy)/(h0 - r0h);
                unbudge *= Geom::Translate (-flip_x * stroke_ratio_w * r0w * scale_x * (1.0 - sqrt(1.0/scale_x/scale_y)), -flip_y * stroke_ratio_h * r0h * scale_y * (1.0 - sqrt(1.0/scale_x/scale_y)));
            }
        } else {                        // 'Preserve Transforms' was chosen, and stroke is scaled
            scale_x = w1 / w0;
            scale_y = h1 / h0;
        }
    }

    // Now we account for mirroring by flipping if needed
    scale *= Geom::Scale(flip_x * scale_x, flip_y * scale_y);

    return (p2o * scale * unbudge * o2n);
}

Geom::Rect get_visual_bbox(Geom::OptRect const &initial_geom_bbox, Geom::Affine const &abs_affine, gdouble const initial_strokewidth, bool const transform_stroke)
{
    g_assert(initial_geom_bbox);
    
    // Find the new geometric bounding box; Do this by transforming each corner of
    // the initial geometric bounding box individually and fitting a new boundingbox
    // around the transformerd corners  
    Geom::Point const p0 = Geom::Point(initial_geom_bbox->corner(0)) * abs_affine;    
    Geom::Rect new_geom_bbox(p0, p0);
    for (unsigned i = 1 ; i < 4 ; i++) {
        new_geom_bbox.expandTo(Geom::Point(initial_geom_bbox->corner(i)) * abs_affine);
    }

    Geom::Rect new_visual_bbox = new_geom_bbox; 
    if (initial_strokewidth > 0 && initial_strokewidth < Geom::infinity()) {
        if (transform_stroke) {
            // scale stroke by: sqrt (((w1-r0)/(w0-r0))*((h1-r0)/(h0-r0))) (for visual bboxes, see get_scale_transform_for_stroke)
            // equals scaling by: sqrt ((w1/w0)*(h1/h0)) for geometrical bboxes            
            // equals scaling by: sqrt (area1/area0) for geometrical bboxes
            gdouble const new_strokewidth = initial_strokewidth * sqrt (new_geom_bbox.area() / initial_geom_bbox->area());
            new_visual_bbox.expandBy(0.5 * new_strokewidth);        
        } else {
            // Do not transform the stroke
            new_visual_bbox.expandBy(0.5 * initial_strokewidth);   
        }
    }
    
    return new_visual_bbox;
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
