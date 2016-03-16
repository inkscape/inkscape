'''
Copyright (C) 2011 Karlisson Bezerra <contact@hacktoon.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''

import inkex
import simplestyle
from simplepath import parsePath
from simpletransform import parseTransform

class Element:
    def attr(self, val, ns=""):
        if ns:
            val = inkex.addNS(val, ns)
        try:
            attr = float(self.node.get(val))
        except:
            attr = self.node.get(val)
        return attr


class GradientDef(Element):
    def __init__(self, node, stops):
        self.node = node
        self.stops = stops


class LinearGradientDef(GradientDef):
    def get_data(self):
        x1 = self.attr("x1")
        y1 = self.attr("y1")
        x2 = self.attr("x2")
        y2 = self.attr("y2")
        #self.createLinearGradient(href, x1, y1, x2, y2)

    def draw(self):
        pass


class RadialGradientDef(GradientDef):
    def get_data(self):
        cx = self.attr("cx")
        cy = self.attr("cy")
        r = self.attr("r")
        #self.createRadialGradient(href, cx, cy, r, cx, cy, r)

    def draw(self):
        pass

class AbstractShape(Element):
    def __init__(self, command, node, ctx):
        self.node = node
        self.command = command
        self.ctx = ctx

    def get_data(self):
        return

    def get_style(self):
        style = simplestyle.parseStyle(self.attr("style"))
        #remove any trailing space in dict keys/values
        style = dict([(str.strip(k), str.strip(v)) for k,v in style.items()])
        return style

    def set_style(self, style):
        """Translates style properties names into method calls"""
        self.ctx.style = style
        for key in style:
            tmp_list = map(str.capitalize, key.split("-"))
            method = "set" + "".join(tmp_list)
            if hasattr(self.ctx, method) and style[key] != "none":
                getattr(self.ctx, method)(style[key])
        #saves style to compare in next iteration
        self.ctx.style_cache = style

    def has_transform(self):
        return bool(self.attr("transform"))

    def get_transform(self):
        data = self.node.get("transform")
        if not data:
            return
        matrix = parseTransform(data)
        m11, m21, dx = matrix[0]
        m12, m22, dy = matrix[1]
        return m11, m12, m21, m22, dx, dy

    def has_gradient(self):
        style = self.get_style()
        if "fill" in style:
            fill = style["fill"]
            return fill.startswith("url(#linear") or \
                   fill.startswith("url(#radial")
        return False

    def get_gradient_href(self):
        style = self.get_style()
        if "fill" in style:
            return style["fill"][5:-1]
        return

    def has_clip(self):
        return bool(self.attr("clip-path"))

    def start(self, gradient):
        self.gradient = gradient
        self.ctx.write("\n// #%s" % self.attr("id"))
        if self.has_transform() or self.has_clip():
            self.ctx.save()

    def draw(self):
        data = self.get_data()
        style = self.get_style()
        self.ctx.beginPath()
        if self.has_transform():
            trans_matrix = self.get_transform()
            self.ctx.transform(*trans_matrix) # unpacks argument list
        if self.has_gradient():
            self.gradient.draw()
        self.set_style(style)
        # unpacks "data" in parameters to given method
        getattr(self.ctx, self.command)(*data)
        self.ctx.closePath()

    def end(self):
        if self.has_transform() or self.has_clip():
            self.ctx.restore()


class G(AbstractShape):
    def draw(self):
        #get layer label, if exists
        gtype = self.attr("groupmode", "inkscape") or "group"
        if self.has_transform():
            trans_matrix = self.get_transform()
            self.ctx.transform(*trans_matrix)


class Rect(AbstractShape):
    def get_data(self):
        x = self.attr("x")
        y = self.attr("y")
        w = self.attr("width")
        h = self.attr("height")
        rx = self.attr("rx") or 0
        ry = self.attr("ry") or 0
        return x, y, w, h, rx, ry


class Circle(AbstractShape):
    def __init__(self, command, node, ctx):
        AbstractShape.__init__(self, command, node, ctx)
        self.command = "arc"

    def get_data(self):
        import math
        cx = self.attr("cx")
        cy = self.attr("cy")
        r = self.attr("r")
        return cx, cy, r, 0, math.pi * 2, True


class Ellipse(AbstractShape):
    def get_data(self):
        cx = self.attr("cx")
        cy = self.attr("cy")
        rx = self.attr("rx")
        ry = self.attr("ry")
        return cx, cy, rx, ry

    def draw(self):
        import math
        cx, cy, rx, ry = self.get_data()
        style = self.get_style()
        self.ctx.beginPath()
        if self.has_transform():
            trans_matrix = self.get_transform()
            self.ctx.transform(*trans_matrix) # unpacks argument list
        self.set_style(style)

        KAPPA = 4 * ((math.sqrt(2) - 1) / 3)
        self.ctx.moveTo(cx, cy - ry)
        self.ctx.bezierCurveTo(cx + (KAPPA * rx), cy - ry,  cx + rx, cy - (KAPPA * ry), cx + rx, cy)
        self.ctx.bezierCurveTo(cx + rx, cy + (KAPPA * ry), cx + (KAPPA * rx), cy + ry, cx, cy + ry)
        self.ctx.bezierCurveTo(cx - (KAPPA * rx), cy + ry, cx - rx, cy + (KAPPA * ry), cx - rx, cy)
        self.ctx.bezierCurveTo(cx - rx, cy - (KAPPA * ry), cx - (KAPPA * rx), cy - ry, cx, cy - ry)
        self.ctx.closePath()


class Path(AbstractShape):
    def get_data(self):
        #path data is already converted to float
        return parsePath(self.attr("d"))

    def pathMoveTo(self, data):
        self.ctx.moveTo(data[0], data[1])
        self.currentPosition = data[0], data[1]

    def pathLineTo(self, data):
        self.ctx.lineTo(data[0], data[1])
        self.currentPosition = data[0], data[1]

    def pathCurveTo(self, data):
        x1, y1, x2, y2 = data[0], data[1], data[2], data[3]
        x, y = data[4], data[5]
        self.ctx.bezierCurveTo(x1, y1, x2, y2, x, y)
        self.currentPosition = x, y

    def pathArcTo(self, data):
        #http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
        # code adapted from http://code.google.com/p/canvg/
        import math
        x1 = self.currentPosition[0]
        y1 = self.currentPosition[1]
        x2 = data[5]
        y2 = data[6]
        rx = data[0]
        ry = data[1]
        angle = data[2] * (math.pi / 180.0)
        arcflag = data[3]
        sweepflag = data[4]

        #compute (x1', y1')
        _x1 = math.cos(angle) * (x1 - x2) / 2.0 + math.sin(angle) * (y1 - y2) / 2.0
        _y1 = -math.sin(angle) * (x1 - x2) / 2.0 + math.cos(angle) * (y1 - y2) / 2.0

        #adjust radii
        l = _x1**2 / rx**2 + _y1**2 / ry**2
        if l > 1:
            rx *= math.sqrt(l)
            ry *= math.sqrt(l)

        #compute (cx', cy')
        numr = (rx**2 * ry**2) - (rx**2 * _y1**2) - (ry**2 * _x1**2)
        demr = (rx**2 * _y1**2) + (ry**2 * _x1**2)
        sig = -1 if arcflag == sweepflag else 1
        sig = sig * math.sqrt(numr / demr)
        if math.isnan(sig): sig = 0;
        _cx = sig * rx * _y1 / ry
        _cy = sig * -ry * _x1 / rx

        #compute (cx, cy) from (cx', cy')
        cx = (x1 + x2) / 2.0 + math.cos(angle) * _cx - math.sin(angle) * _cy
        cy = (y1 + y2) / 2.0 + math.sin(angle) * _cx + math.cos(angle) * _cy

        #compute startAngle & endAngle
        #vector magnitude
        m = lambda v: math.sqrt(v[0]**2 + v[1]**2)
        #ratio between two vectors
        r = lambda u, v: (u[0] * v[0] + u[1] * v[1]) / (m(u) * m(v))
        #angle between two vectors
        a = lambda u, v: (-1 if u[0]*v[1] < u[1]*v[0] else 1) * math.acos(r(u,v))
        #initial angle
        a1 = a([1,0], [(_x1 - _cx) / rx, (_y1 - _cy)/ry])
        #angle delta
        u = [(_x1 - _cx) / rx, (_y1 - _cy) / ry]
        v = [(-_x1 - _cx) / rx, (-_y1 - _cy) / ry]
        ad = a(u, v)
        if r(u,v) <= -1: ad = math.pi
        if r(u,v) >= 1: ad = 0

        if sweepflag == 0 and ad > 0: ad = ad - 2 * math.pi;
        if sweepflag == 1 and ad < 0: ad = ad + 2 * math.pi;

        r = rx if rx > ry else ry
        sx = 1 if rx > ry else rx / ry
        sy = ry / rx if rx > ry else 1

        self.ctx.translate(cx, cy)
        self.ctx.rotate(angle)
        self.ctx.scale(sx, sy)
        self.ctx.arc(0, 0, r, a1, a1 + ad, 1 - sweepflag)
        self.ctx.scale(1/sx, 1/sy)
        self.ctx.rotate(-angle)
        self.ctx.translate(-cx, -cy)
        self.currentPosition = x2, y2

    def draw(self):
        path = self.get_data()
        style = self.get_style()
        """Gets the node type and calls the given method"""
        self.ctx.beginPath()
        if self.has_transform():
            trans_matrix = self.get_transform()
            self.ctx.transform(*trans_matrix) # unpacks argument list
        self.set_style(style)

        """Draws path commands"""
        path_command = {"M": self.pathMoveTo,
                       "L": self.pathLineTo,
                       "C": self.pathCurveTo,
                       "A": self.pathArcTo}
        for pt in path:
            comm, data = pt
            if comm in path_command:
                path_command[comm](data)

        self.ctx.closePath()


class Line(Path):
    def get_data(self):
        x1 = self.attr("x1")
        y1 = self.attr("y1")
        x2 = self.attr("x2")
        y2 = self.attr("y2")
        return (("M", (x1, y1)), ("L", (x2, y2)))


class Polygon(Path):
    def get_data(self):
        points = self.attr("points").strip().split(" ")
        points = map(lambda x: x.split(","), points)
        comm = []
        for pt in points:           # creating path command similar
            pt = map(float, pt)
            comm.append(["L", pt])
        comm[0][0] = "M"            # first command must be a 'M' => moveTo
        return comm


class Polyline(Polygon):
    pass


class Text(AbstractShape):
    def text_helper(self, tspan):
        if not len(tspan):
            return unicode(tspan.text)
        for ts in tspan:
            return ts.text + self.text_helper(ts) + ts.tail

    def set_text_style(self, style):
        keys = ("font-style", "font-weight", "font-size", "font-family")
        text = []
        for key in keys:
            if key in style:
                text.append(style[key])
        self.ctx.setFont(" ".join(text))

    def get_data(self):
        x = self.attr("x")
        y = self.attr("y")
        return x, y

    def draw(self):
        x, y = self.get_data()
        style = self.get_style()
        if self.has_transform():
            trans_matrix = self.get_transform()
            self.ctx.transform(*trans_matrix) # unpacks argument list
        self.set_style(style)
        self.set_text_style(style)

        for tspan in self.node:
            text = self.text_helper(tspan)
            _x = float(tspan.get("x"))
            _y = float(tspan.get("y"))
            self.ctx.fillText(text, _x, _y)
