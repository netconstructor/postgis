/**********************************************************************
 * $Id$
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.refractions.net
 * Copyright 2001-2003 Refractions Research Inc.
 * Copyright 2009-2010 Olivier Courtin <olivier.courtin@oslandia.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of hte GNU General Public Licence. See the COPYING file.
 *
 **********************************************************************/

#include "liblwgeom_internal.h"
#include <math.h>   /* fabs */
#include <string.h> /* strlen */

void asgeojson_srs(json_object *output, char *srs);
void asgeojson_bbox(json_object *output, GBOX *bbox, int hasz, int precision);

static json_object * asgeojson_point(const LWPOINT *point, char *srs, GBOX *bbox, int precision);
static json_object * asgeojson_line(const LWLINE *line, char *srs, GBOX *bbox, int precision);
static json_object * asgeojson_poly(const LWPOLY *poly, char *srs, GBOX *bbox, int precision);
static json_object * asgeojson_multipoint(const LWMPOINT *mpoint, char *srs, GBOX *bbox, int precision);
static json_object * asgeojson_multiline(const LWMLINE *mline, char *srs, GBOX *bbox, int precision);
static json_object * asgeojson_multipolygon(const LWMPOLY *mpoly, char *srs, GBOX *bbox, int precision);
static json_object * asgeojson_collection(const LWCOLLECTION *col, char *srs, GBOX *bbox, int precision);
static json_object * asgeojson_geom(const LWGEOM *geom, GBOX *bbox, int precision);

static json_object * pointArray_to_geojson(POINTARRAY *pa, int precision);

/**
 * Takes a GEOMETRY and returns a GeoJson representation
 */
json_object *
lwgeom_to_geojson(const LWGEOM *geom, char *srs, int precision, int has_bbox)
{
    int type = geom->type;
    GBOX *bbox = NULL;
    GBOX tmp;
    int rv;

    if (has_bbox)
    {
        /* Whether these are geography or geometry,
           the GeoJSON expects a cartesian bounding box */
        rv = lwgeom_calculate_gbox_cartesian(geom, &tmp);
        bbox = &tmp;
    }

    switch (type)
    {
    case POINTTYPE:
        return asgeojson_point((LWPOINT*)geom, srs, bbox, precision);
    case LINETYPE:
        return asgeojson_line((LWLINE*)geom, srs, bbox, precision);
    case POLYGONTYPE:
        return asgeojson_poly((LWPOLY*)geom, srs, bbox, precision);
    case MULTIPOINTTYPE:
        return asgeojson_multipoint((LWMPOINT*)geom, srs, bbox, precision);
    case MULTILINETYPE:
        return asgeojson_multiline((LWMLINE*)geom, srs, bbox, precision);
    case MULTIPOLYGONTYPE:
        return asgeojson_multipolygon((LWMPOLY*)geom, srs, bbox, precision);
    case COLLECTIONTYPE:
        return asgeojson_collection((LWCOLLECTION*)geom, srs, bbox, precision);
    default:
        lwerror("lwgeom_to_geojson: '%s' geometry type not supported",
                lwtype_name(type));
    }

    /* Never get here */
    return NULL;
}



/**
 * Handle SRS
 */

void
asgeojson_srs(json_object *output, char *srs)
{
    json_object *ptr = output;

    json_object *properties = json_object_new_object();
    json_object_object_add( properties, "name", json_object_new_string(srs) );

    json_object *crs = json_object_new_object();
    json_object_object_add( crs, "type", json_object_new_string("name") );
    json_object_object_add( crs, "properties", properties );

    json_object_object_add(ptr, "crs", crs);
}



/**
 * Handle Bbox
 */

