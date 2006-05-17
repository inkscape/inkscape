#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libnr/nr-blit.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/raster-glyph.h>
#include <libnrtype/RasterFont.h>
#include <libnrtype/TextWrapper.h>
#include <libnrtype/one-glyph.h>

#include <glibmm.h>
#include <gtkmm.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>

#include "font-lister.h"

namespace Inkscape
{
                FontLister::FontLister ()
                {
                    font_list_store = Gtk::ListStore::create (FontList);
                    
                    if (font_factory::Default()->Families(&families))
                    {
                        for (unsigned int i = 0; i < families.length; ++i)
                        {
                            Gtk::TreeModel::iterator iter = font_list_store->append();
                            (*iter)[FontList.font] = reinterpret_cast<const char*>(families.names[i]);

                            NRStyleList styles;
                            if (font_factory::Default()->Styles (reinterpret_cast<const char*>(families.names[i]), &styles));

                            GList *Styles=0;
                            for (unsigned int n = 0; n < styles.length; ++n)
                            {
                                    NRStyleRecord style_record = styles.records[n];
                                    Styles = g_list_append (Styles, strdup(style_record.name));
                            }

                            (*iter)[FontList.styles] = Styles;

                            font_list_store_iter_map.insert (std::make_pair (reinterpret_cast<const char*>(families.names[i]), Gtk::TreePath (iter)));
                        }
                    }
                    
                }

                FontLister::~FontLister ()
                {
                };

                const Glib::RefPtr<Gtk::ListStore>
                FontLister::get_font_list () const
                {
                    return font_list_store;
                }
}

