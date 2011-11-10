/**********************************************************************
 * $Id$
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.refractions.net
 * Copyright 2001-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "liblwgeom_internal.h"
#include "lwgeom_log.h"


LWMLINE *lwmcurve_segmentize(LWMCURVE *mcurve, uint32_t perQuad);
LWMPOLY *lwmsurface_segmentize(LWMSURFACE *msurface, uint32_t perQuad);
LWCOLLECTION *lwcollection_segmentize(LWCOLLECTION *collection, uint32_t perQuad);
LWGEOM *append_segment(LWGEOM *geom, POINTARRAY *pts, int type, int srid);
LWGEOM *pta_desegmentize(POINTARRAY *points, int type, int srid);
LWGEOM *lwline_desegmentize(LWLINE *line);
LWGEOM *lwpolygon_desegmentize(LWPOLY *poly);
LWGEOM *lwmline_desegmentize(LWMLINE *mline);
LWGEOM *lwmpolygon_desegmentize(LWMPOLY *mpoly);
LWGEOM *lwgeom_desegmentize(LWGEOM *geom);



/*
 * Tolerance used to determine equality.
 */
#define EPSILON_SQLMM 1e-8

/*
 * Determines (recursively in the case of collections) whether the geometry
 * contains at least on arc geometry or segment.
 */
int
lwgeom_has_arc(const LWGEOM *geom)
{
	LWCOLLECTION *col;
	int i;

	LWDEBUG(2, "lwgeom_has_arc called.");

	switch (geom->type)
	{
	case POINTTYPE:
	case LINETYPE:
	case POLYGONTYPE:
	case TRIANGLETYPE:
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTIPOLYGONTYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
		return LW_FALSE;
	case CIRCSTRINGTYPE:
		return LW_TRUE;
	/* It's a collection that MAY contain an arc */
	default:
		col = (LWCOLLECTION *)geom;
		for (i=0; i<col->ngeoms; i++)
		{
			if (lwgeom_has_arc(col->geoms[i]) == LW_TRUE) 
				return LW_TRUE;
		}
		return LW_FALSE;
	}
}

/*
 * Determines the center of the circle defined by the three given points.
 * In the event the circle is complete, the midpoint of the segment defined
 * by the first and second points is returned.  If the points are colinear,
 * as determined by equal slopes, then NULL is returned.  If the interior
 * point is coincident with either end point, they are taken as colinear.
 */
double
lwcircle_center(const POINT4D *p1, const POINT4D *p2, const POINT4D *p3, POINT4D *result)
{
	POINT4D c;
	double cx, cy, cr;
	double temp, bc, cd, det;

    c.x = c.y = c.z = c.m = 0.0;

	LWDEBUGF(2, "lwcircle_center called (%.16f,%.16f), (%.16f,%.16f), (%.16f,%.16f).", p1->x, p1->y, p2->x, p2->y, p3->x, p3->y);

	/* Closed circle */
	if (fabs(p1->x - p3->x) < EPSILON_SQLMM &&
	    fabs(p1->y - p3->y) < EPSILON_SQLMM)
	{
		cx = p1->x + (p2->x - p1->x) / 2.0;
		cy = p1->y + (p2->y - p1->y) / 2.0;
		c.x = cx;
		c.y = cy;
		*result = c;
		cr = sqrt(pow(cx - p1->x, 2.0) + pow(cy - p1->y, 2.0));
		return cr;
	}

	temp = p2->x*p2->x + p2->y*p2->y;
	bc = (p1->x*p1->x + p1->y*p1->y - temp) / 2.0;
	cd = (temp - p3->x*p3->x - p3->y*p3->y) / 2.0;
	det = (p1->x - p2->x)*(p2->y - p3->y)-(p2->x - p3->x)*(p1->y - p2->y);

	/* Check colinearity */
	if (fabs(det) < EPSILON_SQLMM)
		return -1.0;


	det = 1.0 / det;
	cx = (bc*(p2->y - p3->y)-cd*(p1->y - p2->y))*det;
	cy = ((p1->x - p2->x)*cd-(p2->x - p3->x)*bc)*det;
	c.x = cx;
	c.y = cy;
	*result = c;
	cr = sqrt((cx-p1->x)*(cx-p1->x)+(cy-p1->y)*(cy-p1->y));
	
	LWDEBUGF(2, "lwcircle_center center is (%.16f,%.16f)", result->x, result->y);
	
	return cr;
}


/*******************************************************************************
 * Begin curve segmentize functions
 ******************************************************************************/

