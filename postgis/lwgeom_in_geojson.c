#include "postgres.h"
#include "lwgeom_pg.h"
#include "liblwgeom.h"

#include <assert.h>
#include <json/json.h>
#include <json/json_object_private.h>

Datum geom_from_geojson(PG_FUNCTION_ARGS);
static LWGEOM* parse_geojson(json_object *geojson, bool *hasz,  int *root_srid);

static void geojson_lwerror(char *msg, int error_code) 
{
        POSTGIS_DEBUGF(3, "ST_GeomFromGeoJSON ERROR %i", error_code);
        lwerror("%s", msg);
}

json_object* findMemberByName(json_object* poObj, const char* pszName )
{
    if( NULL == pszName || NULL == poObj)
        return NULL;

    json_object* poTmp = poObj;

    json_object_iter it;
    it.key = NULL;
    it.val = NULL;
    it.entry = NULL;
    if( NULL != json_object_get_object(poTmp) )
    {
        assert( NULL != json_object_get_object(poTmp)->head );

        for( it.entry = json_object_get_object(poTmp)->head;
             ( it.entry ?
               ( it.key = (char*)it.entry->k,
                 it.val = (json_object*)it.entry->v, it.entry) : 0);
             it.entry = it.entry->next)
        {
            if( strcasecmp((char *)it.key, pszName )==0 )
                return it.val;
        }
    }

    return NULL;
}

PG_FUNCTION_INFO_V1(geom_from_geojson);
Datum geom_from_geojson(PG_FUNCTION_ARGS)
{
    PG_LWGEOM *geom;
    LWGEOM *lwgeom;
    text *geojson_input;
    int geojson_size;
    char *geojson;
    int root_srid=0;
    bool hasz=true;
    json_tokener* jstok = NULL;
    json_object* poObj = NULL;

    /* Get the geojson stream */
    if (PG_ARGISNULL(0)) PG_RETURN_NULL();
    geojson_input = PG_GETARG_TEXT_P(0);
    geojson = text2cstring(geojson_input);
    geojson_size = VARSIZE(geojson_input) - VARHDRSZ;

    /* Begin to Parse json */
    jstok = json_tokener_new();
    poObj = json_tokener_parse_ex(jstok, geojson, -1);
    if( jstok->err != json_tokener_success)
    {
        char *err;
        err += sprintf(err, "%s (at offset %d)", json_tokener_errors[jstok->err], jstok->char_offset);
        json_tokener_free(jstok);
        geojson_lwerror(err, 1);
    }
    json_tokener_free(jstok);
    
    json_object* poObjSrs = findMemberByName( poObj, "crs" );
    if (poObjSrs != NULL) {
        json_object* poObjSrsType = findMemberByName( poObjSrs, "type" );
        if (poObjSrsType != NULL) {
            json_object* poObjSrsProps = findMemberByName( poObjSrs, "properties" );
            json_object* poNameURL = findMemberByName( poObjSrsProps, "name" );
            const char* pszName = json_object_get_string( poNameURL );
            root_srid = getSRIDbySRS(pszName);
            POSTGIS_DEBUGF(3, "getSRIDbySRS returned root_srid = %d.", root_srid );
        }
    }

    lwgeom = parse_geojson(poObj, &hasz, &root_srid);

    lwgeom_add_bbox(lwgeom);
    if (root_srid && lwgeom->srid == -1) lwgeom->srid = root_srid;

    if (!hasz)
    {
        LWGEOM *tmp = lwgeom_force_2d(lwgeom);
        lwgeom_free(lwgeom);
        lwgeom = tmp;
        
        POSTGIS_DEBUG(2, "geom_from_geojson called.");
    }

    geom = pglwgeom_serialize(lwgeom);
    lwgeom_free(lwgeom);

    PG_RETURN_POINTER(geom);
}

static POINT4D* parse_geojson_coord(json_object *poObj, bool *hasz)
{
    POINT4D pt;
    int iType = 0;

    POSTGIS_DEBUGF(3, "parse_geojson_coord called for object %s.", json_object_to_json_string( poObj ) );

    if( json_type_array == json_object_get_type( poObj ) )
    {

        const int nSize = json_object_array_length( poObj );
        POSTGIS_DEBUGF(3, "parse_geojson_coord called for array size %d.", nSize );

        json_object* poObjCoord = NULL;

        // Read X coordinate
        poObjCoord = json_object_array_get_idx( poObj, 0 );
        iType = json_object_get_type(poObjCoord);
        if (iType == json_type_double)
            pt.x = json_object_get_double( poObjCoord );
        else
            pt.x = json_object_get_int( poObjCoord );

        // Read Y coordiante
        poObjCoord = json_object_array_get_idx( poObj, 1 );
        if (iType == json_type_double)
            pt.y = json_object_get_double( poObjCoord );
        else
            pt.y = json_object_get_int( poObjCoord );

        *hasz = false;

        if( nSize == 3 )
        {
            // Read Z coordiante
            poObjCoord = json_object_array_get_idx( poObj, 2 );
            if (iType == 3)
                pt.z = json_object_get_double( poObjCoord );
            else
                pt.z = json_object_get_int( poObjCoord );

            *hasz = true;
        }
    }

    return &pt;
}

