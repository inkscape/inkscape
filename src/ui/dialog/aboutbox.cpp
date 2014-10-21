/**
 * @file
 * Inkscape About box - implementation.
 */
/* Authors:
 *   Derek P. Moore <derekm@hackunix.org>
 *   MenTaLguY <mental@rydia.net>
 *   Kees Cook <kees@outflux.net>
 *   Jon Phillips <jon@rejon.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 Derek P. Moore
 * Copyright 2004 Kees Cook
 * Copyright 2004 Jon Phillips
 * Copyright 2005 MenTaLguY
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/dialog/aboutbox.h"
#include <glibmm/i18n.h>
#include <gtkmm/notebook.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/aspectframe.h>
#include <gtkmm/textview.h>
#include <gtkmm/stock.h>

#include "path-prefix.h"
#include "document.h"
#include "svg-view-widget.h"
#include "sp-text.h"
#include "text-editing.h"
#include "util/units.h"

#include "inkscape-version.h"




namespace Inkscape {
namespace UI {
namespace Dialog {



static Gtk::Widget *build_splash_widget();



static Gtk::ScrolledWindow 
  *make_scrolled_text(const Glib::ustring &contents);



static AboutBox *window=NULL;



void AboutBox::show_about() {
    if (!window)
        window = new AboutBox();
    window->show();
}



void AboutBox::hide_about() {
    if (window)
        window->hide();
}


/**
 * Constructor
 */ 
AboutBox::AboutBox() : Gtk::Dialog(_("About Inkscape")) {

    // call this first
    initStrings();

    Gtk::Notebook *tabs=new Gtk::Notebook();

    tabs->set_scrollable();

    Gtk::Widget *splash=build_splash_widget();
    if (splash) {
        tabs->append_page(*manage(splash), _("_Splash"), true);
    }

    tabs->append_page(*manage(
        make_scrolled_text(authors_text)), _("_Authors"), true);
    tabs->append_page(*manage(
        make_scrolled_text(translators_text)), _("_Translators"), true);
    tabs->append_page(*manage(
        make_scrolled_text(license_text)), _("_License"), true);

#if WITH_GTKMM_3_0
    get_content_area()->pack_end(*manage(tabs), true, true);
#else
    get_vbox()->pack_end(*manage(tabs), true, true);
#endif

    tabs->show_all();

    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
    set_default_response(Gtk::RESPONSE_CLOSE);

    Gtk::Label *label=new Gtk::Label();
    gchar *label_text = 
        g_strdup_printf("<small>Inkscape %s</small>",
              Inkscape::version_string);
    label->set_markup(label_text);
    label->set_alignment(Gtk::ALIGN_END, Gtk::ALIGN_CENTER);
    label->set_padding(5,0);
    g_free(label_text);
    label->set_selectable(true);
    label->show();

#if WITH_GTKMM_3_0
    get_content_area()->pack_start(*manage(label), false, false);
#else
    get_vbox()->pack_start(*manage(label), false, false);
#endif

    Gtk::Requisition requisition;
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_get_preferred_size(reinterpret_cast<GtkWidget*>(gobj()), &requisition, NULL);
#else
    gtk_widget_size_request (reinterpret_cast<GtkWidget*>(gobj()), &requisition);
#endif
    // allow window to shrink
    set_size_request(0, 0);
    set_default_size(requisition.width, requisition.height);
}




void AboutBox::on_response(int response_id) {
    if ( response_id == Gtk::RESPONSE_CLOSE ) {
        AboutBox::hide_about();
    }
}




Gtk::Widget *build_splash_widget() {
    /* TRANSLATORS: This is the filename of the `About Inkscape' picture in
       the `screens' directory.  Thus the translation of "about.svg" should be
       the filename of its translated version, e.g. about.zh.svg for Chinese.

       N.B. about.svg changes once per release.  (We should probably rename
       the original to about-0.40.svg etc. as soon as we have a translation.
       If we do so, then add an item to release-checklist saying that the
       string here should be changed.) */

    // FIXME? INKSCAPE_SCREENSDIR and "about.svg" are in UTF-8, not the
    // native filename encoding... and the filename passed to sp_document_new
    // should be in UTF-*8..

    char *about=g_build_filename(INKSCAPE_SCREENSDIR, _("about.svg"), NULL);
    SPDocument *doc=SPDocument::createNewDoc (about, TRUE);
    g_free(about);
    g_return_val_if_fail(doc != NULL, NULL);

    SPObject *version = doc->getObjectById("version");
    if ( version && SP_IS_TEXT(version) ) {
        sp_te_set_repr_text_multiline (SP_TEXT (version), Inkscape::version_string);
    }
    doc->ensureUpToDate();

    GtkWidget *v=sp_svg_view_widget_new(doc);

    double width=doc->getWidth().value("px");
    double height=doc->getHeight().value("px");
    
    doc->doUnref();

    SP_SVG_VIEW_WIDGET(v)->setResize(false, static_cast<int>(width), static_cast<int>(height));

    Gtk::AspectFrame *frame=new Gtk::AspectFrame();
    frame->unset_label();
    frame->set_shadow_type(Gtk::SHADOW_NONE);
    frame->property_ratio() = width / height;
    frame->add(*manage(Glib::wrap(v)));

    return frame;
}




