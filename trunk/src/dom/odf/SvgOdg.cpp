/**
 *
 * This is a small experimental class for converting between
 * SVG and OpenDocument .odg files.   This code is not intended
 * to be a permanent solution for SVG-to-ODG conversion.  Rather,
 * it is a quick-and-easy test bed for ideas which will be later
 * recoded into C++.
 *
 * ---------------------------------------------------------------------
 *
 * SvgOdg - A program to experiment with conversions between SVG and ODG
 * Copyright (C) 2006 Bob Jamison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * For more information, please write to rwjj@earthlink.net
 *
 */


/**
 *
 */
public class SvgOdg
{



/**
 * Namespace declarations
 */
public static final String SVG_NS =
    "http://www.w3.org/2000/svg";
public static final String XLINK_NS =
    "http://www.w3.org/1999/xlink";
public static final String ODF_NS =
    "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
public static final String ODG_NS =
    "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
public static final String ODSVG_NS =
    "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";


DecimalFormat nrfmt;
//static final double pxToCm = 0.0339;
static final double pxToCm  = 0.0275;
static final double piToRad = 0.0174532925;
BufferedWriter out;
BufferedReader in;
int imageNr;
int styleNr;

//########################################################################
//#  M E S S A G E S
//########################################################################

/**
 *
 */
void err(String msg)
{
    System.out.println("SvgOdg ERROR:" + msg);
}

/**
 *
 */
void trace(String msg)
{
    System.out.println("SvgOdg:" + msg);
}




//########################################################################
//#  I N P U T    /    O U T P U T
//########################################################################

boolean po(String s)
{
    try
        {
        out.write(s);
        }
    catch(IOException e)
        {
        return false;
        }
    return true;
}

//########################################################################
//#  U T I L I T Y
//########################################################################

public void dumpDocument(Document doc)
{
    String s = "";
    try
        {
        TransformerFactory factory = TransformerFactory.newInstance();
        Transformer trans = factory.newTransformer();
        DOMSource source = new DOMSource(doc);
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        StreamResult result = new StreamResult(bos);
        trans.transform(source, result);
        byte buf[] = bos.toByteArray();
        s = new String(buf);
        }
    catch (javax.xml.transform.TransformerException e)
        {
        }
    trace("doc:" + s);
}


//########################################################################
//#  I N N E R    C L A S S    ImageInfo
//########################################################################
public class ImageInfo
{
String name;
String newName;
byte   buf[];

public String getName()
{
    return name;
}

public String getNewName()
{
    return newName;
}


public byte[] getBuf()
{
    return buf;
}

public ImageInfo(String name, String newName, byte buf[])
{
    this.name = name;
    this.name = newName;
    this.buf  = buf;
}

}
//########################################################################
//#  I N N E R    C L A S S    StyleInfo
//########################################################################
public class StyleInfo
{

String name;
public String getName()
{
    return name;
}

String cssStyle;
public String getCssStyle()
{
    return cssStyle;
}

String stroke;
public String getStroke()
{
    return stroke;
}

String strokeColor;
public String getStrokeColor()
{
    return strokeColor;
}

String strokeWidth;
public String getStrokeWidth()
{
    return strokeWidth;
}

String fill;
public String getFill()
{
    return fill;
}

String fillColor;
public String getFillColor()
{
    return fillColor;
}

public StyleInfo(String name, String cssStyle)
{
    this.name     = name;
    this.cssStyle = cssStyle;
    fill  = "none";
    stroke = "none";
}

}
//########################################################################
//#  E N D    I N N E R    C L A S S E S
//########################################################################




//########################################################################
//#  V A R I A B L E S
//########################################################################

/**
 * ODF content.xml file
 */
Document content;
public Document getContent()
{
    return content;
}

/**
 * ODF meta.xml file
 */
Document meta;
public Document getMeta()
{
    return meta;
}

/**
 * SVG file
 */
Document       svg;
public Document getSvg()
{
    return svg;
}

/**
 * Loaded ODF or SVG images
 */
ArrayList<ImageInfo> images;
public ArrayList<ImageInfo> getImages()
{
    return images;
}

/**
 * CSS styles
 */
HashMap<String, StyleInfo> styles;
public HashMap<String, StyleInfo> getStyles()
{
    return styles;
}





//########################################################################
//#  S V G    T O    O D F
//########################################################################

class PathData
{
String cmd;
double nr[];
PathData(String s, double buf[])
{
    cmd=s; nr = buf;
}
}

double getPathNum(StringTokenizer st)
{
    if (!st.hasMoreTokens())
        return 0.0;
    String s = st.nextToken();
    double nr = Double.parseDouble(s);
    return nr;
}

String parsePathData(String pathData, double bounds[])
{
    double minx = Double.MAX_VALUE;
    double maxx = Double.MIN_VALUE;
    double miny = Double.MAX_VALUE;
    double maxy = Double.MIN_VALUE;
    //trace("#### pathData:" + pathData);
    ArrayList<PathData> data = new ArrayList<PathData>();
    StringTokenizer st = new StringTokenizer(pathData, " ,");
    while (true)
        {
        String s = st.nextToken();
        if ( s.equals("z") || s.equals("Z") )
            {
            PathData pd = new PathData(s, new double[0]);
            data.add(pd);
            break;
            }
        else if ( s.equals("h") || s.equals("H") )
            {
            double d[] = new double[1];
            d[0] = getPathNum(st) * pxToCm;
            if (d[0] < minx)  minx = d[0];
            else if (d[0] > maxx) maxx = d[0];
            PathData pd = new PathData(s, d);
            data.add(pd);
            }
        else if ( s.equals("v") || s.equals("V") )
            {
            double d[] = new double[1];
            d[0] = getPathNum(st) * pxToCm;
            if (d[0] < miny) miny = d[0];
            else if (d[0] > maxy) maxy = d[0];
            PathData pd = new PathData(s, d);
            data.add(pd);
            }
        else if ( s.equals("m") || s.equals("M") ||
             s.equals("l") || s.equals("L") ||
             s.equals("t") || s.equals("T")  )
            {
            double d[] = new double[2];
            d[0] = getPathNum(st) * pxToCm;
            d[1] = getPathNum(st) * pxToCm;
            if (d[0] < minx)  minx = d[0];
            else if (d[0] > maxx) maxx = d[0];
            if (d[1] < miny) miny = d[1];
            else if (d[1] > maxy) maxy = d[1];
            PathData pd = new PathData(s, d);
            data.add(pd);
            }
        else if ( s.equals("q") || s.equals("Q") ||
             s.equals("s") || s.equals("S")  )
            {
            double d[] = new double[4];
            d[0] = getPathNum(st) * pxToCm;
            d[1] = getPathNum(st) * pxToCm;
            if (d[0] < minx)  minx = d[0];
            else if (d[0] > maxx) maxx = d[0];
            if (d[1] < miny) miny = d[1];
            else if (d[1] > maxy) maxy = d[1];
            d[2] = getPathNum(st) * pxToCm;
            d[3] = getPathNum(st) * pxToCm;
            if (d[2] < minx)  minx = d[2];
            else if (d[2] > maxx) maxx = d[2];
            if (d[3] < miny) miny = d[3];
            else if (d[3] > maxy) maxy = d[3];
            PathData pd = new PathData(s, d);
            data.add(pd);
            }
        else if ( s.equals("c") || s.equals("C") )
            {
            double d[] = new double[6];
            d[0] = getPathNum(st) * pxToCm;
            d[1] = getPathNum(st) * pxToCm;
            if (d[0] < minx)  minx = d[0];
            else if (d[0] > maxx) maxx = d[0];
            if (d[1] < miny) miny = d[1];
            else if (d[1] > maxy) maxy = d[1];
            d[2] = getPathNum(st) * pxToCm;
            d[3] = getPathNum(st) * pxToCm;
            if (d[2] < minx)  minx = d[2];
            else if (d[2] > maxx) maxx = d[2];
            if (d[3] < miny) miny = d[3];
            else if (d[3] > maxy) maxy = d[3];
            d[4] = getPathNum(st) * pxToCm;
            d[5] = getPathNum(st) * pxToCm;
            if (d[4] < minx)  minx = d[4];
            else if (d[4] > maxx) maxx = d[4];
            if (d[5] < miny) miny = d[5];
            else if (d[5] > maxy) maxy = d[5];
            PathData pd = new PathData(s, d);
            data.add(pd);
            }
        else if ( s.equals("a") || s.equals("A") )
            {
            double d[] = new double[6];
            d[0] = getPathNum(st) * pxToCm;
            d[1] = getPathNum(st) * pxToCm;
            if (d[0] < minx)  minx = d[0];
            else if (d[0] > maxx) maxx = d[0];
            if (d[1] < miny) miny = d[1];
            else if (d[1] > maxy) maxy = d[1];
            d[2] = getPathNum(st) * piToRad;//angle
            d[3] = getPathNum(st) * piToRad;//angle
            d[4] = getPathNum(st) * pxToCm;
            d[5] = getPathNum(st) * pxToCm;
            if (d[4] < minx)  minx = d[4];
            else if (d[4] > maxx) maxx = d[4];
            if (d[5] < miny) miny = d[5];
            else if (d[5] > maxy) maxy = d[5];
            PathData pd = new PathData(s, d);
            data.add(pd);
            }
        //trace("x:" + x + " y:" + y);
        }

    trace("minx:"  + minx + " maxx:" + maxx +
          " miny:" + miny + " maxy:" + maxy);

    StringBuffer buf = new StringBuffer();
    for (PathData pd : data)
        {
        buf.append(pd.cmd);
        buf.append(" ");
        for (double d:pd.nr)
            {
            buf.append(nrfmt.format(d * 1000.0));
            buf.append(" ");
            }
        }

    bounds[0] = minx;
    bounds[1] = miny;
    bounds[2] = maxx;
    bounds[3] = maxy;

    return buf.toString();
}



boolean parseTransform(String transStr, AffineTransform trans)
{
    trace("== transform:"+ transStr);
    StringTokenizer st = new StringTokenizer(transStr, ")");
    while (st.hasMoreTokens())
        {
        String chunk = st.nextToken();
        StringTokenizer st2 = new StringTokenizer(chunk, " ,(");
        if (!st2.hasMoreTokens())
            continue;
        String name = st2.nextToken();
        trace("   ++name:"+ name);
        if (name.equals("matrix"))
            {
            double v[] = new double[6];
            for (int i=0 ; i<6 ; i++)
                {
                if (!st2.hasMoreTokens())
                    break;
                v[i] = Double.parseDouble(st2.nextToken()) * pxToCm;
                }
            AffineTransform mat = new AffineTransform(v);
            trans.concatenate(mat);
            }
        else if (name.equals("translate"))
            {
            double dx = 0.0;
            double dy = 0.0;
            if (!st2.hasMoreTokens())
                continue;
            dx = Double.parseDouble(st2.nextToken()) * pxToCm;
            if (st2.hasMoreTokens())
                dy = Double.parseDouble(st2.nextToken()) * pxToCm;
            trans.translate(dx, dy);
            }
        else if (name.equals("scale"))
            {
            double sx = 1.0;
            double sy = 1.0;
            if (!st2.hasMoreTokens())
                continue;
            sx = sy = Double.parseDouble(st2.nextToken());
            if (st2.hasMoreTokens())
                sy = Double.parseDouble(st2.nextToken());
            trans.scale(sx, sy);
            }
        else if (name.equals("rotate"))
            {
            double r  = 0.0;
            double cx = 0.0;
            double cy = 0.0;
            if (!st2.hasMoreTokens())
                continue;
            r = Double.parseDouble(st2.nextToken()) * piToRad;
            if (st2.hasMoreTokens())
                {
                cx = Double.parseDouble(st2.nextToken()) * pxToCm;
                if (!st2.hasMoreTokens())
                    continue;
                cy = Double.parseDouble(st2.nextToken()) * pxToCm;
                trans.rotate(r, cx, cy);
                }
            else
                {
                trans.rotate(r);
                }
            }
        else if (name.equals("skewX"))
            {
            double angle  = 0.0;
            if (!st2.hasMoreTokens())
                continue;
            angle = Double.parseDouble(st2.nextToken());
            trans.shear(angle, 0.0);
            }
        else if (name.equals("skewY"))
            {
            double angle  = 0.0;
            if (!st2.hasMoreTokens())
                continue;
            angle = Double.parseDouble(st2.nextToken());
            trans.shear(0.0, angle);
            }
        }
    return true;
}



String coordToOdg(String sval)
{
    double nr = Double.parseDouble(sval);
    nr = nr * pxToCm;
    String s = nrfmt.format(nr) + "cm";
    return s;
}


boolean writeSvgAttributes(Element elem, AffineTransform trans)
{
    NamedNodeMap attrs = elem.getAttributes();
    String ename = elem.getLocalName();
    for (int i=0 ; i<attrs.getLength() ; i++)
        {
        Attr attr = (Attr)attrs.item(i);
        String aname = attr.getName();
        String aval  = attr.getValue();
        if (aname.startsWith("xmlns"))
            continue;
        else if (aname.equals("d"))//already handled
            continue;
        else if (aname.startsWith("transform"))
            {
            parseTransform(aval, trans);
            continue;
            }
        else if (aname.equals("style"))
            {
            StyleInfo style = styles.get(aval);
            if (style != null)
                {
                po(" draw:style-name=\"");
                po(style.getName());
                po("\"");
                }
            continue;
            }
        if (aname.equals("x") || aname.equals("y") ||
            aname.equals("width") || aname.equals("height"))
            {
            aval = coordToOdg(aval);
            }
        if ("id".equals(aname))
            po(" ");
        else if ("transform".equals(aname))
            po(" draw:");
        else
            po(" svg:");
        po(aname);
        po("=\"");
        po(aval);
        po("\"");
        }

    //Output the current transform
    if (!trans.isIdentity() &&
        !(
          ename.equals("g")     ||
          ename.equals("defs")  ||
          ename.equals("metadata")
         )
        )
        {
        double v[] = new double[6];
        trans.getMatrix(v);
        po(" draw:transform=\"matrix(" +
            nrfmt.format(v[0]) + "," +
            nrfmt.format(v[1]) + "," +
            nrfmt.format(v[2]) + "," +
            nrfmt.format(v[3]) + "," +
            nrfmt.format(v[4]) + "," +
            nrfmt.format(v[5]) + ")\"");
        }
    return true;
}

public boolean writeOdfContent(Element elem, AffineTransform trans)
{
    String ns = elem.getNamespaceURI();
    String tagName = elem.getLocalName();
    //trace("ns:" + ns + " tagName:" + tagName);
    if (!ns.equals(SVG_NS))
        return true;
    if (tagName.equals("svg"))
        {
        NodeList children = elem.getChildNodes();
        for (int i=0 ; i<children.getLength() ; i++)
            {
            Node n = children.item(i);
            if (n.getNodeType() == Node.ELEMENT_NODE)
                if (!writeOdfContent((Element)n,
                     (AffineTransform)trans.clone()))
                    return false;
            }
        }
    else if (tagName.equals("g"))
        {
        //String transform = elem.getAttribute("transform");
        po("<draw:g");
        writeSvgAttributes(elem, trans); po(">\n");
        NodeList children = elem.getChildNodes();
        for (int i=0 ; i<children.getLength() ; i++)
            {
            Node n = children.item(i);
            if (n.getNodeType() == Node.ELEMENT_NODE)
                if (!writeOdfContent((Element)n,
                     (AffineTransform)trans.clone()))
                    return false;
            }
        po("</draw:g>\n");
        }
    else if (tagName.equals("text"))
        {
        String x         = coordToOdg(elem.getAttribute("x"));
        String y         = coordToOdg(elem.getAttribute("y"));
        String width     = "5cm";
        String height    = "2cm";
        String txt       = elem.getTextContent();
        po("<draw:frame draw:style-name=\"grx1\" draw:layer=\"layout\" ");
        po("svg:x=\"" + x + "\" svg:y=\"" + y + "\" ");
        po("svg:width=\""  + width + "\" svg:height=\"" + height + "\"");
        po(">\n");
        po("    <draw:text-box draw:auto-grow-height=\"true\" draw:auto-grow-width=\"true\">\n");
        po("    <text:p text:style-name=\"P1\"> " + txt + "</text:p>\n");
        po("    </draw:text-box>\n");
        po("</draw:frame>\n");
        return true;
        }
    else if (tagName.equals("image"))
        {
        String x         = coordToOdg(elem.getAttribute("x"));
        String y         = coordToOdg(elem.getAttribute("y"));
        String width     = coordToOdg(elem.getAttribute("width"));
        String height    = coordToOdg(elem.getAttribute("height"));
        String imageName = elem.getAttributeNS(XLINK_NS, "href");
        po("<draw:frame draw:style-name=\"grx1\" draw:layer=\"layout\" ");
        po("svg:x=\"" + x + "\" svg:y=\"" + y + "\" ");
        po("svg:width=\""  + width + "\" svg:height=\"" + height + "\">\n");
        po("    <draw:image xlink:href=\"Pictures/" + imageName + "\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"><text:p/></draw:image>\n");
        po("</draw:frame>\n");
        return true;
        }
    else if (tagName.equals("path"))
        {
        double bounds[] = new double[4];
        String d      = elem.getAttribute("d");
        String newd   = parsePathData(d, bounds);
        double x      = bounds[0];
        double y      = bounds[1];
        double width  = (bounds[2]-bounds[0]);
        double height = (bounds[3]-bounds[1]);
        po("<draw:path draw:layer=\"layout\" \n");
        po("    svg:x=\"" + nrfmt.format(x) + "cm\" ");
        po("svg:y=\"" + nrfmt.format(y) + "cm\" ");
        po("svg:width=\""  + nrfmt.format(width) + "cm\" ");
        po("svg:height=\"" + nrfmt.format(height) + "cm\" ");
        po("svg:viewBox=\"0.0 0.0 " +
             nrfmt.format(bounds[2] * 1000.0) + " " +
             nrfmt.format(bounds[3] * 1000.0) + "\"\n");
        po("    svg:d=\"" + newd + "\"\n   ");

        writeSvgAttributes(elem, trans); po("/>\n");
        //po("    svg:d=\"" + d + "\"/>\n");
        return true;
        }

    else
        {
        //Verbatim tab mapping
        po("<draw:"); po(tagName);
        writeSvgAttributes(elem, trans); po(">\n");
        po("</draw:"); po(tagName); po(">\n");
        }

    return true;
}


boolean writeOdfContent(ZipOutputStream outs)
{
    try
        {
        ZipEntry ze = new ZipEntry("content.xml");
        outs.putNextEntry(ze);
        out = new BufferedWriter(new OutputStreamWriter(outs));
        }
    catch (IOException e)
        {
        return false;
        }

    NodeList res = svg.getElementsByTagNameNS(SVG_NS, "svg");
    if (res.getLength() < 1)
        {
        err("saveOdf: no <svg> root in .svg file");
        return false;
        }
    Element root = (Element)res.item(0);


    po("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    po("<office:document-content\n");
    po("    xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    po("    xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n");
    po("    xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n");
    po("    xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n");
    po("    xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n");
    po("    xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n");
    po("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    po("    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    po("    xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    po("    xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n");
    po("    xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    po("    xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n");
    po("    xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n");
    po("    xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n");
    po("    xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n");
    po("    xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n");
    po("    xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n");
    po("    xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    po("    xmlns:ooow=\"http://openoffice.org/2004/writer\"\n");
    po("    xmlns:oooc=\"http://openoffice.org/2004/calc\"\n");
    po("    xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n");
    po("    xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n");
    po("    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    po("    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
    po("    xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    po("    xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    po("    office:version=\"1.0\">\n");
    po("\n");
    po("\n");
    po("<office:scripts/>\n");
    po("<office:automatic-styles>\n");
    po("<style:style style:name=\"dp1\" style:family=\"drawing-page\"/>\n");
    po("<style:style style:name=\"grx1\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
    po("  <style:graphic-properties draw:stroke=\"none\" draw:fill=\"solid\" draw:textarea-horizontal-align=\"center\" draw:textarea-vertical-align=\"middle\" draw:color-mode=\"standard\" draw:luminance=\"0%\" draw:contrast=\"0%\" draw:gamma=\"100%\" draw:red=\"0%\" draw:green=\"0%\" draw:blue=\"0%\" fo:clip=\"rect(0cm 0cm 0cm 0cm)\" draw:image-opacity=\"100%\" style:mirror=\"none\"/>\n");
    po("</style:style>\n");
    po("<style:style style:name=\"P1\" style:family=\"paragraph\">\n");
    po("  <style:paragraph-properties fo:text-align=\"center\"/>\n");
    po("</style:style>\n");

    //##  Dump our style table
    for (StyleInfo s : styles.values())
        {
        po("<style:style style:name=\"" + s.getName() + "\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
        po("  <style:graphic-properties");
        po(" draw:fill=\"" + s.getFill() + "\"");
        if (!s.getFill().equals("none"))
            po(" draw:fill-color=\"" + s.getFillColor() + "\"");
        po(" draw:stroke=\"" + s.getStroke() + "\"");
        if (!s.getStroke().equals("none"))
            {
            po(" svg:stroke-width=\"" + s.getStrokeWidth() + "\"");
            po(" svg:stroke-color=\"" + s.getStrokeColor() + "\"");
            }
        po("/>\n");
        po("</style:style>\n");
        }
    po("</office:automatic-styles>\n");
    po("\n");
    po("\n");
    po("<office:body>\n");
    po("<office:drawing>\n");
    po("<draw:page draw:name=\"page1\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\">\n");
    po("\n\n\n");
    AffineTransform trans = new AffineTransform();
    //trans.scale(12.0, 12.0);
    po("<!-- ######### CONVERSION FROM SVG STARTS ######## -->\n");
    writeOdfContent(root, trans);
    po("<!-- ######### CONVERSION FROM SVG ENDS ######## -->\n");
    po("\n\n\n");

    po("</draw:page>\n");
    po("</office:drawing>\n");
    po("</office:body>\n");
    po("</office:document-content>\n");


    try
        {
        out.flush();
        outs.closeEntry();
        }
    catch (IOException e)
        {
        err("writeOdfContent:" + e);
        return false;
        }
    return true;
}

boolean writeOdfMeta(ZipOutputStream outs)
{
    try
        {
        ZipEntry ze = new ZipEntry("meta.xml");
        outs.putNextEntry(ze);
        out = new BufferedWriter(new OutputStreamWriter(outs));
        }
    catch (IOException e)
        {
        return false;
        }

    po("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    po("<office:document-meta\n");
    po("    xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    po("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    po("    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    po("    xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    po("    xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    po("    xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    po("    xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    po("    xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    po("    office:version=\"1.0\">\n");
    po("<office:meta>\n");
    po("    <meta:generator>Inkscape-0.43</meta:generator>\n");
    po("    <meta:initial-creator>clark kent</meta:initial-creator>\n");
    po("    <meta:creation-date>2005-12-10T10:55:13</meta:creation-date>\n");
    po("    <dc:creator>clark kent</dc:creator>\n");
    po("    <dc:date>2005-12-10T10:56:20</dc:date>\n");
    po("    <dc:language>en-US</dc:language>\n");
    po("    <meta:editing-cycles>2</meta:editing-cycles>\n");
    po("    <meta:editing-duration>PT1M13S</meta:editing-duration>\n");
    po("    <meta:user-defined meta:name=\"Info 1\"/>\n");
    po("    <meta:user-defined meta:name=\"Info 2\"/>\n");
    po("    <meta:user-defined meta:name=\"Info 3\"/>\n");
    po("    <meta:user-defined meta:name=\"Info 4\"/>\n");
    po("    <meta:document-statistic meta:object-count=\"2\"/>\n");
    po("</office:meta>\n");
    po("</office:document-meta>\n");


    try
        {
        out.flush();
        outs.closeEntry();
        }
    catch (IOException e)
        {
        err("writeOdfContent:" + e);
        return false;
        }
    return true;
}


boolean writeOdfManifest(ZipOutputStream outs)
{
    try
        {
        ZipEntry ze = new ZipEntry("META-INF/manifest.xml");
        outs.putNextEntry(ze);
        out = new BufferedWriter(new OutputStreamWriter(outs));
        }
    catch (IOException e)
        {
        return false;
        }

    po("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    po("<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">\n");
    po("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n");
    po("    <manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.graphics\" manifest:full-path=\"/\"/>\n");
    po("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>\n");
    po("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"meta.xml\"/>\n");
    po("    <!--List our images here-->\n");
    for (int i=0 ; i<images.size() ; i++)
        {
        ImageInfo ie = images.get(i);
        String fname = ie.getName();
        if (fname.length() < 5)
            {
            err("image file name too short:" + fname);
            return false;
            }
        String ext = fname.substring(fname.length() - 4);
        po("    <manifest:file-entry manifest:media-type=\"");
        if (ext.equals(".gif"))
            po("image/gif");
        else if (ext.equals(".png"))
            po("image/png");
        else if (ext.equals(".jpg") || ext.equals(".jpeg"))
            po("image/jpeg");
        po("\" manifest:full-path=\"");
        po(fname);
        po("\"/>\n");
        }
    po("</manifest:manifest>\n");

    try
        {
        out.flush();
        outs.closeEntry();
        }
    catch (IOException e)
        {
        err("writeOdfContent:" + e);
        return false;
        }
    return true;
}

boolean writeOdfImages(ZipOutputStream outs)
{
    for (int i=0 ; i<images.size() ; i++)
        {
        ImageInfo ie = images.get(i);
        try
            {
            String iname = "Pictures/" + ie.getName();
            ZipEntry ze = new ZipEntry(iname);
            outs.putNextEntry(ze);
            outs.write(ie.getBuf());
            outs.closeEntry();
            }
        catch (IOException e)
            {
            err("writing images:" + e);
            return false;
            }
        }
    return true;
}

/**
 *
 */
public boolean writeOdf(OutputStream outs)
{
    try
        {
        ZipOutputStream zos = new ZipOutputStream(outs);
        if (!writeOdfContent(zos))
            return false;
        if (!writeOdfManifest(zos))
            return false;
        if (!writeOdfMeta(zos))
            return false;
        if (!writeOdfImages(zos))
            return false;
        //if (!writeOdfStyles(zos))
        //    return false;
        zos.close();
        }
    catch (IOException e)
        {
        err("closing ODF zip output stream:" + e);
        return false;
        }
    return true;
}



/**
 *
 */
public boolean saveOdf(String fileName)
{
    boolean ret = true;
    try
        {
        FileOutputStream fos = new FileOutputStream(fileName);
        ret = writeOdf(fos);
        fos.close();
        }
    catch (IOException e)
        {
        err("writing odf " + fileName + " : " + e);
        return false;
        }
    return ret;
}



boolean parseCss(String css)
{
    trace("##### STYLE ### :" + css);
    String name = "gr" + styleNr;
    styleNr++;
    StyleInfo si = new StyleInfo(name, css);
    StringTokenizer st = new StringTokenizer(css, ";");
    while (st.hasMoreTokens())
        {
        String style = st.nextToken();
        //trace("    " + style);
        int pos = style.indexOf(':');
        if (pos < 1 || pos > style.length()-2)
            continue;
        String attrName = style.substring(0, pos);
        String attrVal  = style.substring(pos+1);
        trace("    =" + attrName + ':' + attrVal);
        if ("stroke".equals(attrName))
            {
            si.stroke = "solid";
            si.strokeColor = attrVal;
            }
        else if ("stroke-width".equals(attrName))
            {
            si.strokeWidth = attrVal;
            }
        else if ("fill".equals(attrName))
            {
            si.fill      = "solid";
            si.fillColor = attrVal;
            }
        }
    styles.put(css, si);
    return true;
}

boolean readSvg(InputStream ins)
{
    //### LOAD XML
    try
        {
        DocumentBuilderFactory factory =
            DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        DocumentBuilder builder =
            factory.newDocumentBuilder();
        builder.setEntityResolver(new EntityResolver()
               {
               public InputSource resolveEntity(String publicId, String systemId)
                   {
                   return new InputSource(new ByteArrayInputStream(new byte[0]));
                   }
               });
        Document doc = builder.parse(ins);
        svg = doc;
        }
    catch (javax.xml.parsers.ParserConfigurationException e)
        {
        err("making DOM parser:"+e);
        return false;
        }
    catch (org.xml.sax.SAXException e)
        {
        err("parsing svg document:"+e);
        return false;
        }
    catch (IOException e)
        {
        err("parsing svg document:"+e);
        return false;
        }
    //dumpDocument(svg);

    //### LOAD IMAGES
    imageNr = 0;
    images = new ArrayList<ImageInfo>();
    NodeList res = svg.getElementsByTagNameNS(SVG_NS, "image");
    for (int i=0 ; i<res.getLength() ; i++)
        {
        Element elem = (Element) res.item(i);
        String fileName = elem.getAttributeNS(XLINK_NS, "href");
        if (fileName == null)
            {
            err("No xlink:href pointer to image data for image");
            return false;
            }
        File f = new File(fileName);
        if (!f.exists())
            {
            err("image '" + fileName + "' does not exist");
            return false;
            }
        int bufSize = (int)f.length();
        byte buf[] = new byte[bufSize];
        int pos = 0;
        try
            {
            FileInputStream fis = new FileInputStream(fileName);
            while (pos < bufSize)
                {
                int len = fis.read(buf, pos, bufSize - pos);
                if (len < 0)
                    break;
                pos += len;
                }
            fis.close();
            }
        catch (IOException e)
            {
            err("reading image '" + fileName + "' :" + e);
            return false;
            }
        if (pos != bufSize)
            {
            err("reading image entry.  expected " +
                bufSize + ", found " + pos + " bytes.");
            return false;
            }
        ImageInfo ie = new ImageInfo(fileName, fileName, buf);
        images.add(ie);
        }

    //### Parse styles
    styleNr = 0;
    styles = new HashMap<String, StyleInfo>();
    res = svg.getElementsByTagName("*");
    for (int i=0 ; i<res.getLength() ; i++)
        {
        Element elem = (Element) res.item(i);
        trace("elem:"+ elem.getNodeName());
        String style = elem.getAttribute("style");
        if (style != null && style.length() > 0)
            parseCss(style);
        }

    return true;
}

boolean readSvg(String fileName)
{
    try
        {
        FileInputStream fis = new FileInputStream(fileName);
        if (!readSvg(fis))
            return false;
        fis.close();
        }
    catch (IOException e)
        {
        err("opening svg file:"+e);
        return false;
        }
    return true;
}


//########################################################################
//#  O D F    T O    S V G
//########################################################################

/**
 *
 */
public boolean readOdfEntry(ZipInputStream zis, ZipEntry ent)
{
    String fileName = ent.getName();
    trace("fileName:" + fileName);
    if (fileName.length() < 4)
        return true;
    String ext = fileName.substring(fileName.length() - 4);
    trace("ext:" + ext);
    ArrayList<Byte> arr = new ArrayList<Byte>();
    try
        {
        while (true)
            {
            int inb = zis.read();
            if (inb < 0)
                break;
            arr.add((byte)inb);
            }
        }
    catch (IOException e)
        {
        return false;
        }
    byte buf[] = new byte[arr.size()];
    for (int i=0 ; i<buf.length ; i++)
         buf[i] = arr.get(i);
    trace("bufsize:" + buf.length);

    if (ext.equals(".xml"))
        {
        try
            {
            DocumentBuilderFactory factory =
                DocumentBuilderFactory.newInstance();
            factory.setNamespaceAware(true);
            DocumentBuilder builder =
                factory.newDocumentBuilder();
            builder.setEntityResolver(new EntityResolver()
                   {
                   public InputSource resolveEntity(String publicId, String systemId)
                       {
                       return new InputSource(new ByteArrayInputStream(new byte[0]));
                       }
                   });
            //trace("doc:"+new String(buf));
            Document doc = builder.parse(new ByteArrayInputStream(buf));
            if (fileName.equals("content.xml"))
                content = doc;
            else if (fileName.equals("meta.xml"))
                meta = doc;
            else if (fileName.equals("styles.xml"))
                {
                //styles = doc;
                }
            }
        catch (javax.xml.parsers.ParserConfigurationException e)
            {
            err("making DOM parser:"+e);
            return false;
            }
        catch (org.xml.sax.SAXException e)
            {
            err("parsing document:"+e);
            return false;
            }
        catch (IOException e)
            {
            err("parsing document:"+e);
            return false;
            }
        }
    else if (ext.equals(".png")  ||
             ext.equals(".gif")  ||
             ext.equals(".jpg")  ||
             ext.equals(".jpeg")   )
        {
        String imageName = "image" + imageNr + ext;
        imageNr++;
        ImageInfo ie = new ImageInfo(fileName, imageName, buf);
        trace("added image '" + imageName + "'.  " + buf.length +" bytes.");
        images.add(ie);

        }
    dumpDocument(content);
    return true;
}


/**
 *
 */
public boolean readOdf(InputStream ins)
{
    imageNr = 0;
    images = new ArrayList<ImageInfo>();
    styleNr = 0;
    styles = new HashMap<String, StyleInfo>();
    ZipInputStream zis = new ZipInputStream(ins);
    while (true)
        {
        try
            {
            ZipEntry ent = zis.getNextEntry();
            if (ent == null)
                break;
            if (!readOdfEntry(zis, ent))
                return false;
            zis.closeEntry();
            }
        catch (IOException e)
            {
            err("reading zip entry");
            return false;
            }
        }

    return true;
}


/**
 *
 */
public boolean readOdf(String fileName)
{
    boolean ret = true;
    try
        {
        FileInputStream fis = new FileInputStream(fileName);
        ret = readOdf(fis);
        fis.close();
        }
    catch (IOException e)
        {
        err("reading " + fileName + " : " + e);
        ret = false;
        }
    return true;
}




public boolean writeSvgElement(Element elem)
{
    String ns = elem.getNamespaceURI();
    String tagName = elem.getLocalName();
    trace("tag:" + tagName + " : " + ns);
    if (ns.equals(ODSVG_NS))
        {
        po("<"); po(tagName);
        NamedNodeMap attrs = elem.getAttributes();
        for (int i=0 ; i<attrs.getLength() ; i++)
            {
            Attr attr = (Attr)attrs.item(i);
            String aname = attr.getName();
            String aval  = attr.getValue();
            //Replace image name
            if ("xlink:href".equals(aname) && "image".equals(tagName))
                {
                for (int j=0 ; j<images.size() ; j++)
                    {
                    ImageInfo ie = images.get(i);
                    if (aval.equals(ie.getName()))
                        aval = ie.getNewName();
                    }
                }
            po(" ");
            po(aname);
            po("=\"");
            po(aval);
            po("\"");
            }
        po(">\n");
        }
    NodeList children = elem.getChildNodes();
    for (int i=0 ; i<children.getLength() ; i++)
        {
        Node n = children.item(i);
        if (n.getNodeType() == Node.ELEMENT_NODE)
            if (!writeSvgElement((Element)n))
                return false;
        }
    if (ns.equals(ODSVG_NS))
        {
        po("</"); po(tagName); po(">\n");
        }
    return true;
}


public boolean saveSvg(String svgFileName)
{
    trace("====== Saving images ===========");
    try
        {
        for (int i=0 ; i<images.size() ; i++)
            {
            ImageInfo entry = images.get(i);
            trace("saving:" + entry.name);
            FileOutputStream fos = new FileOutputStream(entry.name);
            fos.write(entry.buf);
            fos.close();
            }
        }
    catch (IOException e)
        {
        err("saveAsSVG:" + e);
        return false;
        }

    try
        {
        out = new BufferedWriter(new FileWriter(svgFileName));
        }
    catch (IOException e)
        {
        err("save:" + e);
        return false;
        }

    if (content == null)
        {
        err("no content in odf");
        return false;
        }

    NodeList res = content.getElementsByTagNameNS(ODF_NS, "drawing");
    if (res.getLength() < 1)
        {
        err("save: no drawing in document");
        return false;
        }
    Element root = (Element)res.item(0);
    trace("NS:"+root.getNamespaceURI());

    res = root.getElementsByTagNameNS(ODG_NS, "page");
    if (res.getLength() < 1)
        {
        err("save: no page in drawing");
        return false;
        }
    Element page = (Element)res.item(0);

    po("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    po("<svg\n");
    po("    xmlns:svg=\"http://www.w3.org/2000/svg\"\n");
    po("    xmlns=\"http://www.w3.org/2000/svg\"\n");
    po("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    po("    version=\"1.0\"\n");
    po("    >\n");

    writeSvgElement(page);

    po("</svg>\n");

    try
        {
        out.close();
        }
    catch (IOException e)
        {
        err("save:close:" + e);
        return false;
        }
    return true;
}



//########################################################################
//#  C O N S T R U C T O R
//########################################################################

SvgOdg()
{
    //init, if necessary
    nrfmt = new DecimalFormat("#.####");
}


//########################################################################
//#  C O M M A N D    L I N E
//########################################################################

public boolean odfToSvg(String odfName, String svgName)
{
    if (!readOdf(odfName))
        return false;
    if (!saveSvg(svgName))
        return false;
    return true;
}

public boolean svgToOdf(String svgName, String odfName)
{
    if (!readSvg(svgName))
        return false;
    System.out.println("ok");
    if (!saveOdf(odfName))
        return false;
    return true;
}

void usage()
{
    System.out.println("usage: SvgOdf input_file.odg output_file.svg");
    System.out.println("       SvgOdf input_file.svg output_file.odg");
}


boolean parseArguments(String argv[])
{
    if (argv.length != 2)
        {
        usage();
        return false;
        }

    String fileName1 = argv[0];
    String fileName2 = argv[1];

    if (fileName1.length()<5 || fileName2.length()<5)
        {
        System.out.println("one or more file names is too short");
        usage();
        return false;
        }

    String ext1 = fileName1.substring(fileName1.length()-4);
    String ext2 = fileName2.substring(fileName2.length()-4);
    //System.out.println("ext1:"+ext1+" ext2:"+ext2);

    //##### ODG -> SVG #####
    if ((ext1.equals(".odg") || ext1.equals(".odf") || ext1.equals(".zip") ) &&
        ext2.equals(".svg"))
        {
        if (!odfToSvg(argv[0], argv[1]))
            {
            System.out.println("Conversion from ODG to SVG failed");
            return false;
            }
        }

    //##### SVG -> ODG #####
    else if (ext1.equals(".svg") &&
             ( ext2.equals(".odg") || ext2.equals(".odf") || ext2.equals(".zip") ) )
        {
        if (!svgToOdf(fileName1, fileName2))
            {
            System.out.println("Conversion from SVG to ODG failed");
            return false;
            }
        }

    //##### none of the above #####
    else
        {
        usage();
        return false;
        }
    return true;
}


public static void main(String argv[])
{
    SvgOdg svgodg = new SvgOdg();
    svgodg.parseArguments(argv);
}


}

//########################################################################
//#  E N D    O F    F I L E
//########################################################################

