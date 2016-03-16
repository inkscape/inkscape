#!/usr/bin/env python
'''
Copyright (C) 2010 Aurelio A. Heckert, aurium (a) gmail dot com

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


def is_empty(val):
    if val is None:
        return True
    else:
        return len(str(val)) == 0


class WebSlicer_Effect(inkex.Effect):

    def __init__(self):
        inkex.Effect.__init__(self)

    def get_slicer_layer(self, force_creation=False):
        # Test if webslicer-layer layer existis
        layer = self.document.xpath(
                     '//*[@id="webslicer-layer" and @inkscape:groupmode="layer"]',
                     namespaces=inkex.NSS)
        if len(layer) is 0:
            if force_creation:
                # Create a new layer
                layer = inkex.etree.SubElement(self.document.getroot(), 'g')
                layer.set('id', 'webslicer-layer')
                layer.set(inkex.addNS('label', 'inkscape'), 'Web Slicer')
                layer.set(inkex.addNS('groupmode', 'inkscape'), 'layer')
            else:
                layer = None
        else:
            layer = layer[0]
        return layer

    def get_conf_text_from_list(self, conf_atts):
        conf_list = []
        for att in conf_atts:
            if not is_empty(getattr(self.options, att)):
                conf_list.append(
                    att.replace('_','-') +': '+ str(getattr(self.options, att))
                )
        return "\n".join( conf_list )

