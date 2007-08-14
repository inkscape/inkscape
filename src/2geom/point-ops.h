//[[[cog
import operators

setContext("Point", "Matrix", "Point")
make({'*':'*='}, {'/':'/='})
apsnd({'*':'/'}, "b.inverse()")

setContext("Point", "double", "Point")
make({'*=':'*'}, {'/=':'/'}, {'*':'*'}, {'*':'/'})

setContext("Point", "Point", "bool")
make({'==':'!='})

setContext("Point", "Point", "Point")
make({'+=':'+', '-=':'-'})
]]]

**************
GENERATED CODE
**************
If you wish to modify, move function out of generation region and remove the
cause of its generation.
*/

//[[[end]]]
