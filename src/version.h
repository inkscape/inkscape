/*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2003 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_VERSION_H
#define SEEN_INKSCAPE_VERSION_H

#include <glib/gtypes.h>

#define SVG_VERSION "1.1"

namespace Inkscape {

struct Version {
	Version() {}
	Version(unsigned mj, unsigned mn) {
		// somebody pollutes our namespace with major() and minor()
		// macros, so we can't use new-style initializers
		major = mj;
		minor = mn;
	}

	unsigned major;
	unsigned minor;

	bool operator>(Version const &other) const {
		return major > other.major ||
		       ( major == other.major && minor > other.minor );
	}
	bool operator==(Version const &other) const {
		return major == other.major && minor == other.minor;
	}
	bool operator!=(Version const &other) const {
		return major != other.major || minor != other.minor;
	}
	bool operator<(Version const &other) const {
		return major < other.major ||
		       ( major == other.major && minor < other.minor );
	}
};

}

#define SP_VERSION_IS_ZERO (v) (!(v).major && !(v).minor)

gboolean sp_version_from_string (const gchar *string, Inkscape::Version *version);
gchar *sp_version_to_string (Inkscape::Version version);
gboolean sp_version_inside_range (Inkscape::Version version,
                                  unsigned major_min, unsigned minor_min,
                                  unsigned major_max, unsigned minor_max);

#endif
