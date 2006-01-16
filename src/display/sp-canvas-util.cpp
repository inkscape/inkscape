#define __SP_CANVAS_UTILS_C__

/*
 * Helper stuff for SPCanvas
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "libnr/nr-matrix-div.h"
#include "libnr/nr-pixops.h"
#include "sp-canvas-util.h"

#include <livarot/Shape.h>
#include <livarot/int-line.h>
#include <livarot/BitLigne.h>

void
sp_canvas_update_bbox (SPCanvasItem *item, int x1, int y1, int x2, int y2)
{
	sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
	item->x1 = x1;
	item->y1 = y1;
	item->x2 = x2;
	item->y2 = y2;
	sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

void
sp_canvas_item_reset_bounds (SPCanvasItem *item)
{
	item->x1 = 0.0;
	item->y1 = 0.0;
	item->x2 = 0.0;
	item->y2 = 0.0;
}

void
sp_canvas_prepare_buffer (SPCanvasBuf *buf)
{
	if (buf->is_empty) {
		sp_canvas_clear_buffer(buf);
		buf->is_empty = false;
	}
}

void
sp_canvas_clear_buffer (SPCanvasBuf *buf)
{
	unsigned char r, g, b;

	r = (buf->bg_color >> 16) & 0xff;
	g = (buf->bg_color >> 8) & 0xff;
	b = buf->bg_color & 0xff;

	if ((r != g) || (r != b)) {
		int x, y;
		for (y = buf->rect.y0; y < buf->rect.y1; y++) {
			unsigned char *p;
			p = buf->buf + (y - buf->rect.y0) * buf->buf_rowstride;
			for (x = buf->rect.x0; x < buf->rect.x1; x++) {
				*p++ = r;
				*p++ = g;
				*p++ = b;
			}
		}
	} else {
		int y;
		for (y = buf->rect.y0; y < buf->rect.y1; y++) {
			memset (buf->buf + (y - buf->rect.y0) * buf->buf_rowstride, r, 3 * (buf->rect.x1 - buf->rect.x0));
		}
	}
}

NR::Matrix sp_canvas_item_i2p_affine (SPCanvasItem * item)
{
	g_assert (item != NULL); // this may be overly zealous - it is
				 // plausible that this gets called
				 // with item == 0
	
	return item->xform;
}

NR::Matrix  sp_canvas_item_i2i_affine (SPCanvasItem * from, SPCanvasItem * to)
{
	g_assert (from != NULL);
	g_assert (to != NULL);

	return sp_canvas_item_i2w_affine(from) / sp_canvas_item_i2w_affine(to);
}

void sp_canvas_item_set_i2w_affine (SPCanvasItem * item,  NR::Matrix const &i2w)
{
	g_assert (item != NULL);

	sp_canvas_item_affine_absolute(item, i2w / sp_canvas_item_i2w_affine(item->parent));
}

void sp_canvas_item_move_to_z (SPCanvasItem * item, gint z)
{
	g_assert (item != NULL);

	gint current_z = sp_canvas_item_order (item);

	if (current_z == -1) // not found in its parent
		return;

	if (z == current_z)
		return;

	if (z > current_z)
		sp_canvas_item_raise (item, z - current_z);

	sp_canvas_item_lower (item, current_z - z);
}

gint
sp_canvas_item_compare_z (SPCanvasItem * a, SPCanvasItem * b)
{
	const gint o_a = sp_canvas_item_order (a);
	const gint o_b = sp_canvas_item_order (b);

	if (o_a > o_b) return -1;
	if (o_a < o_b) return 1;

	return 0;
}

// These two functions are used by canvasitems that use livarot (currently ctrlline and ctrlquadr)

void
ctrl_run_A8_OR (raster_info &dest,void *data,int st,float vst,int en,float ven)
{
    union {
        uint8_t  comp[4];
        uint32_t col;
    } tempCol;
    if ( st >= en ) return;
    tempCol.col=*(uint32_t*)data;
  
    unsigned int r, g, b, a;
    r = NR_RGBA32_R (tempCol.col);
    g = NR_RGBA32_G (tempCol.col);
    b = NR_RGBA32_B (tempCol.col);
    a = NR_RGBA32_A (tempCol.col);
    if (a == 0) return;
  
    vst*=a;
    ven*=a;
  
    if ( vst < 0 ) vst=0;
    if ( vst > 255 ) vst=255;
    if ( ven < 0 ) ven=0;
    if ( ven > 255 ) ven=255;
    float      sv=vst;
    float      dv=ven-vst;
    int        len=en-st;
    uint8_t*   d=(uint8_t*)dest.buffer;
  
    d+=3*(st-dest.startPix);
    if ( fabs(dv) < 0.001 ) {
        if ( sv > 249.999 ) {
            /* Simple copy */
            while (len > 0) {
                d[0] = INK_COMPOSE (r, 255, d[0]);
                d[1] = INK_COMPOSE (g, 255, d[1]);
                d[2] = INK_COMPOSE (b, 255, d[2]);
                d += 3;
                len -= 1;
            }
        } else {
            unsigned int c0_24=(int)sv;
            c0_24&=0xFF;
            while (len > 0) {
                d[0] = INK_COMPOSE (r, c0_24, d[0]);
                d[1] = INK_COMPOSE (g, c0_24, d[1]);
                d[2] = INK_COMPOSE (b, c0_24, d[2]);
                d += 3;
                len -= 1;
            }
        }
    } else {
        if ( en <= st+1 ) {
            sv=0.5*(vst+ven);
            unsigned int c0_24=(int)sv;
            c0_24&=0xFF;
            d[0] = INK_COMPOSE (r, c0_24, d[0]);
            d[1] = INK_COMPOSE (g, c0_24, d[1]);
            d[2] = INK_COMPOSE (b, c0_24, d[2]);
        } else {
            dv/=len;
            sv+=0.5*dv; // correction trapezoidale
            sv*=65536;
            dv*=65536;
            int c0_24 = static_cast<int>(CLAMP(sv, 0, 16777216));
            int s0_24 = static_cast<int>(dv);
            while (len > 0) {
                unsigned int ca;
                /* Draw */
                ca = c0_24 >> 16;
                if ( ca > 255 ) ca=255;
                d[0] = INK_COMPOSE (r, ca, d[0]);
                d[1] = INK_COMPOSE (g, ca, d[1]);
                d[2] = INK_COMPOSE (b, ca, d[2]);
                d += 3;
                c0_24 += s0_24;
                c0_24 = CLAMP (c0_24, 0, 16777216);
                len -= 1;
            }
        }
    }
}

