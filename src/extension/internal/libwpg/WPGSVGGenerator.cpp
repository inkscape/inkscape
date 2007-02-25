/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "WPGSVGGenerator.h"

libwpg::WPGSVGGenerator::WPGSVGGenerator(std::ostream & output_sink): m_pen(libwpg::WPGPen()), m_brush(libwpg::WPGBrush()), m_fillRule(AlternatingFill), m_gradientIndex(1), m_outputSink(output_sink), m_oldLocale(output_sink.getloc())
{
	// set the stream's locale to "C" when printing floats
	std::locale c_locale("C");
	m_outputSink.imbue(c_locale);
}

libwpg::WPGSVGGenerator::~WPGSVGGenerator()
{
	m_outputSink.imbue(m_oldLocale);
}

void libwpg::WPGSVGGenerator::startDocument(double width, double height) 
{
	m_outputSink << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
	m_outputSink << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"";
	m_outputSink << " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";

	// m_outputSink << "<!-- Created with wpg2svg/libwpg " << LIBWPG_VERSION_STRING << " -->\n";

	m_outputSink << "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" ";
	m_outputSink << "xmlns:xlink=\"http://www.w3.org/1999/xlink\" ";	
	m_outputSink << "width=\"" << 72*width << "\" height=\"" << 72*height << "\" >\n";
	
	m_gradientIndex = 1;
}

void libwpg::WPGSVGGenerator::endDocument()
{
	m_outputSink << "</svg>\n";
}

void libwpg::WPGSVGGenerator::setPen(const libwpg::WPGPen& pen)
{
	m_pen = pen;
}

void libwpg::WPGSVGGenerator::setBrush(const libwpg::WPGBrush& brush)
{
	m_brush = brush;
	
	if(m_brush.style == libwpg::WPGBrush::Gradient)
	{
		double angle = m_brush.gradient.angle();

		m_outputSink << "<defs>\n";
		m_outputSink << "  <linearGradient id=\"grad" << m_gradientIndex++ << "\" >\n";
		for(unsigned c = 0; c < m_brush.gradient.count(); c++)
		{
			// round to nearest percentage
			int ofs = (int)((100.0*m_brush.gradient.stopOffset(c))+0.5);

			libwpg::WPGColor color = m_brush.gradient.stopColor(c);

			m_outputSink << "    <stop offset=\"" << ofs << "%\"";
			// set stream to %02x formatting
			size_t old_stream_size = m_outputSink.width(2);
			m_outputSink << std::hex;

			m_outputSink << " stop-color=\"#" << (color.red <= 0xF ? "0" : "") << color.red;
			m_outputSink << "" << (color.green <= 0xF ? "0" : "") << color.green;
			m_outputSink << "" << (color.blue <=  0xF ? "0" : "") << color.blue << "\" />\n";

			// reset stream formatting
			m_outputSink << std::dec;
			m_outputSink.width(old_stream_size);
		}
		m_outputSink << "  </linearGradient>\n";
		
		// not a simple horizontal gradient
		if(angle != -90.0)
		{
			m_outputSink << "  <linearGradient xlink:href=\"#grad" << m_gradientIndex-1 << "\"";
			m_outputSink << " id=\"grad" << m_gradientIndex++ << "\" ";
			m_outputSink << "x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" "; 
			m_outputSink << "gradientTransform=\"rotate(" << angle << ")\" ";
			m_outputSink << "gradientUnits=\"objectBoundingBox\" >\n";
			m_outputSink << "  </linearGradient>\n";
		}
		
		m_outputSink << "</defs>\n";
	}
}

void libwpg::WPGSVGGenerator::setFillRule(FillRule rule)
{
	m_fillRule = rule;
}

void libwpg::WPGSVGGenerator::startLayer(unsigned int id)
{
	m_outputSink << "<g id=\"Layer" << id << "\" >\n";
}

void libwpg::WPGSVGGenerator::endLayer(unsigned int)
{
	m_outputSink << "</g>\n";
}

void libwpg::WPGSVGGenerator::drawRectangle(const libwpg::WPGRect& rect, double rx, double ry)
{
	m_outputSink << "<rect ";
	m_outputSink << "x=\"" << 72*rect.x1 << "\" y=\"" << 72*rect.y1 << "\" ";
	m_outputSink << "width=\"" << 72*rect.width() << "\" height=\"" << 72*rect.height() << "\" ";
	if((rx !=0) || (ry !=0))
		m_outputSink << "rx=\"" << 72*rx << "\" ry=\"" << 72*ry << "\" ";
	writeStyle();
	m_outputSink << "/>\n";
}

