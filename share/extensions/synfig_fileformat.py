#!/usr/bin/env python
"""
synfig_fileformat.py
Synfig file format utilities

Copyright (C) 2011 Nikita Kitaev

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
"""

###### Constants ##########################################
kux = 60.0 # Number of SVG units (pixels) per Synfig "unit"
gamma = 2.2
tangent_scale = 3.0 # Synfig tangents are scaled by a factor of 3



###### Layer parameters, types, and default values ########
layers = {}

# Layer_Composite is the parent of most layers
default_composite = {
    "z_depth": ["real", 0.0],
    "amount": ["real", 1.0],
    "blend_method": ["integer", 0],
    }

layers["PasteCanvas"] = default_composite.copy()
layers["PasteCanvas"].update({
        "origin": ["vector", [0.0, 0.0]],
        "canvas": ["canvas", None],
        "zoom": ["real", 0.0],
        "time_offset": ["time", "0s"],
        "children_lock": ["bool", False],
        "focus": ["vector", [0.0, 0.0]]
        })


## Layers in mod_geometry

layers["circle"] = default_composite.copy()
layers["circle"].update({
        "color": ["color", [0,0,0,1]],
        "radius": ["real", 1.0],
        "feather": ["real", 0.0],
        "origin": ["vector", [0.0, 0.0]],
        "invert": ["bool", False],
        "falloff": ["integer", 2]
        })

layers["rectangle"] = default_composite.copy()
layers["rectangle"].update({
        "color": ["color", [0,0,0,1]],
        "point1": ["vector", [0,0]],
        "point2": ["vector", [1,1]],
        "expand": ["real", 0.0],
        "invert": ["bool", False]
        })

default_shape = default_composite.copy()
default_shape.update({
        "color": ["color", [0,0,0,1]],
        "origin": ["vector", [0.0, 0.0]],
        "invert": ["bool", False],
        "antialias": ["bool", True],
        "feather": ["real",  0.0],
        "blurtype": ["integer",  1],
        "winding_style": ["integer", 0]
        })

layers["region"] = default_shape.copy()
layers["region"].update({
        "bline": ["bline", None]
        })

layers["outline"] = default_shape.copy()
layers["outline"].update({
        "bline": ["bline", None],
        "round_tip[0]": ["bool", True],
        "round_tip[1]": ["bool", True],
        "sharp_cusps": ["bool", True],
        "width": ["real", 1.0],
        "loopyness": ["real", 1.0],
        "expand": ["real", 0.0],
        "homogeneous_width": ["bool", True]
        })


## Layers in mod_gradient

layers["linear_gradient"] = default_composite.copy()
layers["linear_gradient"].update({
        "p1": ["vector", [0,0]],
        "p2": ["vector", [1,1]],
        "gradient": ["gradient", {0.0:[0,0,0,1], 1.0:[1,1,1,1]} ],
        "loop": ["bool", False],
        "zigzag": ["bool", False]
        })

layers["radial_gradient"] = default_composite.copy()
layers["radial_gradient"].update({
        "gradient": ["gradient", {0.0:[0,0,0,1], 1.0:[1,1,1,1]} ],
        "center": ["vector", [0,0]],
        "radius": ["real", 1.0],
        "loop": ["bool", False],
        "zigzag": ["bool", False]
        })


## Layers in lyr_std

layers["import"] = default_composite.copy()
layers["import"].update({
        "tl": ["vector", [-1,1]],
        "br": ["vector", [1,-1]],
        "c": ["integer", 1],
        "gamma_adjust": ["real", 1.0],
        "filename": ["string", ""],   # <string>foo</string>
        "time_offset": ["time", "0s"]
        })

# transforms are not blending
layers["warp"] = {
    "src_tl": ["vector", [-1,1]],
    "src_br": ["vector", [1,-1]],
    "dest_tl": ["vector", [-1,1]],
    "dest_tr": ["vector", [1,1]],
    "dest_br": ["vector", [1,-1]],
    "dest_bl": ["vector", [-1,-1]],
    "clip": ["bool", False],
    "horizon": ["real", 4.0]
    }

layers["rotate"] = {
    "origin": ["vector", [0.0, 0.0]],
    "amount": ["angle", 0] # <angle value=.../>
    }

layers["translate"] = {
    "origin": ["vector", [0.0, 0.0]]
    }

## Layers in mod_filter
layers["blur"] = default_composite.copy()
layers["blur"].update({
        "size": ["vector", [1,1]],
        "type": ["integer", 3] # 1 is fast gaussian, 3 is regular
        })


###### Layer versions #####################################
layer_versions = {
    "outline" : "0.2",
    "rectangle" : "0.2",
    "linear_gradient" : "0.0",
    "blur" : "0.2",
    None : "0.1" # default
    }

###### Blend Methods ######################################
blend_method_names = {
    0  : "composite",
    1  : "straight",
    13 : "onto",
    21 : "straight onto",
    12 : "behind",
    16 : "screen",
    20 : "overlay",
    17 : "hand light",
    6  : "multiply",
    7  : "divide",
    4  : "add",
    5  : "subtract",
    18 : "difference",
    2  : "brighten",
    3  : "darken",
    8  : "color",
    9  : "hue",
    10 : "saturation",
    11 : "luminance",
    14 : "alpha brighten", #deprecated
    15 : "alpha darken",   #deprecated
    19 : "alpha over"      #deprecated
    }

blend_methods = dict((v, k) for (k, v) in blend_method_names.iteritems())

###### Functions ##########################################
def paramType(layer, param, value=None):
    if layer in layers.keys():
        layer_params = layers[layer]
        if param in layer_params.keys():
            return layer_params[param][0]
        else:
            raise Exception, "Invalid parameter type for layer"
    else:
        # Unknown layer, try to determine parameter type based on value
        if value is None:
            raise Exception, "No information for given layer"
        if type(value) == int:
            return "integer"
        elif type(value) == float:
            return "real"
        elif type(value) == bool:
            return "bool"
        elif type(value) == dict:
            if "points" in value.keys():
                return "bline"
            elif 0.0 in value.keys():
                return "gradient"
            else:
                raise Exception, "Could not automatically determine parameter type"
        elif type(value) == list:
            if len(value) == 2:
                return "vector"
            elif len(value) == 3 or len(value) == 4:
                return "color"
            else:
                # The first two could also be canvases
                return "canvas"
        elif type(value) == str:
            return "string"

def defaultLayerVersion(layer):
    if layer in layer_versions.keys():
        return layer_versions[layer]
    else:
        return layer_versions[None]

def defaultLayerParams(layer):
    if layer in layers.keys():
        return layers[layer].copy()
    else:
        return {}
