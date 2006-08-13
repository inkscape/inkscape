/** \file
 * PDF Mini library.
 */
/*
 * Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Minimalistic library to support generation of PDF files.
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef PDF_MINI_H_SEEN
#define PDF_MINI_H_SEEN


class PdfFile;
class PdfXref;
class PdfObject;

enum pdf_resource_type { pdf_none, pdf_extgstate, pdf_colorspace, pdf_pattern, pdf_shading, pdf_xobject, pdf_font, pdf_properties };

char pdf_res_short[][5] = { "", "GS", "CS", "Pa", "Sh", "XObj", "F", "Prop" };
char pdf_res_long[][20] = { "", "ExtGState", "ColorSpace", "Pattern",
                      "Shading", "XObject", "Font", "Properties" };

class PdfFile {
  public:
    PdfFile(FILE *_stream);
    ~PdfFile();

    long puts(const char *str);
    long puts(std::string ss) { return puts(ss.c_str()); }
    long puts(Inkscape::SVGOStringStream &os) { return puts(os.str().c_str()); }
    long puts(PdfXref *table);
    long puts(PdfObject *object);
    
    long tell();
    
    PdfObject *begin_document(double version = 1.2);
    void end_document(PdfObject *doc_info);
    PdfObject *begin_page(double x0, double y0, double x1, double y1);
    PdfObject *begin_page(double w, double h) { return begin_page(0,0, w, h); }
    void end_page(PdfObject *page);
    PdfObject *begin_object();
    void end_object(PdfObject *object);
    PdfObject *begin_resource(enum pdf_resource_type);
    void end_resource(PdfObject *resource);
    
  protected:
    FILE *fp;
    long length;
    long stream_pos;

    PdfXref *xref;

    PdfXref *pages;
    PdfXref *resources;

    PdfObject *obj_root;
    PdfObject *obj_catalog;
    PdfObject *obj_info;
    PdfObject *obj_resources;
    PdfObject *obj_contents;
    PdfObject *obj_length;
};


class PdfXref {
  public:
    PdfXref() { table.push_back(0); }
    ~PdfXref() { table.clear(); }

    long get_size() { return (long) table.size() - 1; }
    long get_entry(long id) { return table.at(id); }
    
    void update_entry(long id, long offset) {
        table.at(id) = offset;
    }
    
    long add_entry(long offset = 0) {
        table.push_back(offset);
        return get_size();
    }

  protected:
    std::vector<long> table;
};


class PdfObject : public Inkscape::SVGOStringStream {
  public:
    PdfObject(long i) : next(NULL), name(NULL) { id = i; setf(std::ios::fixed); }
    ~PdfObject() { if (name) g_free(name); }

    long get_id() { return id; }
    const char *get_name() { return name; }
    long get_length() { return str().length(); }
    void set_type(enum pdf_resource_type type) {
        if (name) g_free(name);
        name = g_strdup_printf("/%s%ld", pdf_res_short[type], id);
    }
    PdfObject *next;

  protected:
    long id;
    gchar *name;
};

PdfFile::PdfFile(FILE *_stream) {
    fp = _stream;
    xref = new PdfXref();
}

PdfFile::~PdfFile() {
    delete xref;
}

long PdfFile::puts(const char *str) {
    long res = fprintf(fp, "%s", str);
    if (res >= 0) {
        length += res;
        res = length;
    }
    return res;
}

long PdfFile::puts(PdfObject *object) {
    xref->update_entry( object->get_id(), tell() );
    return puts(object->str());
}

long PdfFile::puts(PdfXref *table) {
    Inkscape::SVGOStringStream os;
    char buffer[32];
    int i;

    int size = table->get_size() + 1;

    os << "xref\n";
    os << "0" << " " << size << "\n";

    snprintf(buffer, sizeof(buffer), "%010d %05d %c \n", 0, 65535, 'f');
    buffer[sizeof(buffer)-1] = 0;
    os << buffer;

    for (i = 1; i < size; i++) {
        snprintf(buffer, sizeof(buffer), "%010d %05d %c \n", (int)table->get_entry(i), 0, 'n');
	buffer[sizeof(buffer)-1] = 0;
        os << buffer;
    }

    return puts(os);
}

long PdfFile::tell() {
    return length;
}

PdfObject *PdfFile::begin_document(double version) {
    Inkscape::SVGOStringStream os;
    char bin[5] = {0x80|'B', 0x80|'i', 0x80|'n', 0x80|'!', 0};
    length = 0;
    pages = new PdfXref();;

    os << "%PDF-" << version << "\n";
    os << "%" << bin << "\n";

    obj_info = begin_object();
    *obj_info << "<<\n";

    obj_catalog = begin_object();

    obj_root = begin_object();
    *obj_root << "<<\n";
    *obj_root << "  /Type /Catalog\n";
    *obj_root << "  /Pages " << obj_catalog->get_id() << " 0 R\n";
    *obj_root << ">>\n";
    end_object(obj_root);

    puts(os);
    puts(obj_root);

    return obj_info;
}

void PdfFile::end_document(PdfObject *doc_info) {
    Inkscape::SVGOStringStream os;
    long startxref;
    int i;

    *doc_info << ">>\n";
    end_object(doc_info);
    puts(doc_info);
    
    *obj_catalog << "<<\n"
                 << "  /Type /Pages\n"
                 << "  /Count " << pages->get_size() << "\n"
                 << "  /Kids [\n";
    for (i=1; i<=pages->get_size(); i++) {
        *obj_catalog << "    " << pages->get_entry(i) << " 0 R\n";
    }
    *obj_catalog << "  ]\n"
                 << ">>\n";
    end_object(obj_catalog);
    puts(obj_catalog);
    
    startxref = tell();

    puts(xref);
    
    os << "trailer\n"
       << "<<\n"
       << "  /Size " << xref->get_size() << "\n"
       << "  /Root " << obj_root->get_id() << " 0 R\n"
       << "  /Info " << obj_info->get_id() << " 0 R\n"
       << ">>\n";
    os << "startxref\n"
       << startxref << "\n";
    os << "%%EOF\n";

    puts(os);

    delete pages;
    delete obj_root;
    delete obj_catalog;
    delete obj_info;
}

PdfObject *PdfFile::begin_page(double x0, double y0, double x1, double y1) {
    Inkscape::SVGOStringStream os;

    resources = new PdfXref[11]();
    PdfObject *obj_page = begin_object();
   
    obj_resources = begin_object();
    obj_contents  = begin_object();
    obj_length = begin_object();
    
    *obj_page << "<<\n"
              << "  /Type /Page\n"
              << "  /Parent " << obj_catalog->get_id() << " 0 R\n"
              << "  /MediaBox [ " << x0 << " " << y0 << " " << x1 << " " << y1 << " ]\n"
              << "  /Resources " << obj_resources->get_id() << " 0 R\n"
              << "  /Contents " << obj_contents->get_id() << " 0 R\n"
              << "  /Group\n"
              << "  << /Type /Group\n"
              << "     /S /Transparency\n"
              << "     /CS /DeviceRGB\n"
              << "  >>\n"
              << ">>\n";
    end_object(obj_page);
    puts(obj_page);

    pages->add_entry( obj_page->get_id() );

    *obj_contents << "<<\n"
                  << "  /Length " << obj_length->get_id() << " 0 R\n"
                  << ">>\n"
                  << "stream\n";
    puts(obj_contents);
    stream_pos = ftell(fp);

    return obj_contents;
}

void PdfFile::end_page(PdfObject *page) {
    long stream_length = ftell(fp) - stream_pos;
    
    puts("endstream\n");
    puts("endobj\n");
    
    *obj_length << stream_length << "\n";
    end_object(obj_length);
    puts(obj_length);

    *obj_resources << "<<\n";
    *obj_resources << "  /ProcSet [/PDF /Text /ImageB /ImageC /ImageI]\n";
    int res;
    for (res=pdf_extgstate; res<=pdf_properties; res++) {
        if (resources[res].get_size()) {
            *obj_resources << "  /" << pdf_res_long[res] << "\n";
            *obj_resources << "  <<\n";
            int i;
            for (i=1; i<=resources[res].get_size(); i++) {
                int id = resources[res].get_entry(i);
                *obj_resources << "    /"
                               << pdf_res_short[res] << id
                               << " " << id << " 0 R\n";
            }
            *obj_resources << "  >>\n";
        }
    }
    *obj_resources << ">>\n";
    end_object(obj_resources);
    puts(obj_resources);

    PdfObject *o = obj_resources->next;
    while (o) {
        PdfObject *t = o->next;
        puts(o);
        delete o;
        o = t;
    }
    
    delete obj_resources;
    delete obj_length;
    delete obj_contents;
    
    delete[] resources;
}

PdfObject *PdfFile::begin_object() {
    long id = xref->add_entry();
    PdfObject *obj = new PdfObject(id);
    *obj << id << " 0 obj\n";
    return obj;
}

void PdfFile::end_object(PdfObject *object) {
    *object << "endobj\n";
}

PdfObject *PdfFile::begin_resource(enum pdf_resource_type res) {
    PdfObject *obj = begin_object();
    if (res != pdf_none) {
        resources[res].add_entry(obj->get_id());
        obj->set_type(res);
    }
    obj->next = obj_resources->next;
    obj_resources->next = obj;
    return obj;
}

void PdfFile::end_resource(PdfObject *resource) {
    end_object(resource);
}


#endif /* !PDF_MINI_H_SEEN */