static double interpolate_arc(double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3)
{
	LWDEBUGF(4,"angle %.05g a1 %.05g a2 %.05g a3 %.05g zm1 %.05g zm2 %.05g zm3 %.05g",angle,a1,a2,a3,zm1,zm2,zm3);
	/* Counter-clockwise sweep */
	if ( a1 < a2 )
	{
		if ( angle <= a2 )
			return zm1 + (zm2-zm1) * (angle-a1) / (a2-a1);
		else
			return zm2 + (zm3-zm2) * (angle-a2) / (a3-a2);
	}
	/* Clockwise sweep */
	else
	{
		if ( angle >= a2 )
			return zm1 + (zm2-zm1) * (a1-angle) / (a1-a2);
		else
			return zm2 + (zm3-zm2) * (a2-angle) / (a2-a3);
	}
}

static POINTARRAY *
lwcircle_segmentize(POINT4D *p1, POINT4D *p2, POINT4D *p3, uint32_t perQuad)
{
	POINT4D center;
	POINT4D pt;
	int p2_side = 0;
	int clockwise = LW_TRUE;
	double radius; /* Arc radius */
	double increment; /* Angle per segment */
	double a1, a2, a3, angle;
	POINTARRAY *pa;
	int result;
	int is_circle = LW_FALSE;

	LWDEBUG(2, "lwcircle_calculate_gbox called.");

	radius = lwcircle_center(p1, p2, p3, &center);
	p2_side = signum(lw_segment_side((POINT2D*)p1, (POINT2D*)p3, (POINT2D*)p2));

	/* Matched start/end points imply circle */
	if ( p1->x == p3->x && p1->y == p3->y )
		is_circle = LW_TRUE;
	
	/* Negative radius signals straight line, p1/p2/p3 are colinear */
	if ( radius < 0.0 || p2_side == 0 )
	    return NULL;
		
	/* The side of the p1/p3 line that p2 falls on dictates the sweep  
	   direction from p1 to p3. */
	if ( p2_side == -1 )
		clockwise = LW_TRUE;
	else
		clockwise = LW_FALSE;
		
	increment = fabs(M_PI_2 / perQuad);
	
	/* Angles of each point that defines the arc section */
	a1 = atan2(p1->y - center.y, p1->x - center.x);
	a2 = atan2(p2->y - center.y, p2->x - center.x);
	a3 = atan2(p3->y - center.y, p3->x - center.x);

	/* p2 on left side => clockwise sweep */
	if ( clockwise )
	{
		increment *= -1;
		/* Adjust a3 down so we can decrement from a1 to a3 cleanly */
		if ( a3 > a1 )
			a3 -= 2.0 * M_PI;
		if ( a2 > a1 )
			a2 -= 2.0 * M_PI;
	}
	/* p2 on right side => counter-clockwise sweep */
	else
	{
		/* Adjust a3 up so we can increment from a1 to a3 cleanly */
		if ( a3 < a1 )
			a3 += 2.0 * M_PI;
		if ( a2 < a1 )
			a2 += 2.0 * M_PI;
	}
	
	/* Override angles for circle case */
	if( is_circle )
	{
		a3 = a1 + 2.0 * M_PI;
		a2 = a1 + M_PI;
		increment = fabs(increment);
		clockwise = LW_FALSE;
	}
	
	/* Initialize point array */
	pa = ptarray_construct_empty(1, 1, 32);

	/* Sweep from a1 to a3 */
	for ( angle = a1; clockwise ? angle > a3 : angle < a3; angle += increment ) 
	{
		pt.x = center.x + radius * cos(angle);
		pt.y = center.y + radius * sin(angle);
		pt.z = interpolate_arc(angle, a1, a2, a3, p1->z, p2->z, p3->z);
		pt.m = interpolate_arc(angle, a1, a2, a3, p1->m, p2->m, p3->m);
		result = ptarray_append_point(pa, &pt, LW_FALSE);
	}	
	return pa;
}

