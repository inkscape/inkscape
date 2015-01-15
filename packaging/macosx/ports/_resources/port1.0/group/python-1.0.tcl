# -*- coding: utf-8; mode: tcl; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- vim:fenc=utf-8:ft=tcl:et:sw=4:ts=4:sts=4
# $Id: python-1.0.tcl 118101 2014-03-22 15:16:34Z jmr@macports.org $
#
# Copyright (c) 2011 The MacPorts Project
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of The MacPorts Project nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Usage:
# name should be of the form py-foo for modules
# subports pyXY-foo are declared for each XY in python.versions

# for apps (i.e. not named py-foo), no subports will be defined
# only the python.default_version will be used
# you can change that in variants if you want

# options:
# python.versions: which versions this module supports, e.g. "26 27 31"
#   always set this (even if you have your own subport blocks)
# python.default_version: which version will be installed if the user asks
#   for py-foo rather than pyXY-foo
#
# Note: setting these options requires name to be set beforehand

categories      python

use_configure   no
# we want the default universal variant added despite not using configure
universal_variant yes

build.target    build

pre-destroot    {
    xinstall -d -m 755 ${destroot}${prefix}/share/doc/${subport}/examples
}

options python.versions python.version python.default_version
option_proc python.versions python_set_versions
default python.default_version {[python_get_default_version]}
default python.version {[python_get_version]}

proc python_get_version {} {
    if {[string match py-* [option name]]} {
        return [string range [option subport] 2 3]
    } else {
        return [option python.default_version]
    }
}
proc python_get_default_version {} {
    global python.versions
    if {[info exists python.versions]} {
        if {[lsearch -exact ${python.versions} 27] != -1} {
            return 27
        } else {
            return [lindex ${python.versions} end]
        }
    } else {
        return 27
    }
}

proc python_set_versions {option action args} {
    if {$action ne "set"} {
        return
    }
    global name subport python._addedcode
    if {[string match py-* $name]} {
        foreach v [option $option] {
            subport py${v}[string trimleft $name py] { depends_lib port:python${v} }
        }
        if {$subport eq $name || $subport eq ""} {
            # set up py-foo as a stub port that depends on the default pyXY-foo
            fetch {}
            checksum {}
            extract {}
            supported_archs noarch
            global python.default_version python.version
            unset python.version
            depends_lib port:py${python.default_version}[string trimleft $name py]
            patch {}
            build {}
            destroot {
                system "echo $name is a stub port > ${destroot}${prefix}/share/doc/${name}/README"
            }
        } else {
            set addcode 1
        }
    } else {
        set addcode 1
    }
    if {[info exists addcode] && ![info exists python._addedcode]} {
        pre-build {
            if {${python.add_archflags}} {
                if {[variant_exists universal] && [variant_isset universal]} {
                    build.env-append CFLAGS="${configure.universal_cflags}" \
                                     OBJCFLAGS="${configure.universal_cflags}" \
                                     CXXFLAGS="${configure.universal_cxxflags}" \
                                     LDFLAGS="${configure.universal_ldflags}"
                } else {
                    build.env-append CFLAGS="${configure.cc_archflags}" \
                                     OBJCFLAGS="${configure.objc_archflags}" \
                                     CXXFLAGS="${configure.cxx_archflags}" \
                                     FFLAGS="${configure.f77_archflags}" \
                                     F90FLAGS="${configure.f90_archflags}" \
                                     FCFLAGS="${configure.fc_archflags}" \
                                     LDFLAGS="${configure.ld_archflags}"
                }
            }
            if {${python.set_compiler}} {
                foreach var {cc objc cxx fc f77 f90} {
                    if {[set configure.${var}] ne ""} {
                        build.env-append [string toupper $var]="[set configure.${var}]"
                    }
                }
            }
        }
        post-destroot {
            if {${python.link_binaries}} {
                foreach bin [glob -nocomplain -tails -directory "${destroot}${python.prefix}/bin" *] {
                    if {[catch {file type "${destroot}${prefix}/bin/${bin}${python.link_binaries_suffix}"}]} {
                        ln -s "${python.prefix}/bin/${bin}" "${destroot}${prefix}/bin/${bin}${python.link_binaries_suffix}"
                    }
                }
            }
            if {${python.move_binaries}} {
                foreach bin [glob -nocomplain -tails -directory "${destroot}${prefix}/bin" *] {
                    move ${destroot}${prefix}/bin/${bin} \
                        ${destroot}${prefix}/bin/${bin}${python.move_binaries_suffix}
                }
            }
        }
        set python._addedcode 1
    }
}

