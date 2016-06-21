/*
 * Test fixture with SPDocument per entire test case.
 *
 * Author:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2015 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "doc-per-case-test.h"

#include "inkscape.h"

SPDocument *DocPerCaseTest::_doc = 0;

DocPerCaseTest::DocPerCaseTest() :
    ::testing::Test()
{
}

void DocPerCaseTest::SetUpTestCase()
{
    if ( !Inkscape::Application::exists() )
    {
        // Create the global inkscape object.
        Inkscape::Application::create("", false);
    }

    _doc = SPDocument::createNewDoc( NULL, TRUE, true );
    ASSERT_TRUE( _doc != NULL );
}

void DocPerCaseTest::TearDownTestCase()
{
    if (_doc) {
        _doc->doUnref();
        _doc = NULL;
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