LWLINE *
lwcircstring_segmentize(const LWCIRCSTRING *icurve, uint32_t perQuad)
{
	LWLINE *oline;
	POINTARRAY *ptarray;
	POINTARRAY *tmp;
	uint32_t i, j;
	POINT4D p1, p2, p3, p4;

	LWDEBUGF(2, "lwcircstring_segmentize called., dim = %d", icurve->points->flags);

	ptarray = ptarray_construct_empty(FLAGS_GET_Z(icurve->points->flags), FLAGS_GET_M(icurve->points->flags), 64);

	for (i = 2; i < icurve->points->npoints; i+=2)
	{
		LWDEBUGF(3, "lwcircstring_segmentize: arc ending at point %d", i);

		getPoint4d_p(icurve->points, i - 2, &p1);
		getPoint4d_p(icurve->points, i - 1, &p2);
		getPoint4d_p(icurve->points, i, &p3);
		tmp = lwcircle_segmentize(&p1, &p2, &p3, perQuad);

		if (tmp)
		{
			LWDEBUGF(3, "lwcircstring_segmentize: generated %d points", tmp->npoints);

			for (j = 0; j < tmp->npoints; j++)
			{
				getPoint4d_p(tmp, j, &p4);
				ptarray_append_point(ptarray, &p4, LW_TRUE);
			}
			ptarray_free(tmp);
		}
		else
		{
			LWDEBUG(3, "lwcircstring_segmentize: points are colinear, returning curve points as line");

			for (j = i - 1 ; j <= i ; j++)
			{
				getPoint4d_p(icurve->points, j, &p4);
				ptarray_append_point(ptarray, &p4, LW_TRUE);
			}
		}

	}
	getPoint4d_p(icurve->points, icurve->points->npoints-1, &p1);
	ptarray_append_point(ptarray, &p1, LW_TRUE);
		
	oline = lwline_construct(icurve->srid, NULL, ptarray);
	return oline;
}

LWLINE *
lwcompound_segmentize(const LWCOMPOUND *icompound, uint32_t perQuad)
{
	LWGEOM *geom;
	POINTARRAY *ptarray = NULL;
	LWLINE *tmp = NULL;
	uint32_t i, j;
	POINT4D p;

	LWDEBUG(2, "lwcompound_segmentize called.");

	ptarray = ptarray_construct_empty(FLAGS_GET_Z(icompound->flags), FLAGS_GET_M(icompound->flags), 64);

	for (i = 0; i < icompound->ngeoms; i++)
	{
		geom = icompound->geoms[i];
		if (geom->type == CIRCSTRINGTYPE)
		{
			tmp = lwcircstring_segmentize((LWCIRCSTRING *)geom, perQuad);
			for (j = 0; j < tmp->points->npoints; j++)
			{
				getPoint4d_p(tmp->points, j, &p);
				ptarray_append_point(ptarray, &p, LW_TRUE);
			}
			lwfree(tmp);
		}
		else if (geom->type == LINETYPE)
		{
			tmp = (LWLINE *)geom;
			for (j = 0; j < tmp->points->npoints; j++)
			{
				getPoint4d_p(tmp->points, j, &p);
				ptarray_append_point(ptarray, &p, LW_TRUE);
			}
		}
		else
		{
			lwerror("Unsupported geometry type %d found.",
			        geom->type, lwtype_name(geom->type));
			return NULL;
		}
	}
	return lwline_construct(icompound->srid, NULL, ptarray);
}

LWPOLY *
lwcurvepoly_segmentize(const LWCURVEPOLY *curvepoly, uint32_t perQuad)
{
	LWPOLY *ogeom;
	LWGEOM *tmp;
	LWLINE *line;
	POINTARRAY **ptarray;
	int i;

	LWDEBUG(2, "lwcurvepoly_segmentize called.");

	ptarray = lwalloc(sizeof(POINTARRAY *)*curvepoly->nrings);

	for (i = 0; i < curvepoly->nrings; i++)
	{
		tmp = curvepoly->rings[i];
		if (tmp->type == CIRCSTRINGTYPE)
		{
			line = lwcircstring_segmentize((LWCIRCSTRING *)tmp, perQuad);
			ptarray[i] = ptarray_clone_deep(line->points);
			lwfree(line);
		}
		else if (tmp->type == LINETYPE)
		{
			line = (LWLINE *)tmp;
			ptarray[i] = ptarray_clone_deep(line->points);
		}
		else if (tmp->type == COMPOUNDTYPE)
		{
			line = lwcompound_segmentize((LWCOMPOUND *)tmp, perQuad);
			ptarray[i] = ptarray_clone_deep(line->points);
			lwfree(line);
		}
		else
		{
			lwerror("Invalid ring type found in CurvePoly.");
			return NULL;
		}
	}

	ogeom = lwpoly_construct(curvepoly->srid, NULL, curvepoly->nrings, ptarray);
	return ogeom;
}

