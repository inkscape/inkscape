#define __SP_CANVAS_BPATH_C__

/*
 * Simple bezier bpath CanvasItem for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Lauris Kaplinski and Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "sp-canvas-util.h"
#include "canvas-bpath.h"
#include "display/display-forward.h"
#include "display/curve.h"
#include <livarot/Shape.h>
#include <livarot/Path.h>
#include <livarot/int-line.h>
#include <livarot/BitLigne.h>
#include <libnr/nr-pixops.h>

void nr_pixblock_render_bpath_rgba (Shape* theS,uint32_t color,NRRectL &area,char* destBuf,int stride);

static void sp_canvas_bpath_class_init (SPCanvasBPathClass *klass);
static void sp_canvas_bpath_init (SPCanvasBPath *path);
static void sp_canvas_bpath_destroy (GtkObject *object);

static void sp_canvas_bpath_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags);
static void sp_canvas_bpath_render (SPCanvasItem *item, SPCanvasBuf *buf);
static double sp_canvas_bpath_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item);

static SPCanvasItemClass *parent_class;

GtkType
sp_canvas_bpath_get_type (void)
{
    static GtkType type = 0;
    if (!type) {
        GtkTypeInfo info = {
            (gchar *)"SPCanvasBPath",
            sizeof (SPCanvasBPath),
            sizeof (SPCanvasBPathClass),
            (GtkClassInitFunc) sp_canvas_bpath_class_init,
            (GtkObjectInitFunc) sp_canvas_bpath_init,
            NULL, NULL, NULL
        };
        type = gtk_type_unique (SP_TYPE_CANVAS_ITEM, &info);
    }
    return type;
}

static void
sp_canvas_bpath_class_init (SPCanvasBPathClass *klass)
{
    GtkObjectClass *object_class;
    SPCanvasItemClass *item_class;

    object_class = GTK_OBJECT_CLASS (klass);
    item_class = (SPCanvasItemClass *) klass;

    parent_class = (SPCanvasItemClass*)gtk_type_class (SP_TYPE_CANVAS_ITEM);

    object_class->destroy = sp_canvas_bpath_destroy;

    item_class->update = sp_canvas_bpath_update;
    item_class->render = sp_canvas_bpath_render;
    item_class->point = sp_canvas_bpath_point;
}

static void
sp_canvas_bpath_init (SPCanvasBPath * bpath)
{
    bpath->fill_rgba = 0x000000ff;
    bpath->fill_rule = SP_WIND_RULE_EVENODD;

    bpath->stroke_rgba = 0x00000000;
    bpath->stroke_width = 1.0;
    bpath->stroke_linejoin = SP_STROKE_LINEJOIN_MITER;
    bpath->stroke_linecap = SP_STROKE_LINECAP_BUTT;
    bpath->stroke_miterlimit = 11.0;

    bpath->fill_shp=NULL;
    bpath->stroke_shp=NULL;
}

static void
sp_canvas_bpath_destroy (GtkObject *object)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (object);
    if (cbp->fill_shp) {
        delete cbp->fill_shp;
        cbp->fill_shp = NULL;
    }

    if (cbp->stroke_shp) {
        delete cbp->stroke_shp;
        cbp->stroke_shp = NULL;
    }
    if (cbp->curve) {
        cbp->curve = sp_curve_unref (cbp->curve);
    }

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
sp_canvas_bpath_update (SPCanvasItem *item, NR::Matrix const &affine, unsigned int flags)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (item);

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);

    if (((SPCanvasItemClass *) parent_class)->update)
        ((SPCanvasItemClass *) parent_class)->update (item, affine, flags);

    sp_canvas_item_reset_bounds (item);

    if (cbp->fill_shp) {
        delete cbp->fill_shp;
        cbp->fill_shp = NULL;
    }

    if (cbp->stroke_shp) {
        delete cbp->stroke_shp;
        cbp->stroke_shp = NULL;
    }
    if (!cbp->curve) return;

    NRRect dbox;

    dbox.x0 = dbox.y0 = 0.0;
    dbox.x1 = dbox.y1 = -1.0;

    if ((cbp->fill_rgba & 0xff) || (cbp->stroke_rgba & 0xff)) {
        Path*  thePath=new Path;
        thePath->LoadArtBPath(SP_CURVE_BPATH(cbp->curve), affine, true);
        thePath->Convert(0.25);
        if ((cbp->fill_rgba & 0xff) && (cbp->curve->end > 2)) {
            Shape* theShape=new Shape;
            thePath->Fill(theShape,0);
            if ( cbp->fill_shp == NULL ) cbp->fill_shp=new Shape;
            if ( cbp->fill_rule == SP_WIND_RULE_EVENODD ) {
                cbp->fill_shp->ConvertToShape(theShape,fill_oddEven);
            } else {
                cbp->fill_shp->ConvertToShape(theShape,fill_nonZero);
            }
            delete theShape;
            cbp->fill_shp->CalcBBox();
            if ( cbp->fill_shp->leftX < cbp->fill_shp->rightX ) {
                if ( dbox.x0 >= dbox.x1 ) {
                    dbox.x0 = cbp->fill_shp->leftX; dbox.x1 = cbp->fill_shp->rightX;
                    dbox.y0 = cbp->fill_shp->topY; dbox.y1 = cbp->fill_shp->bottomY;
                } else {
                    if ( cbp->fill_shp->leftX < dbox.x0 ) dbox.x0=cbp->fill_shp->leftX;
                    if ( cbp->fill_shp->rightX > dbox.x1 ) dbox.x1=cbp->fill_shp->rightX;
                    if ( cbp->fill_shp->topY < dbox.y0 ) dbox.y0=cbp->fill_shp->topY;
                    if ( cbp->fill_shp->bottomY > dbox.y1 ) dbox.y1=cbp->fill_shp->bottomY;
                }
            }
        }
        if ((cbp->stroke_rgba & 0xff) && (cbp->curve->end > 1)) {
            JoinType join=join_straight;
//      Shape* theShape=new Shape;
            ButtType butt=butt_straight;
            if ( cbp->stroke_shp == NULL ) cbp->stroke_shp=new Shape;
            if ( cbp->stroke_linecap == SP_STROKE_LINECAP_BUTT ) butt=butt_straight;
            if ( cbp->stroke_linecap == SP_STROKE_LINECAP_ROUND ) butt=butt_round;
            if ( cbp->stroke_linecap == SP_STROKE_LINECAP_SQUARE ) butt=butt_square;
            if ( cbp->stroke_linejoin == SP_STROKE_LINEJOIN_MITER ) join=join_pointy;
            if ( cbp->stroke_linejoin == SP_STROKE_LINEJOIN_ROUND ) join=join_round;
            if ( cbp->stroke_linejoin == SP_STROKE_LINEJOIN_BEVEL ) join=join_straight;
            thePath->Stroke(cbp->stroke_shp,false,0.5*cbp->stroke_width, join,butt,cbp->stroke_width*cbp->stroke_miterlimit );
            //     thePath->Stroke(theShape,false,0.5*cbp->stroke_width, join,butt,cbp->stroke_width*cbp->stroke_miterlimit );
            //     cbp->stroke_shp->ConvertToShape(theShape,fill_nonZero);

            cbp->stroke_shp->CalcBBox();
            if ( cbp->stroke_shp->leftX < cbp->stroke_shp->rightX ) {
                if ( dbox.x0 >= dbox.x1 ) {
                    dbox.x0 = cbp->stroke_shp->leftX;dbox.x1 = cbp->stroke_shp->rightX;
                    dbox.y0 = cbp->stroke_shp->topY;dbox.y1 = cbp->stroke_shp->bottomY;
                } else {
                    if ( cbp->stroke_shp->leftX < dbox.x0 ) dbox.x0 = cbp->stroke_shp->leftX;
                    if ( cbp->stroke_shp->rightX > dbox.x1 ) dbox.x1 = cbp->stroke_shp->rightX;
                    if ( cbp->stroke_shp->topY < dbox.y0 ) dbox.y0 = cbp->stroke_shp->topY;
                    if ( cbp->stroke_shp->bottomY > dbox.y1 ) dbox.y1 = cbp->stroke_shp->bottomY;
                }
            }
//      delete theShape;
        }
        delete thePath;
    }


    item->x1 = (int)dbox.x0;
    item->y1 = (int)dbox.y0;
    item->x2 = (int)dbox.x1;
    item->y2 = (int)dbox.y1;

    sp_canvas_request_redraw (item->canvas, (int)item->x1, (int)item->y1, (int)item->x2, (int)item->y2);
}

static void
sp_canvas_bpath_render (SPCanvasItem *item, SPCanvasBuf *buf)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (item);

    sp_canvas_prepare_buffer(buf);

    NRRectL  area;
    area.x0=buf->rect.x0;
    area.x1=buf->rect.x1;
    area.y0=buf->rect.y0;
    area.y1=buf->rect.y1;
    if ( cbp->fill_shp ) {
        nr_pixblock_render_bpath_rgba (cbp->fill_shp,cbp->fill_rgba,area,(char*)buf->buf, buf->buf_rowstride);
    }
    if ( cbp->stroke_shp ) {
        nr_pixblock_render_bpath_rgba (cbp->stroke_shp,cbp->stroke_rgba,area,(char*)buf->buf, buf->buf_rowstride);
    }

}

#define BIGVAL 1e18

static double
sp_canvas_bpath_point (SPCanvasItem *item, NR::Point p, SPCanvasItem **actual_item)
{
    SPCanvasBPath *cbp = SP_CANVAS_BPATH (item);

    if (cbp->fill_shp && (cbp->fill_shp->PtWinding(p) > 0 )) {
        *actual_item = item;
        return 0.0;
    }
    if (cbp->stroke_shp ) {
        if (cbp->stroke_shp->PtWinding(p) > 0 ) {
            *actual_item = item;
            return 0.0;
        }
        return distance(cbp->stroke_shp, p);
    }
    if (cbp->fill_shp) {
        return distance(cbp->fill_shp, p);
    }

    return BIGVAL;
}

SPCanvasItem *
sp_canvas_bpath_new (SPCanvasGroup *parent, SPCurve *curve)
{
    g_return_val_if_fail (parent != NULL, NULL);
    g_return_val_if_fail (SP_IS_CANVAS_GROUP (parent), NULL);

    SPCanvasItem *item = sp_canvas_item_new (parent, SP_TYPE_CANVAS_BPATH, NULL);

    sp_canvas_bpath_set_bpath (SP_CANVAS_BPATH (item), curve);

    return item;
}

void
sp_canvas_bpath_set_bpath (SPCanvasBPath *cbp, SPCurve *curve)
{
    g_return_if_fail (cbp != NULL);
    g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

    if (cbp->curve) {
        cbp->curve = sp_curve_unref (cbp->curve);
    }

    if (curve) {
        cbp->curve = sp_curve_ref (curve);
    }

    sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}

void
sp_canvas_bpath_set_fill (SPCanvasBPath *cbp, guint32 rgba, SPWindRule rule)
{
    g_return_if_fail (cbp != NULL);
    g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

    cbp->fill_rgba = rgba;
    cbp->fill_rule = rule;

    sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}

void
sp_canvas_bpath_set_stroke (SPCanvasBPath *cbp, guint32 rgba, gdouble width, SPStrokeJoinType join, SPStrokeCapType cap)
{
    g_return_if_fail (cbp != NULL);
    g_return_if_fail (SP_IS_CANVAS_BPATH (cbp));

    cbp->stroke_rgba = rgba;
    cbp->stroke_width = MAX (width, 0.1);
    cbp->stroke_linejoin = join;
    cbp->stroke_linecap = cap;

    sp_canvas_item_request_update (SP_CANVAS_ITEM (cbp));
}



static void
bpath_run_A8_OR (raster_info &dest,void *data,int st,float vst,int en,float ven)
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
                d[0] = NR_COMPOSEN11_1111 (r, 255, d[0]);
                d[1] = NR_COMPOSEN11_1111 (g, 255, d[1]);
                d[2] = NR_COMPOSEN11_1111 (b, 255, d[2]);
                d += 3;
                len -= 1;
            }
        } else {
            unsigned int c0_24=(int)sv;
            c0_24&=0xFF;
            while (len > 0) {
                d[0] = NR_COMPOSEN11_1111 (r, c0_24, d[0]);
                d[1] = NR_COMPOSEN11_1111 (g, c0_24, d[1]);
                d[2] = NR_COMPOSEN11_1111 (b, c0_24, d[2]);
                d += 3;
                len -= 1;
            }
        }
    } else {
        if ( en <= st+1 ) {
            sv=0.5*(vst+ven);
            unsigned int c0_24=(int)sv;
            c0_24&=0xFF;
            d[0] = NR_COMPOSEN11_1111 (r, c0_24, d[0]);
            d[1] = NR_COMPOSEN11_1111 (g, c0_24, d[1]);
            d[2] = NR_COMPOSEN11_1111 (b, c0_24, d[2]);
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
                d[0] = NR_COMPOSEN11_1111 (r, ca, d[0]);
                d[1] = NR_COMPOSEN11_1111 (g, ca, d[1]);
                d[2] = NR_COMPOSEN11_1111 (b, ca, d[2]);
                d += 3;
                c0_24 += s0_24;
                c0_24 = CLAMP (c0_24, 0, 16777216);
                len -= 1;
            }
        }
    }
}

void nr_pixblock_render_bpath_rgba (Shape* theS,uint32_t color,NRRectL &area,char* destBuf,int stride)
{

    theS->CalcBBox();
    float  l=theS->leftX,r=theS->rightX,t=theS->topY,b=theS->bottomY;
    int    il,ir,it,ib;
    il=(int)floor(l);
    ir=(int)ceil(r);
    it=(int)floor(t);
    ib=(int)ceil(b);

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

    theS->DirectQuickScan(curY,curPt,(float)(it),true,0.25);

    char* mdata=(char*)destBuf;
    uint32_t* ligStart=((uint32_t*)(mdata+(3*(il-area.x0)+stride*(it-area.y0))));
    for (int y=it;y<ib;y++) {
        for (int i = 0; i < 4; i++)
            theI[i]->Reset();

        for (int i = 0; i < 4; i++)
            theS->QuickScan(curY, curPt, ((float)(y+0.25*(i+1))),
                            fill_oddEven, theI[i], 0.25);

        theIL->Copy(4,theI);
        //   theI[0]->Affiche();
        //   theIL->Affiche();

        raster_info  dest;
        dest.startPix=il;
        dest.endPix=ir;
        dest.sth=il;
        dest.stv=y;
        dest.buffer=ligStart;
        theIL->Raster(dest,&color,bpath_run_A8_OR);
        ligStart=((uint32_t*)(((char*)ligStart)+stride));
    }
    theS->EndQuickRaster();
    for (int i=0;i<4;i++) delete theI[i];
    delete theIL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
