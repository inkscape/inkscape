/*
 * Factory for SPObject tree
 *
 * Authors:
 *   Markus Engel
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-factory.h"

// primary
#include "box3d.h"
#include "box3d-side.h"
#include "color-profile.h"
#include "persp3d.h"
#include "sp-anchor.h"
#include "sp-clippath.h"
#include "sp-defs.h"
#include "sp-desc.h"
#include "sp-ellipse.h"
#include "sp-filter.h"
#include "sp-flowdiv.h"
#include "sp-flowregion.h"
#include "sp-flowtext.h"
#include "sp-font.h"
#include "sp-font-face.h"
#include "sp-glyph.h"
#include "sp-glyph-kerning.h"
#include "sp-guide.h"
#include "sp-hatch.h"
#include "sp-hatch-path.h"
#include "sp-image.h"
#include "sp-item-group.h"
#include "sp-line.h"
#include "sp-linear-gradient.h"
#include "sp-marker.h"
#include "sp-mask.h"
#include "sp-mesh.h"
#include "sp-mesh-patch.h"
#include "sp-mesh-row.h"
#include "sp-metadata.h"
#include "sp-missing-glyph.h"
#include "sp-namedview.h"
#include "sp-object.h"
#include "sp-offset.h"
#include "sp-path.h"
#include "sp-pattern.h"
#include "sp-polygon.h"
#include "sp-polyline.h"
#include "sp-radial-gradient.h"
#include "sp-rect.h"
#include "sp-root.h"
#include "sp-script.h"
#include "sp-solid-color.h"
#include "sp-spiral.h"
#include "sp-star.h"
#include "sp-stop.h"
#include "sp-string.h"
#include "sp-style-elem.h"
#include "sp-switch.h"
#include "sp-symbol.h"
#include "sp-tag.h"
#include "sp-tag-use.h"
#include "sp-text.h"
#include "sp-textpath.h"
#include "sp-title.h"
#include "sp-tref.h"
#include "sp-tspan.h"
#include "sp-use.h"
#include "live_effects/lpeobject.h"

// filters
#include "filters/blend.h"
#include "filters/colormatrix.h"
#include "filters/componenttransfer.h"
#include "filters/componenttransfer-funcnode.h"
#include "filters/composite.h"
#include "filters/convolvematrix.h"
#include "filters/diffuselighting.h"
#include "filters/displacementmap.h"
#include "filters/distantlight.h"
#include "filters/flood.h"
#include "filters/gaussian-blur.h"
#include "filters/image.h"
#include "filters/merge.h"
#include "filters/mergenode.h"
#include "filters/morphology.h"
#include "filters/offset.h"
#include "filters/pointlight.h"
#include "filters/specularlighting.h"
#include "filters/spotlight.h"
#include "filters/tile.h"
#include "filters/turbulence.h"

SPObject *SPFactory::createObject(std::string const& id)
{
    SPObject *ret = NULL;

    if (id == "inkscape:box3d")
        ret = new SPBox3D;
    else if (id == "inkscape:box3dside")
        ret = new Box3DSide;
    else if (id == "svg:color-profile")
        ret = new Inkscape::ColorProfile;
    else if (id == "inkscape:persp3d")
        ret = new Persp3D;
    else if (id == "svg:a")
        ret = new SPAnchor;
    else if (id == "svg:clipPath")
        ret = new SPClipPath;
    else if (id == "svg:defs")
        ret = new SPDefs;
    else if (id == "svg:desc")
        ret = new SPDesc;
    else if (id == "svg:ellipse") {
        SPGenericEllipse *e = new SPGenericEllipse;
        e->type = SP_GENERIC_ELLIPSE_ELLIPSE;
        ret = e;
    } else if (id == "svg:circle") {
        SPGenericEllipse *c = new SPGenericEllipse;
        c->type = SP_GENERIC_ELLIPSE_CIRCLE;
        ret = c;
    } else if (id == "arc") {
        SPGenericEllipse *a = new SPGenericEllipse;
        a->type = SP_GENERIC_ELLIPSE_ARC;
        ret = a;
    }
    else if (id == "svg:filter")
        ret = new SPFilter;
    else if (id == "svg:flowDiv")
        ret = new SPFlowdiv;
    else if (id == "svg:flowSpan")
        ret = new SPFlowtspan;
    else if (id == "svg:flowPara")
        ret = new SPFlowpara;
    else if (id == "svg:flowLine")
        ret = new SPFlowline;
    else if (id == "svg:flowRegionBreak")
        ret = new SPFlowregionbreak;
    else if (id == "svg:flowRegion")
        ret = new SPFlowregion;
    else if (id == "svg:flowRegionExclude")
        ret = new SPFlowregionExclude;
    else if (id == "svg:flowRoot")
        ret = new SPFlowtext;
    else if (id == "svg:font")
        ret = new SPFont;
    else if (id == "svg:font-face")
        ret = new SPFontFace;
    else if (id == "svg:glyph")
        ret = new SPGlyph;
    else if (id == "svg:hkern")
        ret = new SPHkern;
    else if (id == "svg:vkern")
        ret = new SPVkern;
    else if (id == "sodipodi:guide")
        ret = new SPGuide;
    else if (id == "svg:hatch")
        ret = new SPHatch;
    else if (id == "svg:hatchPath")
        ret = new SPHatchPath;
    else if (id == "svg:image")
        ret = new SPImage;
    else if (id == "svg:g")
        ret = new SPGroup;
    else if (id == "svg:line")
        ret = new SPLine;
    else if (id == "svg:linearGradient")
        ret = new SPLinearGradient;
    else if (id == "svg:marker")
        ret = new SPMarker;
    else if (id == "svg:mask")
        ret = new SPMask;
    else if (id == "svg:mesh")
        ret = new SPMesh;
    else if (id == "svg:meshpatch")
        ret = new SPMeshpatch;
    else if (id == "svg:meshrow")
        ret = new SPMeshrow;
    else if (id == "svg:metadata")
        ret = new SPMetadata;
    else if (id == "svg:missing-glyph")
        ret = new SPMissingGlyph;
    else if (id == "sodipodi:namedview")
        ret = new SPNamedView;
    else if (id == "inkscape:offset")
        ret = new SPOffset;
    else if (id == "svg:path")
        ret = new SPPath;
    else if (id == "svg:pattern")
        ret = new SPPattern;
    else if (id == "svg:polygon")
        ret = new SPPolygon;
    else if (id == "svg:polyline")
        ret = new SPPolyLine;
    else if (id == "svg:radialGradient")
        ret = new SPRadialGradient;
    else if (id == "svg:rect")
        ret = new SPRect;
    else if (id == "svg:svg")
        ret = new SPRoot;
    else if (id == "svg:script")
        ret = new SPScript;
    else if (id == "svg:solidColor")
        ret = new SPSolidColor;
    else if (id == "spiral")
        ret = new SPSpiral;
    else if (id == "star")
        ret = new SPStar;
    else if (id == "svg:stop")
        ret = new SPStop;
    else if (id == "string")
        ret = new SPString;
    else if (id == "svg:style")
        ret = new SPStyleElem;
    else if (id == "svg:switch")
        ret = new SPSwitch;
    else if (id == "svg:symbol")
        ret = new SPSymbol;
    else if (id == "inkscape:tag")
        ret = new SPTag;
    else if (id == "inkscape:tagref")
        ret = new SPTagUse;
    else if (id == "svg:text")
        ret = new SPText;
    else if (id == "svg:title")
        ret = new SPTitle;
    else if (id == "svg:tref")
        ret = new SPTRef;
    else if (id == "svg:tspan")
        ret = new SPTSpan;
    else if (id == "svg:textPath")
        ret = new SPTextPath;
    else if (id == "svg:use")
        ret = new SPUse;
    else if (id == "inkscape:path-effect")
        ret = new LivePathEffectObject;


    // filters
    else if (id == "svg:feBlend")
        ret = new SPFeBlend;
    else if (id == "svg:feColorMatrix")
        ret = new SPFeColorMatrix;
    else if (id == "svg:feComponentTransfer")
        ret = new SPFeComponentTransfer;
    else if (id == "svg:feFuncR")
        ret = new SPFeFuncNode(SPFeFuncNode::R);
    else if (id == "svg:feFuncG")
        ret = new SPFeFuncNode(SPFeFuncNode::G);
    else if (id == "svg:feFuncB")
        ret = new SPFeFuncNode(SPFeFuncNode::B);
    else if (id == "svg:feFuncA")
        ret = new SPFeFuncNode(SPFeFuncNode::A);
    else if (id == "svg:feComposite")
        ret = new SPFeComposite;
    else if (id == "svg:feConvolveMatrix")
        ret = new SPFeConvolveMatrix;
    else if (id == "svg:feDiffuseLighting")
        ret = new SPFeDiffuseLighting;
    else if (id == "svg:feDisplacementMap")
        ret = new SPFeDisplacementMap;
    else if (id == "svg:feDistantLight")
        ret = new SPFeDistantLight;
    else if (id == "svg:feFlood")
        ret = new SPFeFlood;
    else if (id == "svg:feGaussianBlur")
        ret = new SPGaussianBlur;
    else if (id == "svg:feImage")
        ret = new SPFeImage;
    else if (id == "svg:feMerge")
        ret = new SPFeMerge;
    else if (id == "svg:feMergeNode")
        ret = new SPFeMergeNode;
    else if (id == "svg:feMorphology")
        ret = new SPFeMorphology;
    else if (id == "svg:feOffset")
        ret = new SPFeOffset;
    else if (id == "svg:fePointLight")
        ret = new SPFePointLight;
    else if (id == "svg:feSpecularLighting")
        ret = new SPFeSpecularLighting;
    else if (id == "svg:feSpotLight")
        ret = new SPFeSpotLight;
    else if (id == "svg:feTile")
        ret = new SPFeTile;
    else if (id == "svg:feTurbulence")
        ret = new SPFeTurbulence;
    else if (id == "inkscape:grid")
        ret = new SPObject; // TODO wtf
    else if (id == "rdf:RDF") // no SP node yet
        {}
    else if (id == "inkscape:clipboard") // SP node not necessary
        {}
    else if (id.empty()) // comments
        {}
    else {
        fprintf(stderr, "WARNING: unknown type: %s", id.c_str());
    }

    return ret;
}

std::string NodeTraits::get_type_string(Inkscape::XML::Node const &node)
{
    std::string name;

    switch (node.type()) {
    case Inkscape::XML::TEXT_NODE:
        name = "string";
        break;

    case Inkscape::XML::ELEMENT_NODE: {
        char const *const sptype = node.attribute("sodipodi:type");

        if (sptype) {
            name = sptype;
        } else {
            name = node.name();
        }
        break;
    }
    default:
        name = "";
        break;
    }

    return name;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