LWMLINE *
lwmcurve_segmentize(LWMCURVE *mcurve, uint32_t perQuad)
{
	LWMLINE *ogeom;
	LWGEOM *tmp;
	LWGEOM **lines;
	int i;

	LWDEBUGF(2, "lwmcurve_segmentize called, geoms=%d, dim=%d.", mcurve->ngeoms, FLAGS_NDIMS(mcurve->flags));

	lines = lwalloc(sizeof(LWGEOM *)*mcurve->ngeoms);

	for (i = 0; i < mcurve->ngeoms; i++)
	{
		tmp = mcurve->geoms[i];
		if (tmp->type == CIRCSTRINGTYPE)
		{
			lines[i] = (LWGEOM *)lwcircstring_segmentize((LWCIRCSTRING *)tmp, perQuad);
		}
		else if (tmp->type == LINETYPE)
		{
			lines[i] = (LWGEOM *)lwline_construct(mcurve->srid, NULL, ptarray_clone_deep(((LWLINE *)tmp)->points));
		}
		else
		{
			lwerror("Unsupported geometry found in MultiCurve.");
			return NULL;
		}
	}

	ogeom = (LWMLINE *)lwcollection_construct(MULTILINETYPE, mcurve->srid, NULL, mcurve->ngeoms, lines);
	return ogeom;
}

LWMPOLY *
lwmsurface_segmentize(LWMSURFACE *msurface, uint32_t perQuad)
{
	LWMPOLY *ogeom;
	LWGEOM *tmp;
	LWPOLY *poly;
	LWGEOM **polys;
	POINTARRAY **ptarray;
	int i, j;

	LWDEBUG(2, "lwmsurface_segmentize called.");

	polys = lwalloc(sizeof(LWGEOM *)*msurface->ngeoms);

	for (i = 0; i < msurface->ngeoms; i++)
	{
		tmp = msurface->geoms[i];
		if (tmp->type == CURVEPOLYTYPE)
		{
			polys[i] = (LWGEOM *)lwcurvepoly_segmentize((LWCURVEPOLY *)tmp, perQuad);
		}
		else if (tmp->type == POLYGONTYPE)
		{
			poly = (LWPOLY *)tmp;
			ptarray = lwalloc(sizeof(POINTARRAY *)*poly->nrings);
			for (j = 0; j < poly->nrings; j++)
			{
				ptarray[j] = ptarray_clone_deep(poly->rings[j]);
			}
			polys[i] = (LWGEOM *)lwpoly_construct(msurface->srid, NULL, poly->nrings, ptarray);
		}
	}
	ogeom = (LWMPOLY *)lwcollection_construct(MULTIPOLYGONTYPE, msurface->srid, NULL, msurface->ngeoms, polys);
	return ogeom;
}

LWCOLLECTION *
lwcollection_segmentize(LWCOLLECTION *collection, uint32_t perQuad)
{
	LWCOLLECTION *ocol;
	LWGEOM *tmp;
	LWGEOM **geoms;
	int i;

	LWDEBUG(2, "lwcollection_segmentize called.");

	geoms = lwalloc(sizeof(LWGEOM *)*collection->ngeoms);

	for (i=0; i<collection->ngeoms; i++)
	{
		tmp = collection->geoms[i];
		switch (tmp->type)
		{
		case CIRCSTRINGTYPE:
			geoms[i] = (LWGEOM *)lwcircstring_segmentize((LWCIRCSTRING *)tmp, perQuad);
			break;
		case COMPOUNDTYPE:
			geoms[i] = (LWGEOM *)lwcompound_segmentize((LWCOMPOUND *)tmp, perQuad);
			break;
		case CURVEPOLYTYPE:
			geoms[i] = (LWGEOM *)lwcurvepoly_segmentize((LWCURVEPOLY *)tmp, perQuad);
			break;
		case COLLECTIONTYPE:
			geoms[i] = (LWGEOM *)lwcollection_segmentize((LWCOLLECTION *)tmp, perQuad);
			break;
		default:
			geoms[i] = lwgeom_clone(tmp);
			break;
		}
	}
	ocol = lwcollection_construct(COLLECTIONTYPE, collection->srid, NULL, collection->ngeoms, geoms);
	return ocol;
}