static LWGEOM* parse_geojson_point(json_object *geojson, bool *hasz,  int *root_srid)
{
    POSTGIS_DEBUGF(3, "parse_geojson_point called with root_srid = %d.", *root_srid );

    LWGEOM *geom;
    POINTARRAY *pa;
    json_object* coords = NULL;

    coords = findMemberByName( geojson, "coordinates" );
    
    pa = ptarray_construct_empty(1, 0, 1);
    ptarray_append_point(pa, parse_geojson_coord(coords, hasz), LW_FALSE);

    geom = (LWGEOM *) lwpoint_construct(*root_srid, NULL, pa);
    POSTGIS_DEBUG(2, "parse_geojson_point finished.");
    return geom;
}

static LWGEOM* parse_geojson_linestring(json_object *geojson, bool *hasz,  int *root_srid)
{
    POSTGIS_DEBUG(2, "parse_geojson_linestring called.");

    LWGEOM *geom;
    POINTARRAY *pa;
    json_object* points = NULL;
    int i = 0;

    points = findMemberByName( geojson, "coordinates" );

    pa = ptarray_construct_empty(1, 0, 1);

    if( json_type_array == json_object_get_type( points ) )
    {
        const int nPoints = json_object_array_length( points );
        for(i = 0; i < nPoints; ++i)
        {
            json_object* coords = NULL;
            coords = json_object_array_get_idx( points, i );
            ptarray_append_point(pa, parse_geojson_coord(coords, hasz), LW_FALSE);
        }
    }

    geom = (LWGEOM *) lwline_construct(*root_srid, NULL, pa);
    
    POSTGIS_DEBUG(2, "parse_geojson_linestring finished.");
    return geom;
}

static LWGEOM* parse_geojson_polygon(json_object *geojson, bool *hasz,  int *root_srid)
{
    LWGEOM *geom;
    POINTARRAY **ppa;
    json_object* rings = NULL;
    int i = 0, j = 0;
    int ring;

    rings = findMemberByName( geojson, "coordinates" );

    ppa = (POINTARRAY**) lwalloc(sizeof(POINTARRAY*));

    if( json_type_array == json_object_get_type( rings ) )
    {
        ppa[0] = ptarray_construct_empty(1, 0, 1);
        ring = json_object_array_length( rings );
        json_object* points = NULL;
        points = json_object_array_get_idx( rings, 0 );
        const int nPoints = json_object_array_length( points );

        for (i=0; i < nPoints; i++ ) {
            json_object* coords = NULL;
            coords = json_object_array_get_idx( points, i );
            ptarray_append_point(ppa[0], parse_geojson_coord(coords, hasz), LW_FALSE);
        }

        for(i = 1; i < ring; ++i)
        {
            ppa = (POINTARRAY**) lwrealloc((POINTARRAY *) ppa, sizeof(POINTARRAY*) * (i + 1));
            ppa[i] = ptarray_construct_empty(1, 0, 1);
            points = json_object_array_get_idx( rings, i );
            const int nPoints = json_object_array_length( points );
            for (j=0; j < nPoints; j++ ) {
                json_object* coords = NULL;
                coords = json_object_array_get_idx( points, j );
                ptarray_append_point( ppa[i], parse_geojson_coord(coords, hasz), LW_FALSE);
            }
        }
    }

    geom = (LWGEOM *) lwpoly_construct(*root_srid, NULL, ring, ppa);
    return geom;
}

static LWGEOM* parse_geojson_multipoint(json_object *geojson, bool *hasz,  int *root_srid)
{
    LWGEOM *geom;
    int i = 0;

    if (!*root_srid)
    {
        geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOINTTYPE, *root_srid, 1, 0);
    } else {
        geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOINTTYPE, -1, 1, 0);
    }

    json_object* poObjPoints = NULL;
    poObjPoints = findMemberByName( geojson, "coordinates" );

    if( json_type_array == json_object_get_type( poObjPoints ) )
    {
        const int nPoints = json_object_array_length( poObjPoints );
        for( i = 0; i < nPoints; ++i)
        {
            json_object* poObjCoords = NULL;
            poObjCoords = json_object_array_get_idx( poObjPoints, i );

            POINTARRAY *pa;
            pa = ptarray_construct_empty(1, 0, 1);
            ptarray_append_point(pa, parse_geojson_coord(poObjCoords, hasz), LW_FALSE);

            geom = (LWGEOM*)lwmpoint_add_lwpoint((LWMPOINT*)geom,
                 (LWPOINT*)lwpoint_construct(*root_srid, NULL, pa));
        }
    }

    return geom;
}

