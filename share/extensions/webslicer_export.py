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
# standard library
import os
import sys
import tempfile
# local library
from webslicer_effect import *
import inkex


class WebSlicer_Export(WebSlicer_Effect):

    def __init__(self):
        WebSlicer_Effect.__init__(self)
        self.OptionParser.add_option("--tab")
        self.OptionParser.add_option("--dir",
                                     action="store", type="string",
                                     dest="dir",
                                     help="")
        self.OptionParser.add_option("--create-dir",
                                     action="store", type="inkbool",
                                     default=False,
                                     dest="create_dir",
                                     help="")
        self.OptionParser.add_option("--with-code",
                                     action="store", type="inkbool",
                                     default=False,
                                     dest="with_code",
                                     help="")


    svgNS = '{http://www.w3.org/2000/svg}'


    def validate_inputs(self):
        # The user must supply a directory to export:
        if is_empty( self.options.dir ):
            inkex.errormsg(_('You must give a directory to export the slices.'))
            return {'error':'You must give a directory to export the slices.'}
        # No directory separator at the path end:
        if self.options.dir[-1] == '/' or self.options.dir[-1] == '\\':
            self.options.dir = self.options.dir[0:-1]
        # Test if the directory exists:
        if not os.path.exists( self.options.dir ):
            if self.options.create_dir:
                # Try to create it:
                try:
                    os.makedirs( self.options.dir )
                except Exception, e:
                    inkex.errormsg( _('Can\'t create "%s".') % self.options.dir )
                    inkex.errormsg( _('Error: %s') % e )
                    return {'error':'Can\'t create the directory to export.'}
            else:
                inkex.errormsg(_('The directory "%s" does not exists.') % self.options.dir)
                return
        # Check whether slicer layer exists (bug #1198826)
        slicer_layer = self.get_slicer_layer()
        if slicer_layer is None:
            inkex.errormsg(_('No slicer layer found.'))
            return {'error':'No slicer layer found.'}
        else:
            self.unique_html_id( slicer_layer )
        return None


    def get_cmd_output(self, cmd):
        # This solution comes from Andrew Reedick <jr9445 at ATT.COM>
        # http://mail.python.org/pipermail/python-win32/2008-January/006606.html
        # This method replaces the commands.getstatusoutput() usage, with the
        # hope to correct the windows exporting bug:
        # https://bugs.launchpad.net/inkscape/+bug/563722
        if sys.platform != "win32": cmd = '{ '+ cmd +'; }'
        pipe = os.popen(cmd +' 2>&1', 'r')
        text = pipe.read()
        sts = pipe.close()
        if sts is None: sts = 0
        if text[-1:] == '\n': text = text[:-1]
        return sts, text


    _html_ids = []
    def unique_html_id(self, el):
        for child in el.getchildren():
            if child.tag in [ self.svgNS+'rect', self.svgNS+'path',
                              self.svgNS+'circle', self.svgNS+'g' ]:
                conf = self.get_el_conf(child)
                if conf['html-id'] in self._html_ids:
                    inkex.errormsg(
                        _('You have more than one element with "%s" html-id.') %
                        conf['html-id'] )
                    n = 2
                    while (conf['html-id']+"-"+str(n)) in self._html_ids: n += 1
                    conf['html-id'] += "-"+str(n)
                self._html_ids.append( conf['html-id'] )
                self.save_conf( conf, child )
                self.unique_html_id( child )


    def test_if_has_imagemagick(self):
        (status, output) = self.get_cmd_output('convert --version')
        self.has_magick = ( status == 0 and 'ImageMagick' in output )


    def effect(self):
        self.test_if_has_imagemagick()
        error = self.validate_inputs()
        if error: return error
        # Register the basic CSS code:
        self.reg_css( 'body', 'text-align', 'center' )
        # Create the temporary SVG with invisible Slicer layer to export image pieces
        self.create_the_temporary_svg()
        # Start what we really want!
        self.export_chids_of( self.get_slicer_layer() )
        # Write the HTML and CSS files if asked for:
        if self.options.with_code:
            self.make_html_file()
            self.make_css_file()
        # Delete the temporary SVG with invisible Slicer layer
        self.delete_the_temporary_svg()
        #TODO: prevent inkex to return svg code to update Inkscape


    def make_html_file(self):
        f = open(os.path.join(self.options.dir,'layout.html'), 'w')
        f.write(
            '<html>\n<head>\n'                                     +\
            '  <title>Web Layout Testing</title>\n'                +\
            '  <style type="text/css" media="screen">\n'           +\
            '    @import url("style.css")\n'                       +\
            '  </style>\n'                                         +\
            '</head>\n<body>\n'                                    +\
            self.html_code()                                       +\
            '  <p style="position:absolute; bottom:10px">\n'       +\
            '  This HTML code is not done to the web. <br/>\n'     +\
            '  The automatic HTML and CSS code are only a helper.' +\
            '</p>\n</body>\n</html>' )
        f.close()


    def make_css_file(self):
        f = open(os.path.join(self.options.dir,'style.css'), 'w')
        f.write(
            '/*\n'                                                    +\
            '** This CSS code is not done to the web.\n'              +\
            '** The automatic HTML and CSS code are only a helper.\n' +\
            '*/\n'                                                    +\
            self.css_code() )
        f.close()


    def create_the_temporary_svg(self):
        (ref, self.tmp_svg) = tempfile.mkstemp('.svg')
        layer = self.get_slicer_layer()
        current_style = ('style' in layer.attrib) and layer.attrib['style'] or ''
        layer.attrib['style'] = 'display:none'
        self.document.write( self.tmp_svg );
        layer.attrib['style'] = current_style


    def delete_the_temporary_svg(self):
        os.remove( self.tmp_svg )


    noid_element_count = 0
    def get_el_conf(self, el):
        desc = el.find(self.svgNS+'desc')
        conf = {}
        if desc is None:
            desc = inkex.etree.SubElement(el, 'desc')
        if desc.text is None:
            desc.text = ''
        for line in desc.text.split("\n"):
            if line.find(':') > 0:
                line = line.split(':')
                conf[line[0].strip()] = line[1].strip()
        if not 'html-id' in conf:
            if el == self.get_slicer_layer():
                return {'html-id':'#body#'}
            else:
                self.noid_element_count += 1
                conf['html-id'] = 'element-'+str(self.noid_element_count)
                desc.text += "\nhtml-id:"+conf['html-id']
        return conf


    def save_conf(self, conf, el):
        desc = el.find('{http://www.w3.org/2000/svg}desc')
        if desc is not None:
            conf_a = []
            for k in conf:
                conf_a.append( k+' : '+conf[k] )
            desc.text = "\n".join(conf_a)


    def export_chids_of(self, parent):
        parent_id = self.get_el_conf( parent )['html-id']
        for el in parent.getchildren():
            el_conf = self.get_el_conf( el )
            if el.tag == self.svgNS+'g':
                if self.options.with_code:
                    self.register_group_code( el, el_conf )
                else:
                    self.export_chids_of( el )
            if el.tag in [ self.svgNS+'rect', self.svgNS+'path', self.svgNS+'circle' ]:
                if self.options.with_code:
                    self.register_unity_code( el, el_conf, parent_id )
                self.export_img( el, el_conf )


    def register_group_code(self, group, conf):
        self.reg_html('div', group)
        selec = '#'+conf['html-id']
        self.reg_css( selec, 'position', 'absolute' )
        geometry = self.get_relative_el_geometry(group)
        self.reg_css( selec, 'top', str(int(geometry['y']))+'px' )
        self.reg_css( selec, 'left', str(int(geometry['x']))+'px' )
        self.reg_css( selec, 'width', str(int(geometry['w']))+'px' )
        self.reg_css( selec, 'height', str(int(geometry['h']))+'px' )
        self.export_chids_of( group )


    def __validate_slice_conf(self, conf):
        if not 'layout-disposition' in conf:
            conf['layout-disposition'] = 'bg-el-norepeat'
        if not 'layout-position-anchor' in conf:
            conf['layout-position-anchor'] = 'mc'
        return conf


    def register_unity_code(self, el, conf, parent_id):
        conf = self.__validate_slice_conf(conf)
        css_selector = '#'+conf['html-id']
        bg_repeat = 'no-repeat'
        img_name = self.img_name(el, conf)
        if conf['layout-disposition'][0:2] == 'bg':
            if conf['layout-disposition'][0:9] == 'bg-parent':
                if parent_id == '#body#':
                    css_selector = 'body'
                else:
                    css_selector = '#'+parent_id
                if conf['layout-disposition'] == 'bg-parent-repeat':
                    bg_repeat = 'repeat'
                if conf['layout-disposition'] == 'bg-parent-repeat-x':
                    bg_repeat = 'repeat-x'
                if conf['layout-disposition'] == 'bg-parent-repeat-y':
                    bg_repeat = 'repeat-y'
                lay_anchor = conf['layout-position-anchor']
                if lay_anchor == 'tl': lay_anchor = 'top left'
                if lay_anchor == 'tc': lay_anchor = 'top center'
                if lay_anchor == 'tr': lay_anchor = 'top right'
                if lay_anchor == 'ml': lay_anchor = 'middle left'
                if lay_anchor == 'mc': lay_anchor = 'middle center'
                if lay_anchor == 'mr': lay_anchor = 'middle right'
                if lay_anchor == 'bl': lay_anchor = 'bottom left'
                if lay_anchor == 'bc': lay_anchor = 'bottom center'
                if lay_anchor == 'br': lay_anchor = 'bottom right'
                self.reg_css( css_selector, 'background',
                              'url("%s") %s %s' % (img_name, bg_repeat, lay_anchor) )
            else: # conf['layout-disposition'][0:9] == 'bg-el...'
                self.reg_html('div', el)
                self.reg_css( css_selector, 'background',
                              'url("%s") %s' % (img_name, 'no-repeat') )
                self.reg_css( css_selector, 'position', 'absolute' )
                geo = self.get_relative_el_geometry(el,True)
                self.reg_css( css_selector, 'top', geo['y'] )
                self.reg_css( css_selector, 'left', geo['x'] )
                self.reg_css( css_selector, 'width', geo['w'] )
                self.reg_css( css_selector, 'height', geo['h'] )
        else: # conf['layout-disposition'] == 'img...'
                self.reg_html('img', el)
                if conf['layout-disposition'] == 'img-pos':
                    self.reg_css( css_selector, 'position', 'absolute' )
                    geo = self.get_relative_el_geometry(el)
                    self.reg_css( css_selector, 'left', str(geo['x'])+'px' )
                    self.reg_css( css_selector, 'top', str(geo['y'])+'px' )
                if conf['layout-disposition'] == 'img-float-left':
                    self.reg_css( css_selector, 'float', 'right' )
                if conf['layout-disposition'] == 'img-float-right':
                    self.reg_css( css_selector, 'float', 'right' )


    el_geo = { }
    def register_all_els_geometry(self):
        ink_cmm = 'inkscape --query-all '+self.tmp_svg
        (status, output) = self.get_cmd_output( ink_cmm )
        self.el_geo = { }
        if status == 0:
            for el in output.split('\n'):
                el = el.split(',')
                if len(el) == 5:
                    self.el_geo[el[0]] = { 'x':float(el[1]), 'y':float(el[2]),
                                           'w':float(el[3]), 'h':float(el[4]) }
        doc_w = self.unittouu( self.document.getroot().get('width') )
        doc_h = self.unittouu( self.document.getroot().get('height') )
        self.el_geo['webslicer-layer'] = { 'x':0, 'y':0, 'w':doc_w, 'h':doc_h }


    def get_relative_el_geometry(self, el, value_to_css=False):
        # This method return a dictionary with x, y, w and h keys.
        # All values are float, if value_to_css is False, otherwise
        # that is a string ended with "px". The x and y values are
        # relative to parent position.
        if not self.el_geo: self.register_all_els_geometry()
        parent = self.getParentNode(el)
        geometry = self.el_geo[el.attrib['id']]
        geometry['x'] -= self.el_geo[parent.attrib['id']]['x']
        geometry['y'] -= self.el_geo[parent.attrib['id']]['y']
        if value_to_css:
            for k in geometry: geometry[k] = str(int(geometry[k]))+'px'
        return geometry


    def img_name(self, el, conf):
        return el.attrib['id']+'.'+conf['format']


    def export_img(self, el, conf):
        if not self.has_magick:
            inkex.errormsg(_('You must install the ImageMagick to get JPG and GIF.'))
            conf['format'] = 'png'
        img_name = os.path.join( self.options.dir, self.img_name(el, conf) )
        img_name_png = img_name
        if conf['format'] != 'png':
            img_name_png = img_name+'.png'
        opts = ''
        if 'bg-color' in conf: opts += ' -b "'+conf['bg-color']+'" -y 1'
        if 'dpi' in conf: opts += ' -d '+conf['dpi']
        if 'dimension' in conf:
            dim = conf['dimension'].split('x')
            opts += ' -w '+dim[0]+' -h '+dim[1]
        (status, output) = self.get_cmd_output(
            'inkscape %s -i "%s" -e "%s" "%s"' % (
                opts, el.attrib['id'], img_name_png, self.tmp_svg
            )
        )
        if conf['format'] != 'png':
            opts = ''
            if conf['format'] == 'jpg':
                opts += ' -quality '+str(conf['quality'])
            if conf['format'] == 'gif':
                if conf['gif-type'] == 'grayscale':
                    opts += ' -type Grayscale'
                else:
                    opts += ' -type Palette'
                if conf['palette-size'] < 256:
                    opts += ' -colors '+str(conf['palette-size'])
            (status, output) = self.get_cmd_output(
                'convert "%s" %s "%s"' % ( img_name_png, opts, img_name )
            )
            if status != 0:
                inkex.errormsg('Upss... ImageMagick error: '+output)
            os.remove(img_name_png)


    _html = {}
    def reg_html(self, el_tag, el):
        parent = self.getParentNode( el )
        parent_id = self.get_el_conf( parent )['html-id']
        if parent == self.get_slicer_layer(): parent_id = 'body'
        conf = self.get_el_conf( el )
        el_id = conf['html-id']
        if 'html-class' in conf:
          el_class = conf['html-class']
        else:
          el_class = ''
        if not parent_id in self._html: self._html[parent_id] = []
        self._html[parent_id].append({ 'tag':el_tag, 'id':el_id, 'class':el_class })


    def html_code(self, parent='body', ident='  '):
        #inkex.errormsg( self._html )
        if not parent in self._html: return ''
        code = ''
        for el in self._html[parent]:
            child_code = self.html_code(el['id'], ident+'  ')
            tag_class = ''
            if el['class'] != '': tag_class = ' class="'+el['class']+'"'
            if el['tag'] == 'img':
                code += ident+'<img id="'+el['id']+'"'+tag_class+\
                        ' src="'+self.img_name(el, self.get_el_conf(el))+'"/>\n'
            else:
                code += ident+'<'+el['tag']+' id="'+el['id']+'"'+tag_class+'>\n'
                if child_code:
                    code += child_code
                else:
                    code += ident+'  Element '+el['id']+'\n'
                code += ident+'</'+el['tag']+'><!-- id="'+el['id']+'" -->\n'
        return code


    _css = []
    def reg_css(self, selector, att, val):
        pos = i = -1
        for s in self._css:
            i += 1
            if s['selector'] == selector: pos = i
        if pos == -1:
            pos = i + 1
            self._css.append({ 'selector':selector, 'atts':{} })
        if not att in self._css[pos]['atts']: self._css[pos]['atts'][att] = []
        self._css[pos]['atts'][att].append( val )


    def css_code(self):
        code = ''
        for s in self._css:
            code += '\n'+s['selector']+' {\n'
            for att in s['atts']:
                val = s['atts'][att]
                if att == 'background' and len(val) > 1:
                    code += '  /* the next attribute needs a CSS3 enabled browser */\n'
                code += '  '+ att +': '+ (', '.join(val)) +';\n'
            code += '}\n'
        return code


    def output(self):
        # Cancel document serialization to stdout
        pass


if __name__ == '__main__':
    e = WebSlicer_Export()
    e.affect()
