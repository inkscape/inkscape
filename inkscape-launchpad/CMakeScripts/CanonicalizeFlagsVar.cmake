# This file is copyright by Shlomi Fish, 2016.
#
# This file is licensed under the MIT/X11 license:
# https://opensource.org/licenses/mit-license.php

macro (canonicalize_flags_var in_val out_var)
    string(REPLACE " " ";" _c "${in_val}")
    list(REMOVE_DUPLICATES _c)
    list(SORT _c)
    string(REPLACE ";" " " "${out_var}" "${_c}")
endmacro()