static LWGEOM* parse_geojson_multilinestring(json_object *geojson, bool *hasz,  int *root_srid)
{
    LWGEOM *geom = NULL;
    int i, j;

    if (!*root_srid)
    {
        geom = (LWGEOM *)lwcollection_construct_empty(MULTILINETYPE, *root_srid, 1, 0);
    }
    else
    {
        geom = (LWGEOM *)lwcollection_construct_empty(MULTILINETYPE, -1, 1, 0);
    }

    json_object* poObjLines = NULL;
    poObjLines = findMemberByName( geojson, "coordinates" );

    if( json_type_array == json_object_get_type( poObjLines ) )
    {
        const int nLines = json_object_array_length( poObjLines );
        for( i = 0; i < nLines; ++i) {
            json_object* poObjLine = NULL;
            poObjLine = json_object_array_get_idx( poObjLines, i );
            POINTARRAY *pa = NULL;
            pa = ptarray_construct_empty(1, 0, 1);

            if( json_type_array == json_object_get_type( poObjLine ) )
            {
                const int nPoints = json_object_array_length( poObjLine );
                for(j = 0; j < nPoints; ++j)
                {
                    json_object* coords = NULL;
                    coords = json_object_array_get_idx( poObjLine, j );
                    ptarray_append_point(pa, parse_geojson_coord(coords, hasz), LW_FALSE);
                }

                geom = (LWGEOM*)lwmline_add_lwline((LWMLINE*)geom, 
                    (LWMLINE*)lwline_construct(*root_srid, NULL, pa));
            }
        }
    }
    
    return geom;
}

static LWGEOM* parse_geojson_multipolygon(json_object *geojson, bool *hasz,  int *root_srid)
{
    LWGEOM *geom = NULL;
    int i, j, k;
    
    if (!*root_srid)
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOLYGONTYPE, *root_srid, 1, 0);
	}
	else
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOLYGONTYPE, -1, 1, 0);
	}
	
	json_object* poObjPolys = NULL;
	poObjPolys = findMemberByName( geojson, "coordinates" );
	
	if( json_type_array == json_object_get_type( poObjPolys ) )
	{
	    const int nPolys = json_object_array_length( poObjPolys );
        
	    for(i = 0; i < nPolys; ++i) {
	        json_object* poObjPoly = NULL;
            poObjPoly = json_object_array_get_idx( poObjPolys, i );
            
            POINTARRAY **ppa;
            ppa = (POINTARRAY**) lwalloc(sizeof(POINTARRAY*));
            
            if( json_type_array == json_object_get_type( poObjPoly ) )
            {
                ppa[0] = ptarray_construct_empty(1, 0, 1);
                int ring = json_object_array_length( poObjPoly );
                
                json_object* points = NULL;
                points = json_object_array_get_idx( poObjPoly, 0 );
                const int nPoints = json_object_array_length( points );

                for (j=0; j < nPoints; j++ ) {
                    json_object* coords = NULL;
                    coords = json_object_array_get_idx( points, j );
                    ptarray_append_point(ppa[0], parse_geojson_coord(coords, hasz), LW_FALSE);
                }

                for(j = 1; j < ring; ++j)
                {
                    ppa = (POINTARRAY**) lwrealloc((POINTARRAY *) ppa, sizeof(POINTARRAY*) * (j + 1));
                    ppa[i] = ptarray_construct_empty(1, 0, 1);
                    points = json_object_array_get_idx( poObjPoly, j );
                    
                    const int nPoints = json_object_array_length( points );
                    for (k=0; k < nPoints; k++ ) {
                        json_object* coords = NULL;
                        coords = json_object_array_get_idx( points, k );
                        ptarray_append_point( ppa[i], parse_geojson_coord(coords, hasz), LW_FALSE);
                    }
                }
               
               geom = (LWGEOM*)lwmpoly_add_lwpoly((LWMPOLY*)geom,
                     (LWPOLY*)lwpoly_construct(*root_srid, NULL, ring, ppa));
            
           }
        }
    }
    return geom;
}

static LWGEOM* parse_geojson(json_object *geojson, bool *hasz,  int *root_srid)
{
    json_object* type = NULL;

    if( NULL == geojson )
         geojson_lwerror("invalid GeoJSON representation", 2);

    type = findMemberByName( geojson, "type" );
    if( NULL == type )
         geojson_lwerror("unknown GeoJSON type", 3);

    const char* name = json_object_get_string( type );

    if( strcasecmp( name, "Point" )==0 )
        return parse_geojson_point(geojson, hasz, root_srid);

    if( strcasecmp( name, "LineString" )==0 )
        return parse_geojson_linestring(geojson, hasz, root_srid);

    if( strcasecmp( name, "Polygon" )==0 )
        return parse_geojson_polygon(geojson, hasz, root_srid);

    if( strcasecmp( name, "MultiPoint" )==0 )
        return parse_geojson_multipoint(geojson, hasz, root_srid);

    if( strcasecmp( name, "MultiLineString" )==0 )
        return parse_geojson_multilinestring(geojson, hasz, root_srid);

    if( strcasecmp( name, "MultiPolygon" )==0 )
        return parse_geojson_multipolygon(geojson, hasz, root_srid);
        
    lwerror("invalid GeoJson representation");
	return NULL; /* Never reach */
}