option_proc python.default_version python_set_default_version
proc python_set_default_version {option action args} {
    if {$action ne "set"} {
        return
    }
    global name subport python.default_version
    if {[string match py-* $name]} {
        if {$subport eq $name || $subport eq ""} {
            depends_lib port:py${python.default_version}[string trimleft $name py]
        }
    } else {
        python.versions ${python.default_version}
        depends_lib-append port:python[option python.default_version]
    }
}


options python.branch python.prefix python.bin python.lib python.libdir \
        python.include python.pkgd
# for pythonXY, python.branch is X.Y
default python.branch {[string range ${python.version} 0 end-1].[string index ${python.version} end]}
default python.prefix {[python_get_defaults prefix]}
default python.bin  {[python_get_defaults bin]}
default python.lib  {[python_get_defaults lib]}
default python.pkgd {[python_get_defaults pkgd]}
default python.libdir {${python.prefix}/lib/python${python.branch}}
default python.include  {[python_get_defaults include]}

default build.cmd       {"${python.bin} setup.py [python_get_defaults setup_args]"}
default destroot.cmd    {"${python.bin} setup.py [python_get_defaults setup_args]"}
default destroot.destdir {"--prefix=[python_get_defaults setup_prefix] --root=${destroot}"}

proc python_get_defaults {var} {
    global python.version python.branch prefix python.prefix
    switch -- $var {
        prefix {
            global build_arch frameworks_dir
            set ret "${frameworks_dir}/Python.framework/Versions/${python.branch}"
            if {${python.version} == 25 || (${python.version} == 24 &&
                ![file isfile ${ret}/include/python${python.branch}/Python.h] &&
                ([file isfile ${prefix}/include/python${python.branch}/Python.h]
                || [string match *64* $build_arch]))} {
                set ret $prefix
            }
            return $ret
        }
        bin {
            if {${python.version} != 24} {
                return "${python.prefix}/bin/python${python.branch}"
            } else {
                return "${prefix}/bin/python${python.branch}"
            }
        }
        include {
            set inc_dir "${python.prefix}/include/python${python.branch}"
            if {[file exists ${inc_dir}]} {
                return ${inc_dir}
            } else {
                # look for "${inc_dir}*" and pick the first one found;
                # make assumptions if none are found
                if {[catch {set inc_dirs [glob ${inc_dir}*]}]} {
                    if {${python.version} < 30} {
                        return ${inc_dir}
                    } else {
                        return ${inc_dir}m
                    }
                } else {
                    return [lindex ${inc_dirs} 0]
                }
            }
        }
        lib {
            if {${python.version} != 24 && ${python.version} != 25} {
                return "${python.prefix}/Python"
            } else {
                return "${prefix}/lib/lib${python.branch}.dylib"
            }
        }
        pkgd {
            if {${python.version} != 24} {
                return "${python.prefix}/lib/python${python.branch}/site-packages"
            } else {
                return "${prefix}/lib/python${python.branch}/site-packages"
            }
        }
        setup_args {
            if {${python.version} != 24} {
                return "--no-user-cfg"
            } else {
                return ""
            }
        }
        setup_prefix {
            if {${python.version} != 24} {
                return "${python.prefix}"
            } else {
                return "${prefix}"
            }
        }
        link_binaries {
            if {${python.version} != 24 && ${python.version} != 25} {
                return yes
            } else {
                return no
            }
        }
        move_binaries {
            if {${python.version} == 24 || ${python.version} == 25} {
                return yes
            } else {
                return no
            }
        }
        default {
            error "unknown option $var"
        }
    }
}

options python.add_archflags
default python.add_archflags yes
options python.set_compiler
default python.set_compiler yes

options python.link_binaries python.link_binaries_suffix
default python.link_binaries {[python_get_defaults link_binaries]}
default python.link_binaries_suffix {-${python.branch}}

options python.move_binaries python.move_binaries_suffix
default python.move_binaries {[python_get_defaults move_binaries]}
default python.move_binaries_suffix {-${python.branch}}