void nr_pixblock_render_ctrl_rgba (Shape* theS,uint32_t color,NRRectL &area,char* destBuf,int stride)
{
  
    theS->CalcBBox();
    float  l=theS->leftX,r=theS->rightX,t=theS->topY,b=theS->bottomY;
    int    il,ir,it,ib;
    il=(int)floor(l);
    ir=(int)ceil(r);
    it=(int)floor(t);
    ib=(int)ceil(b);
  
//  printf("bbox %i %i %i %i  render %i %i %i %i\n",il,it,ir,ib,area.x0,area.y0,area.x1,area.y1);
  
    if ( il >= area.x1 || ir <= area.x0 || it >= area.y1 || ib <= area.y0 ) return;
    if ( il < area.x0 ) il=area.x0;
    if ( it < area.y0 ) it=area.y0;
    if ( ir > area.x1 ) ir=area.x1;
    if ( ib > area.y1 ) ib=area.y1;
  
/*  // version par FloatLigne
    int    curPt;
    float  curY;
    theS->BeginRaster(curY,curPt,1.0);
  
    FloatLigne* theI=new FloatLigne();
    IntLigne*   theIL=new IntLigne();
  
    theS->Scan(curY,curPt,(float)(it),1.0);
  
    char* mdata=(char*)destBuf;
    uint32_t* ligStart=((uint32_t*)(mdata+(3*(il-area.x0)+stride*(it-area.y0))));
    for (int y=it;y<ib;y++) {
    theI->Reset();
    if ( y&0x00000003 ) {
    theS->Scan(curY,curPt,((float)(y+1)),theI,false,1.0);
    } else {
    theS->Scan(curY,curPt,((float)(y+1)),theI,true,1.0);
    }
    theI->Flatten();
    theIL->Copy(theI);
    
    raster_info  dest;
    dest.startPix=il;
    dest.endPix=ir;
    dest.sth=il;
    dest.stv=y;
    dest.buffer=ligStart;
    theIL->Raster(dest,&color,bpath_run_A8_OR);
    ligStart=((uint32_t*)(((char*)ligStart)+stride));
    }
    theS->EndRaster();
    delete theI;
    delete theIL;  */
  
    // version par BitLigne directe
    int    curPt;
    float  curY;
    theS->BeginQuickRaster(curY, curPt);
  
    BitLigne*   theI[4];
    for (int i=0;i<4;i++) theI[i]=new BitLigne(il,ir);
    IntLigne*   theIL=new IntLigne();
  
    theS->QuickScan(curY,curPt,(float)(it),true,0.25);
  
    char* mdata=(char*)destBuf;
    uint32_t* ligStart=((uint32_t*)(mdata+(3*(il-area.x0)+stride*(it-area.y0))));
    for (int y=it;y<ib;y++) {
        for (int i=0;i<4;i++) theI[i]->Reset();
        theS->QuickScan(curY,curPt,((float)(y+0.25)),fill_oddEven,theI[0],0.25);
        theS->QuickScan(curY,curPt,((float)(y+0.5)),fill_oddEven,theI[1],0.25);
        theS->QuickScan(curY,curPt,((float)(y+0.75)),fill_oddEven,theI[2],0.25);
        theS->QuickScan(curY,curPt,((float)(y+1.0)),fill_oddEven,theI[3],0.25);
        theIL->Copy(4,theI);
    
        raster_info  dest;
        dest.startPix=il;
        dest.endPix=ir;
        dest.sth=il;
        dest.stv=y;
        dest.buffer=ligStart;
        theIL->Raster(dest,&color,ctrl_run_A8_OR);
        ligStart=((uint32_t*)(((char*)ligStart)+stride));
    }
    theS->EndQuickRaster();
    for (int i=0;i<4;i++) delete theI[i];
    delete theIL;    
}