void libwpg::WPGSVGGenerator::drawEllipse(const libwpg::WPGPoint& center, double rx, double ry)
{
	m_outputSink << "<ellipse ";
	m_outputSink << "cx=\"" << 72*center.x << "\" cy=\"" << 72*center.y << "\" ";
	m_outputSink << "rx=\"" << 72*rx << "\" ry=\"" << 72*ry << "\" ";
	writeStyle();
	m_outputSink << "/>\n";
}

void libwpg::WPGSVGGenerator::drawPolygon(const libwpg::WPGPointArray& vertices)
{
	if(vertices.count() < 2)
		return;

	if(vertices.count() == 2)
	{
		const libwpg::WPGPoint& p1 = vertices[0];
		const libwpg::WPGPoint& p2 = vertices[1];
		m_outputSink << "<line ";
		m_outputSink << "x1=\"" << 72*p1.x << "\"  y1=\"" << 72*p1.y << "\" ";
		m_outputSink << "x2=\"" << 72*p2.x << "\"  y2=\"" << 72*p2.y << "\"\n";
		writeStyle();
		m_outputSink << "/>\n";
	}
	else
	{
		m_outputSink << "<polyline ";
		m_outputSink << "points=\"";
		for(unsigned i = 0; i < vertices.count(); i++)
		{
			m_outputSink << 72*vertices[i].x << " " << 72*vertices[i].y;
			if(i < vertices.count()-1) m_outputSink << ", ";
		}
		m_outputSink << "\"\n";
		writeStyle();
		m_outputSink << "/>\n";
	}
}

void libwpg::WPGSVGGenerator::drawPath(const libwpg::WPGPath& path)
{
	m_outputSink << "<path d=\"";
	for(unsigned i = 0; i < path.count(); i++)
	{
		libwpg::WPGPathElement element = path.element(i);
		libwpg::WPGPoint point = element.point;
		switch(element.type)
		{
			case libwpg::WPGPathElement::MoveToElement:
				m_outputSink << "\n M" << 72*point.x << "," << 72*point.y << " ";
				break;
				
			case libwpg::WPGPathElement::LineToElement:
				m_outputSink << "\n L" << 72*point.x << "," << 72*point.y << " ";
				break;
			
			case libwpg::WPGPathElement::CurveToElement:
				m_outputSink << "C";
				m_outputSink << 72*element.extra1.x << "," << 72*element.extra1.y << " ";
				m_outputSink << 72*element.extra2.x << "," << 72*element.extra2.y << " ";
				m_outputSink << 72*point.x << "," << 72*point.y;
				break;
			
			default:
				break;
		}
	}
	
	if(path.closed)
		m_outputSink << "Z";

	m_outputSink << "\" \n";
	writeStyle();
	m_outputSink << "/>\n";
}

// create "style" attribute based on current pen and brush
void libwpg::WPGSVGGenerator::writeStyle()
{
	m_outputSink << "style=\"";

	const libwpg::WPGColor& color = m_pen.foreColor;
	m_outputSink << "stroke-width: " << 72*m_pen.width << "; ";
	if(m_pen.width > 0.0)
	{
		m_outputSink << "stroke: rgb(" << color.red << "," << color.green << "," << color.blue << "); ";
		if(color.alpha != 0)
			// alpha = 0 means opacity = 1.0, alpha = 256 means opacity = 0
			m_outputSink << "stroke-opacity: " << 1.0-(color.alpha/256.0) << "; ";
	}

	if(!m_pen.solid)
	{
		m_outputSink << "stroke-dasharray: ";
		for(unsigned i = 0; i < m_pen.dashArray.count(); i++)
		{
			m_outputSink << 72*m_pen.dashArray.at(i)*m_pen.width;
			if(i < m_pen.dashArray.count()-1) 
				m_outputSink << ", ";
		}
		m_outputSink << "; ";
	}
	
	if(m_brush.style == libwpg::WPGBrush::NoBrush)
		m_outputSink << "fill: none; ";

	if(m_fillRule == WPGSVGGenerator::WindingFill)
		m_outputSink << "fill-rule: nonzero; ";
	else if(m_fillRule == WPGSVGGenerator::AlternatingFill)
		m_outputSink << "fill-rule: evenodd; ";

	if(m_brush.style == libwpg::WPGBrush::Gradient)
		m_outputSink << "fill: url(#grad" << m_gradientIndex-1 << "); ";

	if(m_brush.style == libwpg::WPGBrush::Solid)
		m_outputSink << "fill: rgb(" << m_brush.foreColor.red << "," << m_brush.foreColor.green << "," << m_brush.foreColor.blue << "); ";

	m_outputSink << "\""; // style
}
