#ifndef GDL_DATA_MODEL_TEST_H
#define GDL_DATA_MODEL_TEST_H

#include <glib.h>
#include "gdl-data-model.h"

G_BEGIN_DECLS

#define GDL_TYPE_DATA_MODEL_TEST            (gdl_data_model_test_get_type ())
#define GDL_DATA_MODEL_TEST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DATA_MODEL_TEST, GdlDataModelTest))
#define GDL_IS_DATA_MODEL_TEST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DATA_MODEL_TEST))


typedef struct _GdlDataModelTest      GdlDataModelTest;
typedef struct _GdlDataModelTestClass GdlDataModelTestClass;

struct _GdlDataModelTest {
	GObject parent;

	int stamp;
};

struct _GdlDataModelTestClass {
	GObjectClass parent_class;
};

GType             gdl_data_model_test_get_type (void);
GdlDataModelTest *gdl_data_model_test_new      (void);

G_END_DECLS

#endif