LWGEOM *
lwgeom_segmentize(LWGEOM *geom, uint32_t perQuad)
{
	LWGEOM * ogeom = NULL;
	switch (geom->type)
	{
	case CIRCSTRINGTYPE:
		ogeom = (LWGEOM *)lwcircstring_segmentize((LWCIRCSTRING *)geom, perQuad);
		break;
	case COMPOUNDTYPE:
		ogeom = (LWGEOM *)lwcompound_segmentize((LWCOMPOUND *)geom, perQuad);
		break;
	case CURVEPOLYTYPE:
		ogeom = (LWGEOM *)lwcurvepoly_segmentize((LWCURVEPOLY *)geom, perQuad);
		break;
	case MULTICURVETYPE:
		ogeom = (LWGEOM *)lwmcurve_segmentize((LWMCURVE *)geom, perQuad);
		break;
	case MULTISURFACETYPE:
		ogeom = (LWGEOM *)lwmsurface_segmentize((LWMSURFACE *)geom, perQuad);
		break;
	case COLLECTIONTYPE:
		ogeom = (LWGEOM *)lwcollection_segmentize((LWCOLLECTION *)geom, perQuad);
		break;
	default:
		ogeom = lwgeom_clone(geom);
	}
	return ogeom;
}

/*******************************************************************************
 * End curve segmentize functions
 ******************************************************************************/
LWGEOM *
append_segment(LWGEOM *geom, POINTARRAY *pts, int type, int srid)
{
	LWGEOM *result;
	int currentType, i;

	LWDEBUGF(2, "append_segment called %p, %p, %d, %d", geom, pts, type, srid);

	if (geom == NULL)
	{
		if (type == LINETYPE)
		{
			LWDEBUG(3, "append_segment: line to NULL");

			return (LWGEOM *)lwline_construct(srid, NULL, pts);
		}
		else if (type == CIRCSTRINGTYPE)
		{
#if POSTGIS_DEBUG_LEVEL >= 4
			POINT4D tmp;

			LWDEBUGF(4, "append_segment: curve to NULL %d", pts->npoints);

			for (i=0; i<pts->npoints; i++)
			{
				getPoint4d_p(pts, i, &tmp);
				LWDEBUGF(4, "new point: (%.16f,%.16f)",tmp.x,tmp.y);
			}
#endif
			return (LWGEOM *)lwcircstring_construct(srid, NULL, pts);
		}
		else
		{
			lwerror("Invalid segment type %d - %s.", type, lwtype_name(type));
		}
	}

	currentType = geom->type;
	if (currentType == LINETYPE && type == LINETYPE)
	{
		POINTARRAY *newPoints;
		POINT4D pt;
		LWLINE *line = (LWLINE *)geom;

		LWDEBUG(3, "append_segment: line to line");

		newPoints = ptarray_construct(FLAGS_GET_Z(pts->flags), FLAGS_GET_M(pts->flags), pts->npoints + line->points->npoints - 1);
		for (i=0; i<line->points->npoints; i++)
		{
			getPoint4d_p(line->points, i, &pt);
			LWDEBUGF(5, "copying to %p [%d]", newPoints, i);
			ptarray_set_point4d(newPoints, i, &pt);
		}
		for (i=1; i<pts->npoints; i++)
		{
			getPoint4d_p(pts, i, &pt);
			ptarray_set_point4d(newPoints, i + line->points->npoints - 1, &pt);
		}
		result = (LWGEOM *)lwline_construct(srid, NULL, newPoints);
		lwgeom_release(geom);
		return result;
	}
	else if (currentType == CIRCSTRINGTYPE && type == CIRCSTRINGTYPE)
	{
		POINTARRAY *newPoints;
		POINT4D pt;
		LWCIRCSTRING *curve = (LWCIRCSTRING *)geom;

		LWDEBUG(3, "append_segment: circularstring to circularstring");

		newPoints = ptarray_construct(FLAGS_GET_Z(pts->flags), FLAGS_GET_M(pts->flags), pts->npoints + curve->points->npoints - 1);

		LWDEBUGF(3, "New array length: %d", pts->npoints + curve->points->npoints - 1);

		for (i=0; i<curve->points->npoints; i++)
		{
			getPoint4d_p(curve->points, i, &pt);

			LWDEBUGF(3, "orig point %d: (%.16f,%.16f)", i, pt.x, pt.y);

			ptarray_set_point4d(newPoints, i, &pt);
		}
		for (i=1; i<pts->npoints; i++)
		{
			getPoint4d_p(pts, i, &pt);

			LWDEBUGF(3, "new point %d: (%.16f,%.16f)", i + curve->points->npoints - 1, pt.x, pt.y);

			ptarray_set_point4d(newPoints, i + curve->points->npoints - 1, &pt);
		}
		result = (LWGEOM *)lwcircstring_construct(srid, NULL, newPoints);
		lwgeom_release(geom);
		return result;
	}
	else if (currentType == CIRCSTRINGTYPE && type == LINETYPE)
	{
		LWLINE *line;
		LWGEOM **geomArray;

		LWDEBUG(3, "append_segment: line to curve");

		geomArray = lwalloc(sizeof(LWGEOM *)*2);
		geomArray[0] = lwgeom_clone(geom);

		line = lwline_construct(srid, NULL, pts);
		geomArray[1] = lwgeom_clone((LWGEOM *)line);

		result = (LWGEOM *)lwcollection_construct(COMPOUNDTYPE, srid, NULL, 2, geomArray);
		lwfree((LWGEOM *)line);
		lwgeom_release(geom);
		return result;
	}
	else if (currentType == LINETYPE && type == CIRCSTRINGTYPE)
	{
		LWCIRCSTRING *curve;
		LWGEOM **geomArray;

		LWDEBUG(3, "append_segment: circularstring to line");

		geomArray = lwalloc(sizeof(LWGEOM *)*2);
		geomArray[0] = lwgeom_clone(geom);

		curve = lwcircstring_construct(srid, NULL, pts);
		geomArray[1] = lwgeom_clone((LWGEOM *)curve);

		result = (LWGEOM *)lwcollection_construct(COMPOUNDTYPE, srid, NULL, 2, geomArray);
		lwfree((LWGEOM *)curve);
		lwgeom_release(geom);
		return result;
	}
	else if (currentType == COMPOUNDTYPE)
	{
		LWGEOM *newGeom;
		LWCOMPOUND *compound;
		int count;
		LWGEOM **geomArray;

		compound = (LWCOMPOUND *)geom;
		count = compound->ngeoms + 1;
		geomArray = lwalloc(sizeof(LWGEOM *)*count);
		for (i=0; i<compound->ngeoms; i++)
		{
			geomArray[i] = lwgeom_clone(compound->geoms[i]);
		}
		if (type == LINETYPE)
		{
			LWDEBUG(3, "append_segment: line to compound");

			newGeom = (LWGEOM *)lwline_construct(srid, NULL, pts);
		}
		else if (type == CIRCSTRINGTYPE)
		{
			LWDEBUG(3, "append_segment: circularstring to compound");

			newGeom = (LWGEOM *)lwcircstring_construct(srid, NULL, pts);
		}
		else
		{
			lwerror("Invalid segment type %d - %s.", type, lwtype_name(type));
			return NULL;
		}
		geomArray[compound->ngeoms] = lwgeom_clone(newGeom);

		result = (LWGEOM *)lwcollection_construct(COMPOUNDTYPE, srid, NULL, count, geomArray);
		lwfree(newGeom);
		lwgeom_release(geom);
		return result;
	}
	lwerror("Invalid state [%d] %s - [%d] %s",
	        currentType, lwtype_name(currentType), type, lwtype_name(type));
	return NULL;
}

