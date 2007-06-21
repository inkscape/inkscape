#!/bin/sh

params=`grep "^%%DocumentMedia:" "$1" | head -n 1 | awk '{  if (NF==7) { if ($2!="plain") print "-sPAPERSIZE=" $2; else print "-dDEVICEWIDTHPOINTS=" $3 " -dDEVICEHEIGHTPOINTS=" $4; } }'`

if [ "x${params}x" = "xx" ]
then 
        params=`grep "^%%BoundingBox:" "$1" | head -n 1 | awk '{ print "-dDEVICEWIDTHPOINTS=" $4 " -dDEVICEHEIGHTPOINTS=" $5; }'`
fi

exec ps2pdf $params "$1" - 2> /dev/null
