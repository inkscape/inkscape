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

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkDeprecatedRuler GtkDeprecatedRuler;
typedef struct _GtkDeprecatedRulerClass GtkDeprecatedRulerClass;
typedef struct _GtkDeprecatedVRuler GtkDeprecatedVRuler;
typedef struct _GtkDeprecatedVRulerClass GtkDeprecatedVRulerClass;
typedef struct _GtkDeprecatedHRuler GtkDeprecatedHRuler;
typedef struct _GtkDeprecatedHRulerClass GtkDeprecatedHRulerClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


// This is an import of the now-deprecated Gtk::Ruler class.

namespace Gtk {
namespace Deprecated {
class Ruler_Class;
class VRuler_Class;
class HRuler_Class;

/** Base class for horizontal or vertical rulers.
 *
 * This is an abstract base for Gtk::HRuler and
 * Gtk::VRuler.  Users should only instantiate those types.
 */

class Ruler : public Widget
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef Ruler CppObjectType;
  typedef Ruler_Class CppClassType;
  typedef GtkDeprecatedRuler BaseObjectType;
  typedef GtkDeprecatedRulerClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~Ruler();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class Ruler_Class;
  static CppClassType ruler_class_;

  // noncopyable
  Ruler(const Ruler&);
  Ruler& operator=(const Ruler&);

protected:
  explicit Ruler(const Glib::ConstructParams& construct_params);
  explicit Ruler(GtkDeprecatedRuler* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;


  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  GtkDeprecatedRuler*       gobj()       { return reinterpret_cast<GtkDeprecatedRuler*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const GtkDeprecatedRuler* gobj() const { return reinterpret_cast<GtkDeprecatedRuler*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


private:

  
protected:
  Ruler();

public:


  /** Sets the desired metric of the ruler.  The possible choices are:
   * <ul><li>Gtk::PIXELS
   * <li>Gtk::INCHES
   * <li>Gtk::CENTIMETERS
   * </ul>The default metric is Gtk::PIXELS.
   */
  void set_metric(MetricType metric =  PIXELS);

  
  /** Gets the units used for a Gtk::Ruler. See set_metric().
   * @return The units currently used for @a ruler
   * 
   *  @a Deprecated: 2.24: Gtk::Ruler has been removed from GTK 3 for being
   * unmaintained and too specialized. There is no replacement.
   */
  MetricType get_metric() const;
  
#ifndef GTKMM_DISABLE_DEPRECATED

  /** Gets the units used for a Gtk::Ruler. See set_metric().
   * @deprecated Use the const version
   * @return The units currently used for @a ruler
   * 
   *  @a Deprecated: 2.24: Gtk::Ruler has been removed from GTK 3 for being
   * unmaintained and too specialized. There is no replacement.
   */
  MetricType get_metric();
#endif // GTKMM_DISABLE_DEPRECATED


#ifndef GTKMM_DISABLE_DEPRECATED

//TODO: Remove these when we can break ABI:

  /** @deprecated Use get_range() instead.
   */
   double get_range_lower() const;
 
  /** @deprecated Use get_range() instead.
   */
   double get_range_upper() const;
 #endif // GTKMM_DISABLE_DEPRECATED


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
   *  @a Deprecated: 2.24: Gtk::Ruler has been removed from GTK 3 for being
   * unmaintained and too specialized. There is no replacement.
   * @param lower Location to store lower limit of the ruler, or <tt>0</tt>.
   * @param upper Location to store upper limit of the ruler, or <tt>0</tt>.
   * @param position Location to store the current position of the mark on the ruler, or <tt>0</tt>.
   * @param max_size Location to store the maximum size of the ruler used when calculating
   * the space to leave for the text, or <tt>0</tt>.
   */
  void get_range(double& lower, double& upper, double& position, double& max_size);


  /** draw tick marks on the ruler
   */
  void draw_ticks();


  /** draw a position indicator on the ruler
   */
  void draw_pos();

    virtual void draw_ticks_vfunc();

    virtual void draw_pos_vfunc();


  #ifdef GLIBMM_PROPERTIES_ENABLED
/** Lower limit of ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<double> property_lower() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** Lower limit of ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<double> property_lower() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** Upper limit of ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<double> property_upper() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** Upper limit of ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<double> property_upper() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** Position of mark on the ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<double> property_position() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** Position of mark on the ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<double> property_position() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** Maximum size of the ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<double> property_max_size() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** Maximum size of the ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<double> property_max_size() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** The metric used for the ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<MetricType> property_metric() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** The metric used for the ruler.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<MetricType> property_metric() const;
#endif //#GLIBMM_PROPERTIES_ENABLED


};


/** Vertical Ruler */
class VRuler : public Ruler
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef VRuler CppObjectType;
  typedef VRuler_Class CppClassType;
  typedef GtkDeprecatedVRuler BaseObjectType;
  typedef GtkDeprecatedVRulerClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~VRuler();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class VRuler_Class;
  static CppClassType vruler_class_;

  // noncopyable
  VRuler(const VRuler&);
  VRuler& operator=(const VRuler&);

protected:
  explicit VRuler(const Glib::ConstructParams& construct_params);
  explicit VRuler(GtkDeprecatedVRuler* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;


  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  GtkDeprecatedVRuler*       gobj()       { return reinterpret_cast<GtkDeprecatedVRuler*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const GtkDeprecatedVRuler* gobj() const { return reinterpret_cast<GtkDeprecatedVRuler*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


private:

  
public:
  VRuler();


}; //class VRuler

/** Horizontal Ruler */
class HRuler : public Ruler
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef HRuler CppObjectType;
  typedef HRuler_Class CppClassType;
  typedef GtkDeprecatedHRuler BaseObjectType;
  typedef GtkDeprecatedHRulerClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~HRuler();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class HRuler_Class;
  static CppClassType hruler_class_;

  // noncopyable
  HRuler(const HRuler&);
  HRuler& operator=(const HRuler&);

protected:
  explicit HRuler(const Glib::ConstructParams& construct_params);
  explicit HRuler(GtkDeprecatedHRuler* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;


  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  GtkDeprecatedHRuler*       gobj()       { return reinterpret_cast<GtkDeprecatedHRuler*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const GtkDeprecatedHRuler* gobj() const { return reinterpret_cast<GtkDeprecatedHRuler*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:

protected:
  //GTK+ Virtual Functions (override these to change behaviour):

  //Default Signal Handlers::


private:

  
public:
  HRuler();


}; // class HRuler


}
}



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