LWGEOM *
pta_desegmentize(POINTARRAY *points, int type, int srid)
{
	int i, j, commit, isline, count;
	double last_angle, last_length;
	double dxab, dyab, dxbc, dybc, theta, length;
	POINT4D a, b, c, tmp;
	POINTARRAY *pts;
	LWGEOM *geom = NULL;

	LWDEBUG(2, "pta_desegmentize called.");

	getPoint4d_p(points, 0, &a);
	getPoint4d_p(points, 1, &b);
	getPoint4d_p(points, 2, &c);

	dxab = b.x - a.x;
	dyab = b.y - a.y;
	dxbc = c.x - b.x;
	dybc = c.y - b.y;

	theta = atan2(dyab, dxab);
	last_angle = theta - atan2(dybc, dxbc);
	last_length = sqrt(dxbc*dxbc+dybc*dybc);
	length = sqrt(dxab*dxab+dyab*dyab);
	if ((last_length - length) < EPSILON_SQLMM)
	{
		isline = -1;
		LWDEBUG(3, "Starting as unknown.");
	}
	else
	{
		isline = 1;
		LWDEBUG(3, "Starting as line.");
	}

	commit = 0;
	for (i=3; i<points->npoints; i++)
	{
		getPoint4d_p(points, i-2, &a);
		getPoint4d_p(points, i-1, &b);
		getPoint4d_p(points, i, &c);

		dxab = b.x - a.x;
		dyab = b.y - a.y;
		dxbc = c.x - b.x;
		dybc = c.y - b.y;

		LWDEBUGF(3, "(dxab, dyab, dxbc, dybc) (%.16f, %.16f, %.16f, %.16f)", dxab, dyab, dxbc, dybc);

		theta = atan2(dyab, dxab);
		theta = theta - atan2(dybc, dxbc);
		length = sqrt(dxbc*dxbc+dybc*dybc);

		LWDEBUGF(3, "Last/current length and angle %.16f/%.16f, %.16f/%.16f", last_angle, theta, last_length, length);

		/* Found a line segment */
		if (fabs(length - last_length) > EPSILON_SQLMM ||
		        fabs(theta - last_angle) > EPSILON_SQLMM)
		{
			last_length = length;
			last_angle = theta;
			/* We are tracking a line, keep going */
			if (isline > 0)
				{}
			/* We were tracking a circularstring, commit it and start line*/
			else if (isline == 0)
			{
				LWDEBUGF(3, "Building circularstring, %d - %d", commit, i);

				count = i - commit;
				pts = ptarray_construct(
				          FLAGS_GET_Z(points->flags),
				          FLAGS_GET_M(points->flags),
				          3);
				getPoint4d_p(points, commit, &tmp);
				ptarray_set_point4d(pts, 0, &tmp);
				getPoint4d_p(points,
				             commit + count/2, &tmp);
				ptarray_set_point4d(pts, 1, &tmp);
				getPoint4d_p(points, i - 1, &tmp);
				ptarray_set_point4d(pts, 2, &tmp);

				commit = i-1;
				geom = append_segment(geom, pts, CIRCSTRINGTYPE, srid);
				isline = -1;

				/*
				 * We now need to move ahead one point to
				 * determine if it's a potential new curve,
				 * since the last_angle value is corrupt.
				 *
				 * Note we can only look ahead one point if
				 * we are not already at the end of our
				 * set of points.
				 */
				if (i < points->npoints - 1)
				{
					i++;

					getPoint4d_p(points, i-2, &a);
					getPoint4d_p(points, i-1, &b);
					getPoint4d_p(points, i, &c);

					dxab = b.x - a.x;
					dyab = b.y - a.y;
					dxbc = c.x - b.x;
					dybc = c.y - b.y;

					theta = atan2(dyab, dxab);
					last_angle = theta - atan2(dybc, dxbc);
					last_length = sqrt(dxbc*dxbc+dybc*dybc);
					length = sqrt(dxab*dxab+dyab*dyab);
					if ((last_length - length) < EPSILON_SQLMM)
					{
						isline = -1;
						LWDEBUG(3, "Restarting as unknown.");
					}
					else
					{
						isline = 1;
						LWDEBUG(3, "Restarting as line.");
					}
				}
				else
				{
					/* Indicate that we have tracked a CIRCSTRING */
					isline = 0;
				}
			}
			/* We didn't know what we were tracking, now we do. */
			else
			{
				LWDEBUG(3, "It's a line");
				isline = 1;
			}
		}
		/* Found a circularstring segment */
		else
		{
			/* We were tracking a circularstring, commit it and start line */
			if (isline > 0)
			{
				LWDEBUGF(3, "Building line, %d - %d", commit, i-2);

				count = i - commit - 2;

				pts = ptarray_construct(
				          FLAGS_GET_Z(points->flags),
				          FLAGS_GET_M(points->flags),
				          count);
				for (j=commit; j<i-2; j++)
				{
					getPoint4d_p(points, j, &tmp);
					ptarray_set_point4d(pts, j-commit, &tmp);
				}

				commit = i-3;
				geom = append_segment(geom, pts, LINETYPE, srid);
				isline = -1;
			}
			/* We are tracking a circularstring, keep going */
			else if (isline == 0)
			{
				;
			}
			/* We didn't know what we were tracking, now we do */
			else
			{
				LWDEBUG(3, "It's a circularstring");
				isline = 0;
			}
		}
	}
	count = i - commit;
	if (isline == 0 && count > 2)
	{
		LWDEBUGF(3, "Finishing circularstring %d,%d.", commit, i);

		pts = ptarray_construct(
		          FLAGS_GET_Z(points->flags),
		          FLAGS_GET_M(points->flags),
		          3);
		getPoint4d_p(points, commit, &tmp);
		ptarray_set_point4d(pts, 0, &tmp);
		getPoint4d_p(points, commit + count/2, &tmp);
		ptarray_set_point4d(pts, 1, &tmp);
		getPoint4d_p(points, i - 1, &tmp);
		ptarray_set_point4d(pts, 2, &tmp);

		geom = append_segment(geom, pts, CIRCSTRINGTYPE, srid);
	}
	else
	{
		LWDEBUGF(3, "Finishing line %d,%d.", commit, i);

		pts = ptarray_construct(
		          FLAGS_GET_Z(points->flags),
		          FLAGS_GET_M(points->flags),
		          count);
		for (j=commit; j<i; j++)
		{
			getPoint4d_p(points, j, &tmp);
			ptarray_set_point4d(pts, j-commit, &tmp);
		}
		geom = append_segment(geom, pts, LINETYPE, srid);
	}
	return geom;
}

