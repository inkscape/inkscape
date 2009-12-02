#include "connection-points.h"


bool ConnectionPoint::operator!=(ConnectionPoint& cp)
{
    return (id!=cp.id || type!=cp.type || dir!=cp.dir || pos!=cp.pos);
}

bool ConnectionPoint::operator==(ConnectionPoint& cp)
{
    return (id==cp.id && type==cp.type && dir==cp.dir && pos==cp.pos);
}


namespace Inkscape{

SVGIStringStream&
operator>>(SVGIStringStream& istr, ConnectionPoint& cp)
{
    istr>>cp.id>>cp.dir>>cp.pos[Geom::X]>>cp.pos[Geom::Y];

    return istr;
}

SVGOStringStream&
operator<<(SVGOStringStream& ostr, const ConnectionPoint& cp)
{
    ostr<<cp.id<<' '<<cp.dir<<' ';
    ::operator<<( ostr, cp.pos[Geom::X] );
    ostr<<' ';
    ::operator<<( ostr, cp.pos[Geom::Y] );

    return ostr;
}


}