/**
 *  This attempts to emulate the AboutDialog child dialog settings as
 *  closely as possible.  Mostly, that's margin widths and shadows.  Size
 *  is probably set in some other way, but this looked close enough.
 */
static Gtk::ScrolledWindow *make_scrolled_text(const Glib::ustring &contents) {
    Gtk::ScrolledWindow *scrolled=new Gtk::ScrolledWindow(); 

    scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled->set_shadow_type(Gtk::SHADOW_IN);

    Gtk::TextView *textview=manage(new Gtk::TextView());
    textview->set_editable(false);
    textview->set_left_margin(10);
    textview->set_right_margin(10);
    textview->get_buffer()->set_text(contents);
    scrolled->add(*textview);

    return scrolled;
}




/**
 * This method must be called before any of the texts are
 * used for making widgets
 */  
void AboutBox::initStrings() {

    //##############################
    //# A U T H O R S
    //##############################
    
    /* This text is copied from the AUTHORS file.
     * To update it, execute this snippet of sed magic in the toplevel
     * source directory:
     *
     * sed -e 's/^\(.*\) \([^ ]*\)*$/\2_ \1/' AUTHORS
           | sort
           | sed -e 's/^\([^_]*\)_ \(.*\)$/\2 \1/;s/^.*$/"\0\\n"/;$s/\\n//'
           | zenity --text-info
     *
     * and paste the result from the combo box here.
     */ 
    authors_text =
"Maximilian Albert\n"
"Joshua A. Andler\n"
"Tavmjong Bah\n"
"Pierre Barbry-Blot\n"
"Jean-François Barraud\n"
"Campbell Barton\n"
"Bill Baxter\n"
"John Beard\n"
"John Bintz\n"
"Arpad Biro\n"
"Nicholas Bishop\n"
"Joshua L. Blocher\n"
"Hanno Böck\n"
"Tomasz Boczkowski\n"
"Henrik Bohre\n"
"Boldewyn\n"
"Daniel Borgmann\n"
"Bastien Bouclet\n"
"Hans Breuer\n"
"Gustav Broberg\n"
"Christopher Brown\n"
"Marcus Brubaker\n"
"Luca Bruno\n"
"Nicu Buculei\n"
"Bulia Byak\n"
"Pierre Caclin\n"
"Ian Caldwell\n"
"Gail Carmichael\n"
"Ed Catmur\n"
"Chema Celorio\n"
"Jabiertxo Arraiza Cenoz\n"
"Johan Ceuppens\n"
"Zbigniew Chyla\n"
"Alexander Clausen\n"
"John Cliff\n"
"Kees Cook\n"
"Ben Cromwell\n"
"Robert Crosbie\n"
"Jon Cruz\n"
"Aurélie De-Cooman\n"
"Milosz Derezynski\n"
"Daniel Díaz\n"
"Bruno Dilly\n"
"Larry Doolittle\n"
"Nicolas Dufour\n"
"Tim Dwyer\n"
"Maxim V. Dziumanenko\n"
"Johan Engelen\n"
"Miklos Erdelyi\n"
"Ulf Erikson\n"
"Noé Falzon\n"
"Frank Felfe\n"
"Andrew Fitzsimon\n"
"Edward Flick\n"
"Marcin Floryan\n"
"Ben Fowler\n"
"Fred\n"
"Cedric Gemy\n"
"Steren Giannini\n"
"Olivier Gondouin\n"
"Ted Gould\n"
"Toine de Greef\n"
"Michael Grosberg\n"
"Kris De Gussem\n"
"Bryce Harrington\n"
"Dale Harvey\n"
"Aurélio Adnauer Heckert\n"
"Carl Hetherington\n"
"Jos Hirth\n"
"Hannes Hochreiner\n"
"Thomas Holder\n"
"Joel Holdsworth\n"
"Christoffer Holmstedt\n"
"Alan Horkan\n"
"Karl Ove Hufthammer\n"
"Richard Hughes\n"
"Nathan Hurst\n"
"inductiveload\n"
"Thomas Ingham\n"
"Jean-Olivier Irisson\n"
"Bob Jamison\n"
"Ted Janeczko\n"
"jEsuSdA\n"
"Fernando Lucchesi Bastos Jurema\n"
"Lauris Kaplinski\n"
"Lynn Kerby\n"
"Niko Kiirala\n"
"James Kilfiger\n"
"Nikita Kitaev\n"
"Jason Kivlighn\n"
"Adrian Knoth\n"
"Krzysztof Kosiński\n"
"Petr Kovar\n"
"Benoît Lavorata\n"
"Alex Leone\n"
"Julien Leray\n"
"Raph Levien\n"
"Diederik van Lierop\n"
"Nicklas Lindgren\n"
"Vitaly Lipatov\n"
"Ivan Louette\n"
"Pierre-Antoine Marc\n"
"Aurel-Aimé Marmion\n"
"Colin Marquardt\n"
"Craig Marshall\n"
"Ivan Masár\n"
"Dmitry G. Mastrukov\n"
"David Mathog\n"
"Matiphas\n"
"Michael Meeks\n"
"Federico Mena\n"
"MenTaLguY\n"
"Aubanel Monnier\n"
"Vincent Montagne\n"
"Tim Mooney\n"
"Derek P. Moore\n"
"Chris Morgan\n"
"Peter Moulder\n"
"Jörg Müller\n"
"Yukihiro Nakai\n"
"Victor Navez\n"
"Christian Neumair\n"
"Nick\n"
"Andreas Nilsson\n"
"Mitsuru Oka\n"
"Vinícius dos Santos Oliveira\n"
"Martin Owens\n"
"Alvin Penner\n"
"Matthew Petroff\n"
"Jon Phillips\n"
"Zdenko Podobny\n"
"Alexandre Prokoudine\n"
"Jean-René Reinhard\n"
"Alexey Remizov\n"
"Frederic Rodrigo\n"
"Hugo Rodrigues\n"
"Juarez Rudsatz\n"
"Xavier Conde Rueda\n"
"Felipe Corrêa da Silva Sanches\n"
"Christian Schaller\n"
"Marco Scholten\n"
"Tom von Schwerdtner\n"
"Danilo Šegan\n"
"Abhishek Sharma\n"
"Shivaken\n"
"Michael Sloan\n"
"John Smith\n"
"Boštjan Špetič\n"
"Aaron Spike\n"
"Kaushik Sridharan\n"
"Ralf Stephan\n"
"Dariusz Stojek\n"
"Martin Sucha\n"
"~suv\n"
"Pat Suwalski\n"
"Adib Taraben\n"
"Hugh Tebby\n"
"Jonas Termeau\n"
"David Turner\n"
"Andre Twupack\n"
"Aleksandar Urošević\n"
"Alex Valavanis\n"
"Joakim Verona\n"
"Lucas Vieites\n"
"Daniel Wagenaar\n"
"Liam P. White\n"
"Sebastian Wüst\n"
"Michael Wybrow\n"
"Gellule Xg\n"
"Daniel Yacob\n"
"Masatake Yamato\n"
"David Yip";

    //##############################
    //# T R A N S L A T O R S
    //##############################


    translators_text = "";
   
    // TRANSLATORS: Put here your name (and other national contributors')      
    // one per line in the form of: name surname (email). Use \n for newline.
    Glib::ustring thisTranslation = _("translator-credits");

    /**
     * See if the translators for the current language
     * made an entry for "translator-credits".  If it exists,
     * put it at the top of the window,  add some space between
     * it and the list of all translators.
     *      
     * NOTE:  Do we need 2 more .po entries for titles:
     *  "translators for this language"
     *  "all translators"  ??     
     */                          
    if (thisTranslation != "translator-credits") {
        translators_text.append(thisTranslation);
        translators_text.append("\n\n\n");
    }

    /* This text is copied from the TRANSLATORS file.
     * To update it, execute this snippet of sed magic in the toplevel
     * source directory:
     *
     * sed -e 's/^\(.*\) \([^ ]*\)*$/\2_ \1/' TRANSLATORS
           | sed -e 's/^\([^_]*\)_ \(.*\)$/\2 \1/;s/^.*$/"\0\\n"/;$s/\\n//'
           | zenity --text-info
     *
     * and paste the result from the combo box here.
     */          
    gchar const *allTranslators =
"3ARRANO.com <3arrano@3arrano.com>, 2005.\n"
"Adib Taraben <theadib@gmail.com>, 2004-2014.\n"
"Alan Monfort <alan.monfort@free.fr>, 2009-2010.\n"
"Alastair McKinstry <mckinstry@computer.org>, 2000.\n"
"Aleksandar Urošević <urke@users.sourceforge.net>, 2004-2006.\n"
"Alessio Frusciante <algol@firenze.linux.it>, 2002, 2003.\n"
"Alexander Shopov <ash@contact.bg>, 2006.\n"
"Alexandre Prokoudine <alexandre.prokoudine@gmail.com>, 2005, 2010-2014.\n"
"Alexey Remizov <alexey@remizov.pp.ru>, 2004.\n"
"Ali Ghanavatian <ghanvatian.ali@gmail.com>, 2010.\n"
"Álvaro Lopes <alvieboy@alvie.com>, 2001, 2002.\n"
"Andreas Hyden <a.hyden@cyberpoint.se>, 2000.\n"
"Andrius Ramanauskas <knutux@gmail.com>, 2006.\n"
"Antonio Codazzi <f_sophia@libero.it>, 2006, 2007.\n"
"Antônio Cláudio (LedStyle) <ledstyle@gmail.com>, 2006.\n"
"Amanpreet Singh Brar Alamwalia <apbrar@gmail.com>, 2005.\n"
"Arman Aksoy <armish@linux-sevenler.de>, 2003.\n"
"Arpad Biro <biro_arpad@yahoo.com>, 2004, 2005.\n"
"Benedikt Roth <Benedikt.Roth@gmx.net>, 2000.\n"
"Benno Schulenberg <benno@vertaalt.nl>,  2008.\n"
"Boštjan Špetič <igzebedze@cyberpipe.org>, 2004, 2005.\n"
"Brisa Francesco <fbrisa@yahoo.it>, 2000.\n"
"Bruce Cowan <bruce@bcowan.me.uk>, 2010.\n"
"bulia byak <buliabyak@users.sf.net>, 2004.\n"
"Chris jia <Chrisjiasl@gmail.com>, 2006.\n"
"Christian Meyer <chrisime@gnome.org>, 2000-2002.\n"
"Christian Neumair <chris@gnome-de.org>, 2002, 2003.\n"
"Christian Rose <menthos@menthos.com>, 2000-2003.\n"
"Cristian Secară <cristi@secarica.ro>, 2010-2013.\n"
"Christophe Merlet (RedFox) <redfox@redfoxcenter.org>, 2000-2002.\n"
"Clytie Siddall <clytie@riverland.net.au>, 2004-2008.\n"
"Colin Marquardt <colin@marquardt-home.de>, 2004-2006.\n"
"Cédric Gemy <radar.map35@free.fr>, 2006.\n"
"Daniel Díaz <yosoy@danieldiaz.org>, 2004.\n"
"Didier Conchaudron <conchaudron@free.fr>, 2003.\n"
"Dimitris Spingos <dmtrs32@gmail.com>, 2011-2013.\n"
"Dorji Tashi <dorjee_doss@hotmail.com>, 2006.\n"
"Duarte Loreto <happyguy_pt@hotmail.com> 2002, 2003 (Maintainer).\n"
"Elias Norberg <elno0959 at student.su.se>, 2009.\n"
"Equipe de Tradução Inkscape Brasil <www.inkscapebrasil.org>, 2007.\n"
"Fatih Demir <kabalak@gtranslator.org>, 2000.\n"
"Firas Hanife <FirasHanife@gmail.com>, 2014.\n"
"Foppe Benedictus <foppe.benedictus@gmail.com>, 2007-2009.\n"
"Francesc Dorca <f.dorca@filnet.es>, 2003. Traducció sodipodi.\n"
"Francisco Javier F. Serrador <serrador@arrakis.es>, 2003.\n"
"Francisco Xosé Vázquez Grandal <fxvazquez@arrakis.es>, 2001.\n"
"Frederic Rodrigo <f.rodrigo free.fr>, 2004-2005.\n"
"Ge'ez Frontier Foundation <locales@geez.org>, 2002.\n"
"George Boukeas <boukeas@gmail.com>, 2011.\n"
"Hleb Valoshka <375gnu@gmail.com>, 2008-2009.\n"
"Hizkuntza Politikarako Sailburuordetza <hizkpol@ej-gv.es>, 2005.\n"
"Ilia Penev <lichopicho@gmail.com>, 2006.\n"
"Ivan Masár <helix84@centrum.sk>, 2006-2014. \n"
"Ivan Řihošek <irihosek@seznam.cz>, 2014.\n"
"Iñaki Larrañaga <dooteo@euskalgnu.org>, 2006.\n"
"Jānis Eisaks <jancs@dv.lv>, 2012-2014.\n"
"Jeffrey Steve Borbón Sanabria <jeff_kerokid@yahoo.com>, 2005.\n"
"Jesper Öqvist <jesper@llbit.se>, 2010, 2011.\n"
"Joaquim Perez i Noguer <noguer@gmail.com>, 2008-2009.\n"
"Jörg Müller <jfm@ram-brand.de>, 2005.\n"
"Jeroen van der Vegt <jvdvegt@gmail.com>, 2003, 2005, 2008.\n"
"Jin-Hwan Jeong <yongdoria@gmail.com>, 2009.\n"
"Jonathan Ernst <jernst@users.sourceforge.net>, 2006.\n"
"Jose Antonio Salgueiro Aquino <developer@telefonica.net>, 2003.\n"
"Josef Vybiral <josef.vybiral@gmail.com>, 2005-2006.\n"
"Juarez Rudsatz <juarez@correio.com>, 2004.\n"
"Junichi Uekawa <dancer@debian.org>, 2002.\n"
"Jurmey Rabgay <jur_gay@yahoo.com>, 2006.\n"
"Kai Lahmann <kailahmann@01019freenet.de>, 2000.\n"
"Karl Ove Hufthammer <karl@huftis.org>, 2004, 2005.\n"
"KATSURAGAWA Naoki <naopon@private.email.ne.jp>, 2006.\n"
"Keld Simonsen <keld@dkuug.dk>, 2000, 2001.\n"
"Kenji Inoue <kenz@oct.zaq.ne.jp>, 2006-2007.\n"
"Khandakar Mujahidul Islam <suzan@bengalinux.org>, 2006.\n"
"Kingsley Turner <kingsley@maddogsbreakfast.com.au>, 2006.\n"
"Kitae <bluetux@gmail.com>, 2006.\n"
"Kjartan Maraas <kmaraas@gnome.org>, 2000-2002.\n"
"Kris De Gussem <Kris.DeGussem@gmail.com>, 2008-2014.\n"
"Lauris Kaplinski <lauris@ariman.ee>, 2000.\n"
"Leandro Regueiro <leandro.regueiro@gmail.com>, 2006-2008, 2010.\n"
"Liu Xiaoqin <liuxqsmile@gmail.com>, 2008.\n"
"Luca Bruno <luca.br@uno.it>, 2005.\n"
"Lucas Vieites Fariña<lucas@codexion.com>, 2003-2013.\n"
"Mahesh subedi <submanesh@hotmail.com>, 2006.\n"
"Martin Srebotnjak, <miles@filmsi.net>, 2005, 2010.\n"
"Masatake YAMATO <jet@gyve.org>, 2002.\n"
"Masato Hashimoto <cabezon.hashimoto@gmail.com>, 2009-2014.\n"
"Matiphas <matiphas _a_ free _point_ fr>, 2004-2006.\n"
"Mattias Hultgren <mattias_hultgren@tele2.se>, 2005, 2006.\n"
"Maxim Dziumanenko <mvd@mylinux.com.ua>, 2004.\n"
"Mətin Əmirov <metin@karegen.com>, 2003.\n"
"Mitsuru Oka <oka326@parkcity.ne.jp>, 2002.\n"
"Morphix User <pchitrakar@gmail.com>, 2006.\n"
"Mufit Eribol <meribol@ere.com.tr>, 2000.\n"
"Muhammad Bashir Al-Noimi <mhdbnoimi@gmail.com>, 2008.\n"
"Myckel Habets <myckel@sdf.lonestar.org>, 2008.\n"
"Nguyen Dinh Trung <nguyendinhtrung141@gmail.com>, 2007, 2008.\n"
"Nicolas Dufour <nicoduf@yahoo.fr>, 2008-2014.\n"
"Pawan Chitrakar <pchitrakar@gmail.com>, 2006.\n"
"Przemysław Loesch <p_loesch@poczta.onet.pl>, 2005.\n"
"Quico Llach <quico@softcatala.org>, 2000. Traducció sodipodi.\n"
"Raymond Ostertag <raymond@linuxgraphic.org>, 2002, 2003.\n"
"Riku Leino <tsoots@gmail.com>, 2006-2011.\n"
"Rune Rønde Laursen <runerl@skjoldhoej.dk>, 2006.\n"
"Ruud Steltenpool <svg@steltenpower.com>, 2006.\n"
"Serdar Soytetir <sendirom@gmail.com>, 2005.\n"
"shivaken <shivaken@owls-nest.net>, 2004.\n"
"Shyam Krishna Bal <shyamkrishna_bal@yahoo.com>, 2006.\n"
"Simos Xenitellis <simos@hellug.gr>, 2001, 2011.\n"
"Spyros Blanas <cid_e@users.sourceforge.net>, 2006, 2011.\n"
"Stefan Graubner <pflaumenmus92@gmx.net>, 2005.\n"
"Supranee Thirawatthanasuk <supranee@opentle.org>, 2006.\n"
"Takeshi Aihana <aihana@muc.biglobe.ne.jp>, 2000, 2001.\n"
"Tim Sheridan <tim.sheridan@gmail.com>, 2007-2011.\n"
"Theppitak Karoonboonyanan <thep@linux.thai.net>, 2006.\n"
"Thiago Pimentel <thiago.merces@gmail.com>, 2006.\n"
"Toshifumi Sato <sato@centrosystem.com>, 2005.\n"
"Jon South <striker@lunar-linux.org>, 2006. \n"
"Uwe Schöler <oss@oss-marketplace.com>, 2006-2014.\n"
"Valek Filippov <frob@df.ru>, 2000, 2003.\n"
"Victor Dachev <vdachev@gmail.com>, 2006.\n"
"Victor Westmann <victor.westmann@gmail.com>, 2011, 2014.\n"
"Ville Pätsi, 2013.\n"
"Vincent van Adrighem <V.vanAdrighem@dirck.mine.nu>, 2003.\n"
"Vital Khilko <dojlid@mova.org>, 2003.\n"
"Vitaly Lipatov <lav@altlinux.ru>, 2002, 2004.\n"
"vonHalenbach <vonHalenbach@users.sourceforge.net>, 2005.\n"
"Waluyo Adi Siswanto <was.uthm@gmail.com>, 2011.\n"
"Wang Li <charlesw1234@163.com>, 2002.\n"
"Wei-Lun Chao <william.chao@ossii.com.tw>, 2006.\n"
"Wolfram Strempfer <wolfram@strempfer.de>, 2006.\n"
"Xavier Conde Rueda <xavi.conde@gmail.com>, 2004-2008.\n"
"Yaron Shahrabani <sh.yaron@gmail.com>, 2009.\n"
"Yukihiro Nakai <nakai@gnome.gr.jp>, 2000, 2003.\n"
"Yuri Beznos <zhiz0id@gmail.com>, 2006.\n"
"Yuri Chornoivan <yurchor@ukr.net>, 2007-2014.\n"
"Yuri Syrota <rasta@renome.rovno.ua>, 2000.\n"
"Yves Guillou <yvesguillou@users.sourceforge.net>, 2004.\n"
"Zdenko Podobný <zdpo@mailbox.sk>, 2003, 2004."
    ;
    
    translators_text.append(allTranslators);



    //##############################
    //# L I C E N S E
    //##############################

    /**
     * This text is copied from the COPYING file
     */         
    license_text =
    "            GNU GENERAL PUBLIC LICENSE\n"
    "               Version 2, June 1991\n"
    "\n"
    " Copyright (C) 1989, 1991 Free Software Foundation, Inc.\n"
    "     59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
    " Everyone is permitted to copy and distribute verbatim copies\n"
    " of this license document, but changing it is not allowed.\n"
    "\n"
    "                Preamble\n"
    "\n"
    "  The licenses for most software are designed to take away your\n"
    "freedom to share and change it.  By contrast, the GNU General Public\n"
    "License is intended to guarantee your freedom to share and change free\n"
    "software--to make sure the software is free for all its users.  This\n"
    "General Public License applies to most of the Free Software\n"
    "Foundation's software and to any other program whose authors commit to\n"
    "using it.  (Some other Free Software Foundation software is covered by\n"
    "the GNU Library General Public License instead.)  You can apply it to\n"
    "your programs, too.\n"
    "\n"
    "  When we speak of free software, we are referring to freedom, not\n"
    "price.  Our General Public Licenses are designed to make sure that you\n"
    "have the freedom to distribute copies of free software (and charge for\n"
    "this service if you wish), that you receive source code or can get it\n"
    "if you want it, that you can change the software or use pieces of it\n"
    "in new free programs; and that you know you can do these things.\n"
    "\n"
    "  To protect your rights, we need to make restrictions that forbid\n"
    "anyone to deny you these rights or to ask you to surrender the rights.\n"
    "These restrictions translate to certain responsibilities for you if you\n"
    "distribute copies of the software, or if you modify it.\n"
    "\n"
    "  For example, if you distribute copies of such a program, whether\n"
    "gratis or for a fee, you must give the recipients all the rights that\n"
    "you have.  You must make sure that they, too, receive or can get the\n"
    "source code.  And you must show them these terms so they know their\n"
    "rights.\n"
    "\n"
    "  We protect your rights with two steps: (1) copyright the software, and\n"
    "(2) offer you this license which gives you legal permission to copy,\n"
    "distribute and/or modify the software.\n"
    "\n"
    "  Also, for each author's protection and ours, we want to make certain\n"
    "that everyone understands that there is no warranty for this free\n"
    "software.  If the software is modified by someone else and passed on, we\n"
    "want its recipients to know that what they have is not the original, so\n"
    "that any problems introduced by others will not reflect on the original\n"
    "authors' reputations.\n"
    "\n"
    "  Finally, any free program is threatened constantly by software\n"
    "patents.  We wish to avoid the danger that redistributors of a free\n"
    "program will individually obtain patent licenses, in effect making the\n"
    "program proprietary.  To prevent this, we have made it clear that any\n"
    "patent must be licensed for everyone's free use or not licensed at all.\n"
    "\n"
    "  The precise terms and conditions for copying, distribution and\n"
    "modification follow.\n"
    "\n"
    "            GNU GENERAL PUBLIC LICENSE\n"
    "   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n"
    "\n"
    "  0. This License applies to any program or other work which contains\n"
    "a notice placed by the copyright holder saying it may be distributed\n"
    "under the terms of this General Public License.  The \"Program\", below,\n"
    "refers to any such program or work, and a \"work based on the Program\"\n"
    "means either the Program or any derivative work under copyright law:\n"
    "that is to say, a work containing the Program or a portion of it,\n"
    "either verbatim or with modifications and/or translated into another\n"
    "language.  (Hereinafter, translation is included without limitation in\n"
    "the term \"modification\".)  Each licensee is addressed as \"you\".\n"
    "\n"
    "Activities other than copying, distribution and modification are not\n"
    "covered by this License; they are outside its scope.  The act of\n"
    "running the Program is not restricted, and the output from the Program\n"
    "is covered only if its contents constitute a work based on the\n"
    "Program (independent of having been made by running the Program).\n"
    "Whether that is true depends on what the Program does.\n"
    "\n"
    "  1. You may copy and distribute verbatim copies of the Program's\n"
    "source code as you receive it, in any medium, provided that you\n"
    "conspicuously and appropriately publish on each copy an appropriate\n"
    "copyright notice and disclaimer of warranty; keep intact all the\n"
    "notices that refer to this License and to the absence of any warranty;\n"
    "and give any other recipients of the Program a copy of this License\n"
    "along with the Program.\n"
    "\n"
    "You may charge a fee for the physical act of transferring a copy, and\n"
    "you may at your option offer warranty protection in exchange for a fee.\n"
    "\n"
    "  2. You may modify your copy or copies of the Program or any portion\n"
    "of it, thus forming a work based on the Program, and copy and\n"
    "distribute such modifications or work under the terms of Section 1\n"
    "above, provided that you also meet all of these conditions:\n"
    "\n"
    "    a) You must cause the modified files to carry prominent notices\n"
    "    stating that you changed the files and the date of any change.\n"
    "\n"
    "    b) You must cause any work that you distribute or publish, that in\n"
    "    whole or in part contains or is derived from the Program or any\n"
    "    part thereof, to be licensed as a whole at no charge to all third\n"
    "    parties under the terms of this License.\n"
    "\n"
    "    c) If the modified program normally reads commands interactively\n"
    "    when run, you must cause it, when started running for such\n"
    "    interactive use in the most ordinary way, to print or display an\n"
    "    announcement including an appropriate copyright notice and a\n"
    "    notice that there is no warranty (or else, saying that you provide\n"
    "    a warranty) and that users may redistribute the program under\n"
    "    these conditions, and telling the user how to view a copy of this\n"
    "    License.  (Exception: if the Program itself is interactive but\n"
    "    does not normally print such an announcement, your work based on\n"
    "    the Program is not required to print an announcement.)\n"
    "\n"
    "These requirements apply to the modified work as a whole.  If\n"
    "identifiable sections of that work are not derived from the Program,\n"
    "and can be reasonably considered independent and separate works in\n"
    "themselves, then this License, and its terms, do not apply to those\n"
    "sections when you distribute them as separate works.  But when you\n"
    "distribute the same sections as part of a whole which is a work based\n"
    "on the Program, the distribution of the whole must be on the terms of\n"
    "this License, whose permissions for other licensees extend to the\n"
    "entire whole, and thus to each and every part regardless of who wrote it.\n"
    "\n"
    "Thus, it is not the intent of this section to claim rights or contest\n"
    "your rights to work written entirely by you; rather, the intent is to\n"
    "exercise the right to control the distribution of derivative or\n"
    "collective works based on the Program.\n"
    "\n"
    "In addition, mere aggregation of another work not based on the Program\n"
    "with the Program (or with a work based on the Program) on a volume of\n"
    "a storage or distribution medium does not bring the other work under\n"
    "the scope of this License.\n"
    "\n"
    "  3. You may copy and distribute the Program (or a work based on it,\n"
    "under Section 2) in object code or executable form under the terms of\n"
    "Sections 1 and 2 above provided that you also do one of the following:\n"
    "\n"
    "    a) Accompany it with the complete corresponding machine-readable\n"
    "    source code, which must be distributed under the terms of Sections\n"
    "    1 and 2 above on a medium customarily used for software interchange; or,\n"
    "\n"
    "    b) Accompany it with a written offer, valid for at least three\n"
    "    years, to give any third party, for a charge no more than your\n"
    "    cost of physically performing source distribution, a complete\n"
    "    machine-readable copy of the corresponding source code, to be\n"
    "    distributed under the terms of Sections 1 and 2 above on a medium\n"
    "    customarily used for software interchange; or,\n"
    "\n"
    "    c) Accompany it with the information you received as to the offer\n"
    "    to distribute corresponding source code.  (This alternative is\n"
    "    allowed only for noncommercial distribution and only if you\n"
    "    received the program in object code or executable form with such\n"
    "    an offer, in accord with Subsection b above.)\n"
    "\n"
    "The source code for a work means the preferred form of the work for\n"
    "making modifications to it.  For an executable work, complete source\n"
    "code means all the source code for all modules it contains, plus any\n"
    "associated interface definition files, plus the scripts used to\n"
    "control compilation and installation of the executable.  However, as a\n"
    "special exception, the source code distributed need not include\n"
    "anything that is normally distributed (in either source or binary\n"
    "form) with the major components (compiler, kernel, and so on) of the\n"
    "operating system on which the executable runs, unless that component\n"
    "itself accompanies the executable.\n"
    "\n"
    "If distribution of executable or object code is made by offering\n"
    "access to copy from a designated place, then offering equivalent\n"
    "access to copy the source code from the same place counts as\n"
    "distribution of the source code, even though third parties are not\n"
    "compelled to copy the source along with the object code.\n"
    "\n"
    "  4. You may not copy, modify, sublicense, or distribute the Program\n"
    "except as expressly provided under this License.  Any attempt\n"
    "otherwise to copy, modify, sublicense or distribute the Program is\n"
    "void, and will automatically terminate your rights under this License.\n"
    "However, parties who have received copies, or rights, from you under\n"
    "this License will not have their licenses terminated so long as such\n"
    "parties remain in full compliance.\n"
    "\n"
    "  5. You are not required to accept this License, since you have not\n"
    "signed it.  However, nothing else grants you permission to modify or\n"
    "distribute the Program or its derivative works.  These actions are\n"
    "prohibited by law if you do not accept this License.  Therefore, by\n"
    "modifying or distributing the Program (or any work based on the\n"
    "Program), you indicate your acceptance of this License to do so, and\n"
    "all its terms and conditions for copying, distributing or modifying\n"
    "the Program or works based on it.\n"
    "\n"
    "  6. Each time you redistribute the Program (or any work based on the\n"
    "Program), the recipient automatically receives a license from the\n"
    "original licensor to copy, distribute or modify the Program subject to\n"
    "these terms and conditions.  You may not impose any further\n"
    "restrictions on the recipients' exercise of the rights granted herein.\n"
    "You are not responsible for enforcing compliance by third parties to\n"
    "this License.\n"
    "\n"
    "  7. If, as a consequence of a court judgment or allegation of patent\n"
    "infringement or for any other reason (not limited to patent issues),\n"
    "conditions are imposed on you (whether by court order, agreement or\n"
    "otherwise) that contradict the conditions of this License, they do not\n"
    "excuse you from the conditions of this License.  If you cannot\n"
    "distribute so as to satisfy simultaneously your obligations under this\n"
    "License and any other pertinent obligations, then as a consequence you\n"
    "may not distribute the Program at all.  For example, if a patent\n"
    "license would not permit royalty-free redistribution of the Program by\n"
    "all those who receive copies directly or indirectly through you, then\n"
    "the only way you could satisfy both it and this License would be to\n"
    "refrain entirely from distribution of the Program.\n"
    "\n"
    "If any portion of this section is held invalid or unenforceable under\n"
    "any particular circumstance, the balance of the section is intended to\n"
    "apply and the section as a whole is intended to apply in other\n"
    "circumstances.\n"
    "\n"
    "It is not the purpose of this section to induce you to infringe any\n"
    "patents or other property right claims or to contest validity of any\n"
    "such claims; this section has the sole purpose of protecting the\n"
    "integrity of the free software distribution system, which is\n"
    "implemented by public license practices.  Many people have made\n"
    "generous contributions to the wide range of software distributed\n"
    "through that system in reliance on consistent application of that\n"
    "system; it is up to the author/donor to decide if he or she is willing\n"
    "to distribute software through any other system and a licensee cannot\n"
    "impose that choice.\n"
    "\n"
    "This section is intended to make thoroughly clear what is believed to\n"
    "be a consequence of the rest of this License.\n"
    "\n"
    "  8. If the distribution and/or use of the Program is restricted in\n"
    "certain countries either by patents or by copyrighted interfaces, the\n"
    "original copyright holder who places the Program under this License\n"
    "may add an explicit geographical distribution limitation excluding\n"
    "those countries, so that distribution is permitted only in or among\n"
    "countries not thus excluded.  In such case, this License incorporates\n"
    "the limitation as if written in the body of this License.\n"
    "\n"
    "  9. The Free Software Foundation may publish revised and/or new versions\n"
    "of the General Public License from time to time.  Such new versions will\n"
    "be similar in spirit to the present version, but may differ in detail to\n"
    "address new problems or concerns.\n"
    "\n"
    "Each version is given a distinguishing version number.  If the Program\n"
    "specifies a version number of this License which applies to it and \"any\n"
    "later version\", you have the option of following the terms and conditions\n"
    "either of that version or of any later version published by the Free\n"
    "Software Foundation.  If the Program does not specify a version number of\n"
    "this License, you may choose any version ever published by the Free Software\n"
    "Foundation.\n"
    "\n"
    "  10. If you wish to incorporate parts of the Program into other free\n"
    "programs whose distribution conditions are different, write to the author\n"
    "to ask for permission.  For software which is copyrighted by the Free\n"
    "Software Foundation, write to the Free Software Foundation; we sometimes\n"
    "make exceptions for this.  Our decision will be guided by the two goals\n"
    "of preserving the free status of all derivatives of our free software and\n"
    "of promoting the sharing and reuse of software generally.\n"
    "\n"
    "                NO WARRANTY\n"
    "\n"
    "  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
    "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
    "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
    "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
    "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
    "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
    "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
    "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
    "REPAIR OR CORRECTION.\n"
    "\n"
    "  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
    "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
    "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
    "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
    "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
    "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
    "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
    "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
    "POSSIBILITY OF SUCH DAMAGES.\n"
    "\n"
    "             END OF TERMS AND CONDITIONS\n"
    "\n"
    "        How to Apply These Terms to Your New Programs\n"
    "\n"
    "  If you develop a new program, and you want it to be of the greatest\n"
    "possible use to the public, the best way to achieve this is to make it\n"
    "free software which everyone can redistribute and change under these terms.\n"
    "\n"
    "  To do so, attach the following notices to the program.  It is safest\n"
    "to attach them to the start of each source file to most effectively\n"
    "convey the exclusion of warranty; and each file should have at least\n"
    "the \"copyright\" line and a pointer to where the full notice is found.\n"
    "\n"
    "    <one line to give the program's name and a brief idea of what it does.>\n"
    "    Copyright (C) <year>  <name of author>\n"
    "\n"
    "    This program is free software; you can redistribute it and/or modify\n"
    "    it under the terms of the GNU General Public License as published by\n"
    "    the Free Software Foundation; either version 2 of the License, or\n"
    "    (at your option) any later version.\n"
    "\n"
    "    This program is distributed in the hope that it will be useful,\n"
    "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "    GNU General Public License for more details.\n"
    "\n"
    "    You should have received a copy of the GNU General Public License\n"
    "    along with this program; if not, write to the Free Software\n"
    "    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
    "\n"
    "\n"
    "Also add information on how to contact you by electronic and paper mail.\n"
    "\n"
    "If the program is interactive, make it output a short notice like this\n"
    "when it starts in an interactive mode:\n"
    "\n"
    "    Gnomovision version 69, Copyright (C) year  name of author\n"
    "    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
    "    This is free software, and you are welcome to redistribute it\n"
    "    under certain conditions; type `show c' for details.\n"
    "\n"
    "The hypothetical commands `show w' and `show c' should show the appropriate\n"
    "parts of the General Public License.  Of course, the commands you use may\n"
    "be called something other than `show w' and `show c'; they could even be\n"
    "mouse-clicks or menu items--whatever suits your program.\n"
    "\n"
    "You should also get your employer (if you work as a programmer) or your\n"
    "school, if any, to sign a \"copyright disclaimer\" for the program, if\n"
    "necessary.  Here is a sample; alter the names:\n"
    "\n"
    "  Yoyodyne, Inc., hereby disclaims all copyright interest in the program\n"
    "  `Gnomovision' (which makes passes at compilers) written by James Hacker.\n"
    "\n"
    "  <signature of Ty Coon>, 1 April 1989\n"
    "  Ty Coon, President of Vice\n"
    "\n"
    "This General Public License does not permit incorporating your program into\n"
    "proprietary programs.  If your program is a subroutine library, you may\n"
    "consider it more useful to permit linking proprietary applications with the\n"
    "library.  If this is what you want to do, use the GNU Library General\n"
    "Public License instead of this License.\n"
    ;

}




} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