LWGEOM *
lwline_desegmentize(LWLINE *line)
{
	LWDEBUG(2, "lwline_desegmentize called.");

	return pta_desegmentize(line->points, line->flags, line->srid);
}

LWGEOM *
lwpolygon_desegmentize(LWPOLY *poly)
{
	LWGEOM **geoms;
	int i, hascurve = 0;

	LWDEBUG(2, "lwpolygon_desegmentize called.");

	geoms = lwalloc(sizeof(LWGEOM *)*poly->nrings);
	for (i=0; i<poly->nrings; i++)
	{
		geoms[i] = pta_desegmentize(poly->rings[i], poly->flags, poly->srid);
		if (geoms[i]->type == CIRCSTRINGTYPE || geoms[i]->type == COMPOUNDTYPE)
		{
			hascurve = 1;
		}
	}
	if (hascurve == 0)
	{
		for (i=0; i<poly->nrings; i++)
		{
			lwfree(geoms[i]);
		}
		return lwgeom_clone((LWGEOM *)poly);
	}

	return (LWGEOM *)lwcollection_construct(CURVEPOLYTYPE, poly->srid, NULL, poly->nrings, geoms);
}

LWGEOM *
lwmline_desegmentize(LWMLINE *mline)
{
	LWGEOM **geoms;
	int i, hascurve = 0;

	LWDEBUG(2, "lwmline_desegmentize called.");

	geoms = lwalloc(sizeof(LWGEOM *)*mline->ngeoms);
	for (i=0; i<mline->ngeoms; i++)
	{
		geoms[i] = lwline_desegmentize((LWLINE *)mline->geoms[i]);
		if (geoms[i]->type == CIRCSTRINGTYPE || geoms[i]->type == COMPOUNDTYPE)
		{
			hascurve = 1;
		}
	}
	if (hascurve == 0)
	{
		for (i=0; i<mline->ngeoms; i++)
		{
			lwfree(geoms[i]);
		}
		return lwgeom_clone((LWGEOM *)mline);
	}
	return (LWGEOM *)lwcollection_construct(MULTICURVETYPE, mline->srid, NULL, mline->ngeoms, geoms);
}

