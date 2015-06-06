import os

IGNORE = (
    # dirs
    "/cxxtest/",
    "/dom/work/",
    "/extension/dbus/",
    "/src/extension/dxf2svg/",
    "/test/",

    # files
    "buildtool.cpp",
    "src/inkscape-x64.rc",
    "src/inkview-x64.rc",
    "packaging/macosx/ScriptExec/main.c",
    "share/ui/keybindings.rc",
    "src/2geom/conic_section_clipper_impl.cpp",
    "src/2geom/conicsec.cpp",
    "src/2geom/recursive-bezier-intersection.cpp",
    "src/deptool.cpp",
    "src/display/nr-filter-skeleton.cpp",
    "src/display/testnr.cpp",
    "src/dom/io/httpclient.cpp",
    "src/dom/odf/SvgOdg.cpp",
    "src/dom/xmlwriter.cpp",
    "src/inkview.cpp",
    "src/inkview.rc",
    "src/io/streamtest.cpp",
    "src/libcola/cycle_detector.cpp",
    "src/libnr/nr-compose-reference.cpp",
    "src/libnr/testnr.cp",
    "src/live_effects/lpe-skeleton.cpp",
    "src/svg/test-stubs.cpp",
    "src/winconsole.cpp",

    # header files
    "share/filters/filters.svg.h",
    "share/palettes/palettes.h",
    "share/patterns/patterns.svg.h",
    "share/templates/templates.h",
    "share/symbols/symbols.h",
    "src/libcola/cycle_detector.h",
    "src/libnr/in-svg-plane-test.h",
    "src/libnr/nr-point-fns-test.h",
    "src/libnr/nr-translate-test.h",
    "src/svg/test-stubs.h",
    
    # generated files, created by an in-source build
    "CMakeFiles/CompilerIdC/CMakeCCompilerId.c",
    "CMakeFiles/CompilerIdCXX/CMakeCXXCompilerId.cpp",
    "src/helper/sp-marshal.cpp",
    "src/helper/sp-marshal.h",
    "src/inkscape-version.cpp",
    "config.h",
    )

UTF8_CHECK = False

SOURCE_DIR = os.path.normpath(os.path.abspath(os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))))