void
asgeojson_bbox(json_object *output, GBOX *bbox, int hasz, int precision)
{
    json_object *bboxArray = json_object_new_array();
    
    json_object_array_add(bboxArray, json_object_new_double(bbox->xmin));
    json_object_array_add(bboxArray, json_object_new_double(bbox->ymin));
    if (!hasz)
        json_object_array_add(bboxArray, json_object_new_double(bbox->zmin));

    json_object_array_add(bboxArray, json_object_new_double(bbox->xmax));
    json_object_array_add(bboxArray, json_object_new_double(bbox->ymax));
    if (!hasz)
        json_object_array_add(bboxArray, json_object_new_double(bbox->zmax));

    json_object_object_add(output, "bbox", bboxArray);
}



/**
 * Point Geometry
 */

static json_object *
asgeojson_point(const LWPOINT *point, char *srs, GBOX *bbox, int precision)
{
    json_object *output;
    output = json_object_new_object();

    json_object_object_add(output, "type", json_object_new_string("Point"));
    if (srs) asgeojson_srs(output, srs);
    if (bbox) asgeojson_bbox(output, bbox, FLAGS_GET_Z(point->flags), precision);

    json_object_object_add(output, "coordinates",  
                           json_object_array_get_idx(pointArray_to_geojson(point->point, precision),0));

    return output;
}



/**
 * Line Geometry
 */

static json_object *
asgeojson_line(const LWLINE *line, char *srs, GBOX *bbox, int precision)
{
    json_object *output = json_object_new_object();

    json_object_object_add(output, "type", json_object_new_string("LineString"));
    if (srs) asgeojson_srs(output, srs);
    if (bbox) asgeojson_bbox(output, bbox, FLAGS_GET_Z(line->flags), precision);

    json_object_object_add(output, "coordinates",  pointArray_to_geojson(line->points, precision));

    return output;
}



/**
 * Polygon Geometry
 */

static json_object *
asgeojson_poly(const LWPOLY *poly, char *srs, GBOX *bbox, int precision)
{
    int i;
    json_object *output = json_object_new_object();

    json_object_object_add(output, "type", json_object_new_string("Polygon"));
    if (srs) asgeojson_srs(output, srs);
    if (bbox) asgeojson_bbox(output, bbox, FLAGS_GET_Z(poly->flags), precision);

    json_object* coordinates = json_object_new_array();
    for (i=0; i<poly->nrings; i++)
    {
        json_object_array_add(coordinates, pointArray_to_geojson(poly->rings[i], precision));
    }
    json_object_object_add(output, "coordinates",  coordinates);

    return output;
}



/**
 * Multipoint Geometry
 */

static json_object *
asgeojson_multipoint(const LWMPOINT *mpoint, char *srs, GBOX *bbox, int precision)
{
    int i;
    json_object *output = json_object_new_object();

    json_object_object_add(output, "type", json_object_new_string("MultiPoint"));
    if (srs) asgeojson_srs(output, srs);
    if (bbox) asgeojson_bbox(output, bbox, FLAGS_GET_Z(mpoint->flags), precision);

    json_object* coordinates = json_object_new_array();
    for (i=0; i<mpoint->ngeoms; i++)
    {
        json_object_array_add(coordinates, pointArray_to_geojson(mpoint->geoms[i]->point, precision));
    }
    json_object_object_add(output, "coordinates",  coordinates);
    return output;
}



/**
 * Multiline Geometry
 */

static json_object *
asgeojson_multiline(const LWMLINE *mline, char *srs, GBOX *bbox, int precision)
{
    int i;
    json_object *output = json_object_new_object();

    json_object_object_add(output, "type", json_object_new_string("MultiLineString"));
    if (srs) asgeojson_srs(output, srs);
    if (bbox) asgeojson_bbox(output, bbox, FLAGS_GET_Z(mline->flags), precision);

    json_object* coordinates = json_object_new_array();
    for (i=0; i<mline->ngeoms; i++)
    {
        json_object_array_add(coordinates, pointArray_to_geojson(mline->geoms[i]->points, precision));
    }
    json_object_object_add(output, "coordinates",  coordinates);
    return output;
}