LWGEOM *
lwmpolygon_desegmentize(LWMPOLY *mpoly)
{
	LWGEOM **geoms;
	int i, hascurve = 0;

	LWDEBUG(2, "lwmpoly_desegmentize called.");

	geoms = lwalloc(sizeof(LWGEOM *)*mpoly->ngeoms);
	for (i=0; i<mpoly->ngeoms; i++)
	{
		geoms[i] = lwpolygon_desegmentize((LWPOLY *)mpoly->geoms[i]);
		if (geoms[i]->type == CURVEPOLYTYPE)
		{
			hascurve = 1;
		}
	}
	if (hascurve == 0)
	{
		for (i=0; i<mpoly->ngeoms; i++)
		{
			lwfree(geoms[i]);
		}
		return lwgeom_clone((LWGEOM *)mpoly);
	}
	return (LWGEOM *)lwcollection_construct(MULTISURFACETYPE, mpoly->srid, NULL, mpoly->ngeoms, geoms);
}

LWGEOM *
lwgeom_desegmentize(LWGEOM *geom)
{
	LWDEBUG(2, "lwgeom_desegmentize called.");

	switch (geom->type)
	{
	case LINETYPE:
		return lwline_desegmentize((LWLINE *)geom);
	case POLYGONTYPE:
		return lwpolygon_desegmentize((LWPOLY *)geom);
	case MULTILINETYPE:
		return lwmline_desegmentize((LWMLINE *)geom);
	case MULTIPOLYGONTYPE:
		return lwmpolygon_desegmentize((LWMPOLY *)geom);
	default:
		return lwgeom_clone(geom);
	}
}

