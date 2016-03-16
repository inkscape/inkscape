# coding=utf-8
'''
Copyright (C) 2013 Sebastian Wüst, sebi@timewaster.de

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

# standard libraries
import math
from StringIO import StringIO
# local library
import inkex


class hpglDecoder:

    def __init__(self, hpglString, options):
        ''' options:
                "resolutionX":float
                "resolutionY":float
                "showMovements":bool
                "docWidth":float
                "docHeight":float
        '''
        self.hpglString = hpglString
        self.options = options
        self.scaleX = options.resolutionX / 25.4 # dots/inch to dots/mm
        self.scaleY = options.resolutionY / 25.4 # dots/inch to dots/mm
        self.warning = ''
        self.textMovements = _("Movements")
        self.textPenNumber = _("Pen ")
        self.layers = {}
        self.oldCoordinates = (0.0, self.options.docHeight)

    def getSvg(self):
        actualLayer = 0
        # prepare document
        self.doc = inkex.etree.parse(StringIO('<svg xmlns:sodipodi="' + inkex.NSS['sodipodi'] + '" xmlns:inkscape="' + inkex.NSS['inkscape'] + '" width="%smm" height="%smm" viewBox="0 0 %s %s"></svg>' %
            (self.options.docWidth, self.options.docHeight, self.options.docWidth, self.options.docHeight)))
        inkex.etree.SubElement(self.doc.getroot(), inkex.addNS('namedview', 'sodipodi'), {inkex.addNS('document-units', 'inkscape'): 'mm'})
        if self.options.showMovements:
            self.layers[0] = inkex.etree.SubElement(self.doc.getroot(), 'g', {inkex.addNS('groupmode', 'inkscape'): 'layer', inkex.addNS('label', 'inkscape'): self.textMovements, 'id': self.textMovements})
        # cut stream into commands
        hpglData = self.hpglString.split(';')
        # if number of commands is under needed minimum, no data was found
        if len(hpglData) < 3:
            raise Exception('NO_HPGL_DATA')
        # decode commands into svg data
        for i, command in enumerate(hpglData):
            if command.strip() != '':
                if command[:2] == 'IN' or command[:2] == 'FS' or command[:2] == 'VS':
                    # if Initialize, force or speed command ignore it
                    pass
                elif command[:2] == 'SP':
                    # if Select Pen command
                    actualLayer = int(command[2:])
                elif command[:2] == 'PU':
                    # if Pen Up command
                    self.parametersToPath(command[2:], 0, True)
                elif command[:2] == 'PD':
                    # if Pen Down command
                    self.parametersToPath(command[2:], actualLayer + 1, False)
                else:
                    self.warning = 'UNKNOWN_COMMANDS'
        return (self.doc, self.warning)

    def parametersToPath(self, parameters, layerNum, isPU):
        # split params and sanity check them
        parameters = parameters.strip().split(',')
        if len(parameters) > 0 and len(parameters) % 2 == 0:
            for i, param in enumerate(parameters):
                # convert params to document units
                if i % 2 == 0:
                    parameters[i] = str(float(param) / self.scaleX)
                else:
                    parameters[i] = str(self.options.docHeight - (float(param) / self.scaleY))
            # create path and add it to the corresponding layer
            if not isPU or (self.options.showMovements and isPU):
                # create layer if it does not exist
                try:
                    self.layers[layerNum]
                except KeyError:
                    self.layers[layerNum] = inkex.etree.SubElement(self.doc.getroot(), 'g',
                        {inkex.addNS('groupmode', 'inkscape'): 'layer', inkex.addNS('label', 'inkscape'): self.textPenNumber + str(layerNum - 1), 'id': self.textPenNumber + str(layerNum - 1)})
                path = 'M %f,%f L %s' % (self.oldCoordinates[0], self.oldCoordinates[1], ','.join(parameters))
                inkex.etree.SubElement(self.layers[layerNum], 'path', {'d': path, 'style': 'stroke:#' + ('ff0000' if isPU else '000000') + '; stroke-width:0.2; fill:none;'})
            self.oldCoordinates = (float(parameters[-2]), float(parameters[-1]))


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
