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

#include <stdio.h>

#include "libwpg.h"
#include "WPGStreamImplementation.h"

using namespace libwpg;

class SvgPainter : public libwpg::WPGPaintInterface {
public:
	SvgPainter();

	void startDocument(double imageWidth, double imageHeight);
	void endDocument();
	void startLayer(unsigned int id);
	void endLayer(unsigned int id);

	void setPen(const WPGPen& pen);
	void setBrush(const WPGBrush& brush);
	void setFillRule(FillRule rule);

	void drawRectangle(const WPGRect& rect, double rx, double ry);
	void drawEllipse(const WPGPoint& center, double rx, double ry);
	void drawPolygon(const WPGPointArray& vertices);
	void drawPath(const WPGPath& path);

private:
	WPGPen m_pen;
	WPGBrush m_brush;
	FillRule m_fillRule;
	int m_gradientIndex;
	void writeStyle();
};

SvgPainter::SvgPainter(): m_fillRule(AlternatingFill), m_gradientIndex(1)
{
}

void SvgPainter::startDocument(double width, double height) 
{
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	printf("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"");
	printf(" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");

	printf("<!-- Created with wpg2svg/libwpg %s -->\n", LIBWPG_VERSION_STRING);

	printf("<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" ");
	printf("xmlns:xlink=\"http://www.w3.org/1999/xlink\" ");	
	printf("width=\"%g\" height=\"%f\" >\n", 72*width, 72*height);
	
	m_gradientIndex = 1;
}

void SvgPainter::endDocument()
{
	printf("</svg>\n");
}

void SvgPainter::setPen(const WPGPen& pen)
{
	m_pen = pen;
}

void SvgPainter::setBrush(const WPGBrush& brush)
{
	m_brush = brush;
	
	if(m_brush.style == WPGBrush::Gradient)
	{
		double angle = m_brush.gradient.angle();

		printf("<defs>\n");
		printf("  <linearGradient id=\"grad%d\" >\n", m_gradientIndex++);
		for(unsigned c = 0; c < m_brush.gradient.count(); c++)
		{
			// round to nearest percentage
			int ofs = (int)(100.0*m_brush.gradient.stopOffset(c)+0.5);

			WPGColor color = m_brush.gradient.stopColor(c);
			printf("    <stop offset=\"%d%%\" stop-color=\"#%02x%02x%02x\" />\n",
				ofs, color.red, color.green, color.blue);
		}
		printf("  </linearGradient>\n");
		
		// not a simple horizontal gradient
		if(angle != -90.0)
		{
			printf("  <linearGradient xlink:href=\"#grad%d\"", m_gradientIndex-1);
			printf(" id=\"grad%d\" ", m_gradientIndex++);
			printf("x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" "); 
			printf("gradientTransform=\"rotate(%f)\" ", angle);
			printf("gradientUnits=\"objectBoundingBox\" >\n");
			printf("  </linearGradient>\n");
		}
		
		printf("</defs>\n");
	}
}

void SvgPainter::setFillRule(FillRule rule)
{
	m_fillRule = rule;
}

void SvgPainter::startLayer(unsigned int id)
{
	printf("<g id=\"Layer%d\" >\n", id);
}

void SvgPainter::endLayer(unsigned int)
{
	printf("</g>\n");
}

void SvgPainter::drawRectangle(const WPGRect& rect, double rx, double ry)
{
	printf("<rect ");
	printf("x=\"%f\" y=\"%f\" ", 72*rect.x1, 72*rect.y1);
	printf("width=\"%f\" height=\"%f\" ", 72*rect.width(), 72*rect.height());
	if((rx !=0) || (ry !=0))
		printf("rx=\"%f\" ry=\"%f\" ", 72*rx, 72*ry);
	writeStyle();
	printf("/>\n");
}

void SvgPainter::drawEllipse(const WPGPoint& center, double rx, double ry)
{
	printf("<ellipse ");
	printf("cx=\"%f\" cy=\"%f\" ", 72*center.x, 72*center.y);
	printf("rx=\"%f\" ry=\"%f\" ", 72*rx, 72*ry);
	writeStyle();
	printf("/>\n");
}

void SvgPainter::drawPolygon(const WPGPointArray& vertices)
{
	if(vertices.count() < 2)
		return;

	if(vertices.count() == 2)
	{
		const WPGPoint& p1 = vertices[0];
		const WPGPoint& p2 = vertices[1];
		printf("<line ");
		printf("x1=\"%f\"  y1=\"%f\" ", 72*p1.x, 72*p1.y);
		printf("x2=\"%f\"  y2=\"%f\"\n", 72*p2.x, 72*p2.y);
		writeStyle();
		printf("/>\n");
	}
	else
	{
		printf("<polyline ");
		printf("points=\"");
		for(unsigned i = 0; i < vertices.count(); i++)
		{
			printf("%f %f", 72*vertices[i].x, 72*vertices[i].y);
			if(i < vertices.count()-1) printf(", ");
		}
		printf("\"\n");
		writeStyle();
		printf("/>\n");
	}
}

void SvgPainter::drawPath(const WPGPath& path)
{
	printf("<path d=\"");
	for(unsigned i = 0; i < path.count(); i++)
	{
		WPGPathElement element = path.element(i);
		WPGPoint point = element.point;
		switch(element.type)
		{
			case WPGPathElement::MoveToElement:
				printf("\n M%f,%f ", 72*point.x, 72*point.y );
				break;
				
			case WPGPathElement::LineToElement:
				printf("\n L%f,%f ", 72*point.x, 72*point.y );
				break;
			
			case WPGPathElement::CurveToElement:
				printf("C");
				printf("%f,%f ", 72*element.extra1.x, 72*element.extra1.y );
				printf("%f,%f ", 72*element.extra2.x, 72*element.extra2.y );
				printf("%f,%f", 72*point.x, 72*point.y );
				break;
			
			default:
				break;
		}
	}
	printf("\" \n");
	writeStyle();
	printf("/>\n");
}

// create "style" attribute based on current pen and brush
void SvgPainter::writeStyle()
{
	printf("style=\"");

	const WPGColor& color = m_pen.foreColor;
	printf("stroke-width: %f; ", 72*m_pen.width);
	if(m_pen.width > 0.0)
	{
		printf("stroke: rgb(%d,%d,%d); ", color.red, color.green, color.blue);
		if(color.alpha != 0)
			// alpha = 0 means opacity = 1.0, alpha = 256 means opacity = 0
			printf("stroke-opacity: %f; ", 1.0-(color.alpha/256.0));
	}

	if(!m_pen.solid)
	{
		printf("stroke-dasharray: ");
		for(unsigned i = 0; i < m_pen.dashArray.count(); i++)
		{
			printf("%f", 72*m_pen.dashArray.at(i)*m_pen.width);
			if(i < m_pen.dashArray.count()-1) 
				printf(", ");
		}
		printf("; ");
	}
	
	if(m_brush.style == WPGBrush::NoBrush)
		printf("fill: none; ");

	if(m_fillRule == SvgPainter::WindingFill)
		printf("fill-rule: nonzero; ");
	else if(m_fillRule == SvgPainter::AlternatingFill)
		printf("fill-rule: evenodd; ");

	if(m_brush.style == WPGBrush::Gradient)
		printf("fill: url(#grad%d); ", m_gradientIndex-1);

	if(m_brush.style == WPGBrush::Solid)
		printf("fill: rgb(%d,%d,%d); ", m_brush.foreColor.red, 
			m_brush.foreColor.green, m_brush.foreColor.blue);

	printf("\""); // style
}


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("usage: wpg2svg <WordPerfect Graphic>\n");
		return -1;
	}

	const char* filename = argv[1];
	WPGInputStream* input = new WPGFileStream(filename);
	if (input->isOle())
	{
		WPGInputStream* olestream = input->getWPGOleStream();
		if (olestream)
		{
			delete input;
			input = olestream;
		}
	}

	if (!WPGraphics::isSupported(input))
	{
		fprintf(stderr, "ERROR: Unsupported file format (unsupported version) or file is encrypted!\n");
		return 1;
	}
	
	SvgPainter painter;
	WPGraphics::parse(input, &painter);
	
	return 0;
}
