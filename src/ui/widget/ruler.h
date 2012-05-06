#ifndef SEEN_UI_WIDGET_RULER_H
#define SEEN_UI_WIDGET_RULER_H

/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 The Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/eventbox.h>
#include <2geom/point.h>

struct SPCanvasItem;
class  SPDesktop;
namespace Glib {
    class ustring;
}

typedef struct _SPRuler SPRuler;

// This is an import of the now-deprecated Gtk::Ruler API.
namespace Gtk {
namespace Deprecated {

/** Base class for horizontal or vertical rulers.
 *
 * This is an abstract base for Gtk::HRuler and
 * Gtk::VRuler.  Users should only instantiate those types.
 */
class Ruler : public Widget
{
public:
  virtual ~Ruler();

protected:
  explicit Ruler(const Glib::ConstructParams& construct_params);
  explicit Ruler(SPRuler* castitem);

public:
  ///Provides access to the underlying C GtkObject.
  SPRuler*       gobj()       { return reinterpret_cast<SPRuler*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const SPRuler* gobj() const { return reinterpret_cast<SPRuler*>(gobject_); }

protected:
  Ruler();

public:
  /** sets the range of the ruler.
   * <i>upper</i> and <i>lower</i> arguments denote the extents of the Ruler.
   * <i>max_size</i> is the largest number displayed by the ruler.
   * <i>position</i> gives the initial value of the ruler.
   * Rulers do not have sane defaults so this function should always be called.
   */
  void set_range(double lower, double upper, double position, double max_size);
  
  /** Retrieves values indicating the range and current position of a Gtk::Ruler.
   * See set_range().
   * 
   * @param lower Location to store lower limit of the ruler, or <tt>0</tt>.
   * @param upper Location to store upper limit of the ruler, or <tt>0</tt>.
   * @param position Location to store the current position of the mark on the ruler, or <tt>0</tt>.
   * @param max_size Location to store the maximum size of the ruler used when calculating
   * the space to leave for the text, or <tt>0</tt>.
   */
  void get_range(double& lower, double& upper, double& position, double& max_size);
};


/** Vertical Ruler */
class VRuler : public Ruler
{
  public:
  virtual ~VRuler();

public:
  ///Provides access to the underlying C GtkObject.
  SPRuler*       gobj()       { return reinterpret_cast<SPRuler*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const SPRuler* gobj() const { return reinterpret_cast<SPRuler*>(gobject_); }

public:
  VRuler();
}; //class VRuler

/** Horizontal Ruler */
class HRuler : public Ruler
{
  public:
  virtual ~HRuler();

public:
  ///Provides access to the underlying C GtkObject.
  SPRuler*       gobj()       { return reinterpret_cast<SPRuler*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const SPRuler* gobj() const { return reinterpret_cast<SPRuler*>(gobject_); }
  
public:
  HRuler();
}; // class HRuler
} // namespace Deprecated
} // namespace Gtk



namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Gtkmm facade/wrapper around sp_rulers.
 */
class Ruler : public Gtk::EventBox
{
public:
    void          init (SPDesktop*, Gtk::Widget&);
    void          get_range (double&, double&, double&, double&);
    void          set_range (double, double, double, double);
    void          update_metric();
    Glib::ustring get_tip();

protected:
    SPDesktop    *_dt;
    SPCanvasItem *_guide;
    Gtk::Widget  *_canvas_widget;
    Gtk::Deprecated::Ruler   *_r;
    bool         _horiz_f, _dragging;

    virtual bool on_button_press_event (GdkEventButton *);
    virtual bool on_motion_notify_event (GdkEventMotion *);
    virtual bool on_button_release_event (GdkEventButton *);

private:
    void canvas_get_pointer (int&, int&);
    Geom::Point get_event_dt();
};

/**
 * Horizontal ruler gtkmm wrapper.
 */
class HRuler : public Ruler
{
public:
    HRuler();
    ~HRuler();
};

/**
 * Vertical ruler gtkmm wrapper.
 */
class VRuler : public Ruler
{
public:
    VRuler();
    ~VRuler();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif // SEEN_UI_WIDGET_RULER_H


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
