#include <glibmm/ustring.h>
#include <glib-object.h>
#include <vector>

struct Urange{
	gchar* start;
	gchar* end;
};

class UnicodeRange{
public:
UnicodeRange(const gchar* val);
int add_range(gchar* val);
bool contains(gchar unicode);
Glib::ustring attribute_string();
gunichar sample_glyph();

private:
std::vector<Urange> range;
std::vector<gunichar> unichars;
};

