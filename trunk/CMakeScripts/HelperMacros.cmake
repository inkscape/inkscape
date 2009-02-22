# A macro to replace slashes and spaces in a string with underscores
MACRO(SANITIZE_PATH _string_var)
  STRING(REGEX REPLACE "[\\/ ]+" "_" ${_string_var} ${${_string_var}})
ENDMACRO(SANITIZE_PATH _string_var)

# A macro to prepend a given string onto the beginning of each string in a list
MACRO(PREPEND _list _str)
  SET(_temp_list ${${_list}})
  SET(${_list})
  FOREACH(x ${_temp_list})
    SET(${_list} ${${_list}} ${_str}${x})
  ENDFOREACH(x)
ENDMACRO(PREPEND _list _str)