/**
 * MultiPolygon Geometry
 */


static json_object *
asgeojson_multipolygon(const LWMPOLY *mpoly, char *srs, GBOX *bbox, int precision)
{
    int i, j;
    json_object *output = json_object_new_object();

    json_object_object_add(output, "type", json_object_new_string("MultiPolygon"));
    if (srs) asgeojson_srs(output, srs);
    if (bbox) asgeojson_bbox(output, bbox, FLAGS_GET_Z(mpoly->flags), precision);

    json_object* coordinates = json_object_new_array();
    for (i=0; i<mpoly->ngeoms; i++)
    {
        json_object* poly = json_object_new_array();
        for (j=0 ; j < mpoly->geoms[i]->nrings ; j++)
        {
            json_object_array_add(poly, pointArray_to_geojson(mpoly->geoms[i]->rings[j], precision));
        }
        json_object_array_add(coordinates, poly);
    }
    json_object_object_add(output, "coordinates",  coordinates);

    return output;
}



/**
 * Collection Geometry
 */

static json_object *
asgeojson_collection(const LWCOLLECTION *col, char *srs, GBOX *bbox, int precision)
{
    int i;
    json_object *output = json_object_new_object();

    json_object_object_add(output, "type", json_object_new_string("GeometryCollection"));
    if (srs) asgeojson_srs(output, srs);
    if (bbox) asgeojson_bbox(output, bbox, FLAGS_GET_Z(col->flags), precision);

    json_object* geometries = json_object_new_array();
    for (i=0; i<col->ngeoms; i++)
    {
        json_object_array_add(geometries, asgeojson_geom(col->geoms[i], NULL, precision));
    }
    json_object_object_add(output, "geometries",  geometries);
    return output;
}

static json_object *
asgeojson_geom(const LWGEOM *geom, GBOX *bbox, int precision)
{
    int type = geom->type;

    switch (type)
    {
    case POINTTYPE:
        return asgeojson_point((LWPOINT*)geom, NULL, bbox, precision);
    case LINETYPE:
        return asgeojson_line((LWLINE*)geom, NULL, bbox, precision);
    case POLYGONTYPE:
        return asgeojson_poly((LWPOLY*)geom, NULL, bbox, precision);
    case MULTIPOINTTYPE:
        return asgeojson_multipoint((LWMPOINT*)geom, NULL, bbox, precision);
    case MULTILINETYPE:
        return asgeojson_multiline((LWMLINE*)geom, NULL, bbox, precision);
    case MULTIPOLYGONTYPE:
        return asgeojson_multipolygon((LWMPOLY*)geom, NULL, bbox, precision);
    default:
        if (bbox) lwfree(bbox);
        lwerror("asgeojson_geom: '%s' geometry type not supported",
                lwtype_name(type));
    }

    /* Never get here */
    return NULL;
}


static json_object *
pointArray_to_geojson(POINTARRAY *pa, int precision)
{
    int i;
    json_object *output = json_object_new_array();

    if (!FLAGS_GET_Z(pa->flags))
    {
        for (i=0; i<pa->npoints; i++)
        {
            POINT2D pt;
            json_object *pointArray = json_object_new_array();
        
            getPoint2d_p(pa, i, &pt);

            json_object_array_add(pointArray, json_object_new_double(pt.x));
            json_object_array_add(pointArray, json_object_new_double(pt.y));
            json_object_array_add( output, pointArray );
        }
    }
    else
    {
        for (i=0; i<pa->npoints; i++)
        {
            POINT4D pt;
            json_object *pointArray = json_object_new_array();
            
            getPoint4d_p(pa, i, &pt);
        
            json_object_array_add(pointArray, json_object_new_double(pt.x));
            json_object_array_add(pointArray, json_object_new_double(pt.y));
            json_object_array_add(pointArray, json_object_new_double(pt.z));
            json_object_array_add( output, pointArray );
        }
    }

    return output;
}
