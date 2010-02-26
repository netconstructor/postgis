/**********************************************************************
 * $Id$
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.refractions.net
 * Copyright 2010 Olivier Courtin <olivier.courtin@oslandia.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CUnit/Basic.h"

#include "libgeom.h"
#include "cu_tester.h"

static void do_gml2_test(char * in, char * out, char * srs, int precision)
{
	LWGEOM *g;
	char *h;

	g = lwgeom_from_ewkt(in, PARSER_CHECK_NONE);
	h = lwgeom_to_gml2(lwgeom_serialize(g), srs, precision);

	if (strcmp(h, out))
		fprintf(stderr, "\nIn:   %s\nOut:  %s\nTheo: %s\n", in, h, out);

	CU_ASSERT_STRING_EQUAL(h, out);

	lwgeom_free(g);
	lwfree(h);
}


static void do_gml3_test(char * in, char * out, char * srs, int precision, int is_geodetic)
{
	LWGEOM *g;
	char *h;

	g = lwgeom_from_ewkt(in, PARSER_CHECK_NONE);
	h = lwgeom_to_gml3(lwgeom_serialize(g), srs, precision, is_geodetic);

	if (strcmp(h, out))
		fprintf(stderr, "\nIn:   %s\nOut:  %s\nTheo: %s\n", in, h, out);

	CU_ASSERT_STRING_EQUAL(h, out);

	lwgeom_free(g);
	lwfree(h);
}

static void do_gml2_unsupported(char * in, char * out)
{
	LWGEOM *g;
	char *h;

	g = lwgeom_from_ewkt(in, PARSER_CHECK_NONE);
	h = lwgeom_to_gml2(lwgeom_serialize(g), NULL, 0);

	if (strcmp(cu_error_msg, out))
		fprintf(stderr, "\nGML 2 - In:   %s\nOut:  %s\nTheo: %s\n",
		        in, cu_error_msg, out);
	CU_ASSERT_STRING_EQUAL(out, cu_error_msg);
	cu_error_msg_reset();

	lwfree(h);
	lwgeom_free(g);
}

static void do_gml3_unsupported(char * in, char * out)
{
	LWGEOM *g;
	char *h;

	g = lwgeom_from_ewkt(in, PARSER_CHECK_NONE);
	h = lwgeom_to_gml3(lwgeom_serialize(g), NULL, 0, 0);

	if (strcmp(cu_error_msg, out))
		fprintf(stderr, "\nGML 3 - In:   %s\nOut:  %s\nTheo: %s\n",
		        in, cu_error_msg, out);

	CU_ASSERT_STRING_EQUAL(out, cu_error_msg);
	cu_error_msg_reset();

	lwfree(h);
	lwgeom_free(g);
}


static void out_gml_test_precision(void)
{
	/* GML2 - 0 precision, i.e a round */
	do_gml2_test(
	    "POINT(1.1111111111111 1.1111111111111)",
	    "<gml:Point><gml:coordinates>1,1</gml:coordinates></gml:Point>",
	    NULL, 0);

	/* GML3 - 0 precision, i.e a round */
	do_gml3_test(
	    "POINT(1.1111111111111 1.1111111111111)",
	    "<gml:Point><gml:pos srsDimension=\"2\">1 1</gml:pos></gml:Point>",
	    NULL, 0, 0);


	/* GML2 - 3 digits precision */
	do_gml2_test(
	    "POINT(1.1111111111111 1.1111111111111)",
	    "<gml:Point><gml:coordinates>1.111,1.111</gml:coordinates></gml:Point>",
	    NULL, 3);

	/* GML3 - 3 digits precision */
	do_gml3_test(
	    "POINT(1.1111111111111 1.1111111111111)",
	    "<gml:Point><gml:pos srsDimension=\"2\">1.111 1.111</gml:pos></gml:Point>",
	    NULL, 3, 0);


	/* GML2 - 9 digits precision */
	do_gml2_test(
	    "POINT(1.2345678901234 1.2345678901234)",
	    "<gml:Point><gml:coordinates>1.23456789,1.23456789</gml:coordinates></gml:Point>",
	    NULL, 9);

	/* GML3 - 9 digits precision */
	do_gml3_test(
	    "POINT(1.2345678901234 1.2345678901234)",
	    "<gml:Point><gml:pos srsDimension=\"2\">1.23456789 1.23456789</gml:pos></gml:Point>",
	    NULL, 9, 0);


	/* GML2 - huge data */
	do_gml2_test(
	    "POINT(1E300 -1E300)",
	    "<gml:Point><gml:coordinates>1e+300,-1e+300</gml:coordinates></gml:Point>",
	    NULL, 0);

	/* GML3 - huge data */
	do_gml3_test(
	    "POINT(1E300 -1E300)",
	    "<gml:Point><gml:pos srsDimension=\"2\">1e+300 -1e+300</gml:pos></gml:Point>",
	    NULL, 0, 0);
}

static void out_gml_test_srid(void)
{
	/* GML2 - Point with SRID */
	do_gml2_test(
	    "POINT(0 1)",
	    "<gml:Point srsName=\"EPSG:4326\"><gml:coordinates>0,1</gml:coordinates></gml:Point>",
	    "EPSG:4326", 0);

	/* GML3 - Point with SRID */
	do_gml3_test(
	    "POINT(0 1)",
	    "<gml:Point srsName=\"EPSG:4326\"><gml:pos srsDimension=\"2\">0 1</gml:pos></gml:Point>",
	    "EPSG:4326", 0, 0);


	/* GML2 - Linestring with SRID */
	do_gml2_test(
	    "LINESTRING(0 1,2 3,4 5)",
	    "<gml:LineString srsName=\"EPSG:4326\"><gml:coordinates>0,1 2,3 4,5</gml:coordinates></gml:LineString>",
	    "EPSG:4326", 0);

	/* GML3 - Linestring with SRID */
	do_gml3_test(
	    "LINESTRING(0 1,2 3,4 5)",
	    "<gml:Curve srsName=\"EPSG:4326\"><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">0 1 2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve>",
	    "EPSG:4326", 0, 0);


	/* GML2 Polygon with SRID */
	do_gml2_test(
	    "POLYGON((0 1,2 3,4 5,0 1))",
	    "<gml:Polygon srsName=\"EPSG:4326\"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1 2,3 4,5 0,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>",
	    "EPSG:4326", 0);

	/* GML3 Polygon with SRID */
	do_gml3_test(
	    "POLYGON((0 1,2 3,4 5,0 1))",
	    "<gml:Polygon srsName=\"EPSG:4326\"><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"2\">0 1 2 3 4 5 0 1</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon>",
	    "EPSG:4326", 0, 0);


	/* GML2 MultiPoint with SRID */
	do_gml2_test(
	    "MULTIPOINT(0 1,2 3)",
	    "<gml:MultiPoint srsName=\"EPSG:4326\"><gml:pointMember><gml:Point><gml:coordinates>0,1</gml:coordinates></gml:Point></gml:pointMember><gml:pointMember><gml:Point><gml:coordinates>2,3</gml:coordinates></gml:Point></gml:pointMember></gml:MultiPoint>",
	    "EPSG:4326", 0);

	/* GML3 MultiPoint with SRID */
	do_gml3_test(
	    "MULTIPOINT(0 1,2 3)",
	    "<gml:MultiPoint srsName=\"EPSG:4326\"><gml:pointMember><gml:Point><gml:pos srsDimension=\"2\">0 1</gml:pos></gml:Point></gml:pointMember><gml:pointMember><gml:Point><gml:pos srsDimension=\"2\">2 3</gml:pos></gml:Point></gml:pointMember></gml:MultiPoint>",
	    "EPSG:4326", 0, 0);


	/* GML2 Multiline with SRID */
	do_gml2_test(
	    "MULTILINESTRING((0 1,2 3,4 5),(6 7,8 9,10 11))",
	    "<gml:MultiLineString srsName=\"EPSG:4326\"><gml:lineStringMember><gml:LineString><gml:coordinates>0,1 2,3 4,5</gml:coordinates></gml:LineString></gml:lineStringMember><gml:lineStringMember><gml:LineString><gml:coordinates>6,7 8,9 10,11</gml:coordinates></gml:LineString></gml:lineStringMember></gml:MultiLineString>",
	    "EPSG:4326", 0);


	/* GML3 Multiline with SRID */
	do_gml3_test(
	    "MULTILINESTRING((0 1,2 3,4 5),(6 7,8 9,10 11))",
	    "<gml:MultiCurve srsName=\"EPSG:4326\"><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">0 1 2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">6 7 8 9 10 11</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember></gml:MultiCurve>",
	    "EPSG:4326", 0, 0);


	/* GML2 MultiPolygon with SRID */
	do_gml2_test(
	    "MULTIPOLYGON(((0 1,2 3,4 5,0 1)),((6 7,8 9,10 11,6 7)))",
	    "<gml:MultiPolygon srsName=\"EPSG:4326\"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1 2,3 4,5 0,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,7 8,9 10,11 6,7</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon>",
	    "EPSG:4326", 0);

	/* GML3 MultiPolygon with SRID */
	do_gml3_test(
	    "MULTIPOLYGON(((0 1,2 3,4 5,0 1)),((6 7,8 9,10 11,6 7)))",
	    "<gml:MultiSurface srsName=\"EPSG:4326\"><gml:surfaceMember><gml:Polygon><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"2\">0 1 2 3 4 5 0 1</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember><gml:surfaceMember><gml:Polygon><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"2\">6 7 8 9 10 11 6 7</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember></gml:MultiSurface>",
	    "EPSG:4326", 0, 0);

	/* GML2 GeometryCollection with SRID */
	do_gml2_test(
	    "GEOMETRYCOLLECTION(POINT(0 1),LINESTRING(2 3,4 5))",
	    "<gml:MultiGeometry srsName=\"EPSG:4326\"><gml:geometryMember><gml:Point><gml:coordinates>0,1</gml:coordinates></gml:Point></gml:geometryMember><gml:geometryMember><gml:LineString><gml:coordinates>2,3 4,5</gml:coordinates></gml:LineString></gml:geometryMember></gml:MultiGeometry>",
	    "EPSG:4326", 0);

	/* GML3 GeometryCollection with SRID */
	do_gml3_test(
	    "GEOMETRYCOLLECTION(POINT(0 1),LINESTRING(2 3,4 5))",
	    "<gml:MultiGeometry srsName=\"EPSG:4326\"><gml:geometryMember><gml:Point><gml:pos srsDimension=\"2\">0 1</gml:pos></gml:Point></gml:geometryMember><gml:geometryMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:geometryMember></gml:MultiGeometry>",
	    "EPSG:4326", 0, 0);
}


static void out_gml_test_geodetic(void)
{
	/* GML3 - Geodetic Point */
	do_gml3_test(
	    "POINT(0 1)",
	    "<gml:Point srsName=\"urn:ogc:def:crs:EPSG:4326\"><gml:pos srsDimension=\"2\">1 0</gml:pos></gml:Point>",
	    "urn:ogc:def:crs:EPSG:4326", 0, 1);

	/* GML3 - 3D Geodetic Point */
	do_gml3_test(
	    "POINT(0 1 2)",
	    "<gml:Point srsName=\"urn:ogc:def:crs:EPSG:4326\"><gml:pos srsDimension=\"3\">1 0 2</gml:pos></gml:Point>",
	    "urn:ogc:def:crs:EPSG:4326", 0, 1);
}


static void out_gml_test_dims(void)
{
	/* GML2 - 3D */
	do_gml2_test(
	    "POINT(0 1 2)",
	    "<gml:Point><gml:coordinates>0,1,2</gml:coordinates></gml:Point>",
	    NULL, 0);

	/* GML3 - 3D */
	do_gml3_test(
	    "POINT(0 1 2)",
	    "<gml:Point><gml:pos srsDimension=\"3\">0 1 2</gml:pos></gml:Point>",
	    NULL, 0, 0);


	/* GML2 - 3DM */
	do_gml2_test(
	    "POINTM(0 1 2)",
	    "<gml:Point><gml:coordinates>0,1</gml:coordinates></gml:Point>",
	    NULL, 0);

	/* GML3 - 3DM */
	do_gml3_test(
	    "POINTM(0 1 2)",
	    "<gml:Point><gml:pos srsDimension=\"2\">0 1</gml:pos></gml:Point>",
	    NULL, 0, 0);


	/* GML2 - 4D */
	do_gml2_test(
	    "POINT(0 1 2 3)",
	    "<gml:Point><gml:coordinates>0,1,2</gml:coordinates></gml:Point>",
	    NULL, 0);

	/* GML3 - 4D */
	do_gml3_test(
	    "POINT(0 1 2 3)",
	    "<gml:Point><gml:pos srsDimension=\"3\">0 1 2</gml:pos></gml:Point>",
	    NULL, 0, 0);
}


static void out_gml_test_geoms(void)
{
	/* GML2 - Linestring */
	do_gml2_test(
	    "LINESTRING(0 1,2 3,4 5)",
	    "<gml:LineString><gml:coordinates>0,1 2,3 4,5</gml:coordinates></gml:LineString>",
	    NULL, 0);

	/* GML3 - Linestring */
	do_gml3_test(
	    "LINESTRING(0 1,2 3,4 5)",
	    "<gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">0 1 2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve>",
	    NULL, 0, 0);


	/* GML2 Polygon */
	do_gml2_test(
	    "POLYGON((0 1,2 3,4 5,0 1))",
	    "<gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1 2,3 4,5 0,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>",
	    NULL, 0);

	/* GML3 Polygon */
	do_gml3_test(
	    "POLYGON((0 1,2 3,4 5,0 1))",
	    "<gml:Polygon><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"2\">0 1 2 3 4 5 0 1</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon>",
	    NULL, 0, 0);


	/* GML2 Polygon - with internal ring */
	do_gml2_test(
	    "POLYGON((0 1,2 3,4 5,0 1),(6 7,8 9,10 11,6 7))",
	    "<gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1 2,3 4,5 0,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>6,7 8,9 10,11 6,7</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs></gml:Polygon>",
	    NULL, 0);

	/* GML3 Polygon - with internal ring */
	do_gml3_test(
	    "POLYGON((0 1,2 3,4 5,0 1),(6 7,8 9,10 11,6 7))",
	    "<gml:Polygon><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"2\">0 1 2 3 4 5 0 1</gml:posList></gml:LinearRing></gml:exterior><gml:interior><gml:LinearRing><gml:posList srsDimension=\"2\">6 7 8 9 10 11 6 7</gml:posList></gml:LinearRing></gml:interior></gml:Polygon>",
	    NULL, 0, 0);


	/* GML2 MultiPoint */
	do_gml2_test(
	    "MULTIPOINT(0 1,2 3)",
	    "<gml:MultiPoint><gml:pointMember><gml:Point><gml:coordinates>0,1</gml:coordinates></gml:Point></gml:pointMember><gml:pointMember><gml:Point><gml:coordinates>2,3</gml:coordinates></gml:Point></gml:pointMember></gml:MultiPoint>",
	    NULL, 0);

	/* GML3 MultiPoint */
	do_gml3_test(
	    "MULTIPOINT(0 1,2 3)",
	    "<gml:MultiPoint><gml:pointMember><gml:Point><gml:pos srsDimension=\"2\">0 1</gml:pos></gml:Point></gml:pointMember><gml:pointMember><gml:Point><gml:pos srsDimension=\"2\">2 3</gml:pos></gml:Point></gml:pointMember></gml:MultiPoint>",
	    NULL, 0, 0);


	/* GML2 Multiline */
	do_gml2_test(
	    "MULTILINESTRING((0 1,2 3,4 5),(6 7,8 9,10 11))",
	    "<gml:MultiLineString><gml:lineStringMember><gml:LineString><gml:coordinates>0,1 2,3 4,5</gml:coordinates></gml:LineString></gml:lineStringMember><gml:lineStringMember><gml:LineString><gml:coordinates>6,7 8,9 10,11</gml:coordinates></gml:LineString></gml:lineStringMember></gml:MultiLineString>",
	    NULL, 0);

	/* GML3 Multiline */
	do_gml3_test(
	    "MULTILINESTRING((0 1,2 3,4 5),(6 7,8 9,10 11))",
	    "<gml:MultiCurve><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">0 1 2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember><gml:curveMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">6 7 8 9 10 11</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:curveMember></gml:MultiCurve>",
	    NULL, 0, 0);


	/* GML2 MultiPolygon */
	do_gml2_test(
	    "MULTIPOLYGON(((0 1,2 3,4 5,0 1)),((6 7,8 9,10 11,6 7)))",
	    "<gml:MultiPolygon><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,1 2,3 4,5 0,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,7 8,9 10,11 6,7</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon>",
	    NULL, 0);

	/* GML3 MultiPolygon */
	do_gml3_test(
	    "MULTIPOLYGON(((0 1,2 3,4 5,0 1)),((6 7,8 9,10 11,6 7)))",
	    "<gml:MultiSurface><gml:surfaceMember><gml:Polygon><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"2\">0 1 2 3 4 5 0 1</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember><gml:surfaceMember><gml:Polygon><gml:exterior><gml:LinearRing><gml:posList srsDimension=\"2\">6 7 8 9 10 11 6 7</gml:posList></gml:LinearRing></gml:exterior></gml:Polygon></gml:surfaceMember></gml:MultiSurface>",
	    NULL, 0, 0);


	/* GML2 - GeometryCollection */
	do_gml2_test(
	    "GEOMETRYCOLLECTION(POINT(0 1),LINESTRING(2 3,4 5))",
	    "<gml:MultiGeometry><gml:geometryMember><gml:Point><gml:coordinates>0,1</gml:coordinates></gml:Point></gml:geometryMember><gml:geometryMember><gml:LineString><gml:coordinates>2,3 4,5</gml:coordinates></gml:LineString></gml:geometryMember></gml:MultiGeometry>",
	    NULL, 0);

	/* GML3 - GeometryCollection */
	do_gml3_test(
	    "GEOMETRYCOLLECTION(POINT(0 1),LINESTRING(2 3,4 5))",
	    "<gml:MultiGeometry><gml:geometryMember><gml:Point><gml:pos srsDimension=\"2\">0 1</gml:pos></gml:Point></gml:geometryMember><gml:geometryMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:geometryMember></gml:MultiGeometry>",
	    NULL, 0, 0);


	/* GML2 - Empty GeometryCollection */
	do_gml2_test(
	    "GEOMETRYCOLLECTION EMPTY",
	    "<gml:MultiGeometry></gml:MultiGeometry>",
	    NULL, 0);

	/* GML3 - Empty GeometryCollection */
	do_gml3_test(
	    "GEOMETRYCOLLECTION EMPTY",
	    "<gml:MultiGeometry></gml:MultiGeometry>",
	    NULL, 0, 0);

	/* GML2 - Nested GeometryCollection */
	do_gml2_test(
	    "GEOMETRYCOLLECTION(POINT(0 1),GEOMETRYCOLLECTION(LINESTRING(2 3,4 5)))",
	    "<gml:MultiGeometry><gml:geometryMember><gml:Point><gml:coordinates>0,1</gml:coordinates></gml:Point></gml:geometryMember><gml:geometryMember><gml:MultiGeometry><gml:geometryMember><gml:LineString><gml:coordinates>2,3 4,5</gml:coordinates></gml:LineString></gml:geometryMember></gml:MultiGeometry></gml:geometryMember></gml:MultiGeometry>",
	    NULL, 0);

	/* GML3 - Nested GeometryCollection */
	do_gml3_test(
	    "GEOMETRYCOLLECTION(POINT(0 1),GEOMETRYCOLLECTION(LINESTRING(2 3,4 5)))",
	    "<gml:MultiGeometry><gml:geometryMember><gml:Point><gml:pos srsDimension=\"2\">0 1</gml:pos></gml:Point></gml:geometryMember><gml:geometryMember><gml:MultiGeometry><gml:geometryMember><gml:Curve><gml:segments><gml:LineStringSegment><gml:posList srsDimension=\"2\">2 3 4 5</gml:posList></gml:LineStringSegment></gml:segments></gml:Curve></gml:geometryMember></gml:MultiGeometry></gml:geometryMember></gml:MultiGeometry>",
	    NULL, 0, 0);



	/* GML2 - CircularString */
	do_gml2_unsupported(
	    "CIRCULARSTRING(-2 0,0 2,2 0,0 2,2 4)",
	    "lwgeom_to_gml2: 'CircularString' geometry type not supported");
	/* GML3 - CircularString */
	do_gml3_unsupported(
	    "CIRCULARSTRING(-2 0,0 2,2 0,0 2,2 4)",
	    "lwgeom_to_gml3: 'CircularString' geometry type not supported");

	/* GML2 - CompoundString */
	do_gml2_unsupported(
	    "COMPOUNDCURVE(CIRCULARSTRING(0 0,1 1,1 0),(1 0,0 1))",
	    "lwgeom_to_gml2: 'CompoundString' geometry type not supported");
	/* GML3 - CompoundString */
	do_gml3_unsupported(
	    "COMPOUNDCURVE(CIRCULARSTRING(0 0,1 1,1 0),(1 0,0 1))",
	    "lwgeom_to_gml3: 'CompoundString' geometry type not supported");

	/* GML2 - CurvePolygon */
	do_gml2_unsupported(
	    "CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0))",
	    "lwgeom_to_gml2: 'CurvePolygon' geometry type not supported");

	/* GML3 - CurvePolygon */
	do_gml3_unsupported(
	    "CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0))",
	    "lwgeom_to_gml3: 'CurvePolygon' geometry type not supported");


	/* GML2 - MultiCurve */
	do_gml2_unsupported(
	    "MULTICURVE((5 5,3 5,3 3,0 3),CIRCULARSTRING(0 0,2 1,2 2))",
	    "lwgeom_to_gml2: 'MultiCurve' geometry type not supported");

	/* GML3 - MultiCurve */
	do_gml3_unsupported(
	    "MULTICURVE((5 5,3 5,3 3,0 3),CIRCULARSTRING(0 0,2 1,2 2))",
	    "lwgeom_to_gml3: 'MultiCurve' geometry type not supported");

	/* GML2 - MultiSurface */
	do_gml2_unsupported(
	    "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0)),((7 8,10 10,6 14,4 11,7 8)))",
	    "lwgeom_to_gml2: 'MultiSurface' geometry type not supported");

	/* GML3 - MultiSurface */
	do_gml3_unsupported(
	    "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(-2 0,-1 -1,0 0,1 -1,2 0,0 2,-2 0),(-1 0,0 0.5,1 0,0 1,-1 0)),((7 8,10 10,6 14,4 11,7 8)))",
	    "lwgeom_to_gml3: 'MultiSurface' geometry type not supported");
}

/*
** Used by test harness to register the tests in this file.
*/
CU_TestInfo out_gml_tests[] = {
	PG_TEST(out_gml_test_precision),
	PG_TEST(out_gml_test_srid),
	PG_TEST(out_gml_test_dims),
	PG_TEST(out_gml_test_geodetic),
	PG_TEST(out_gml_test_geoms),
	CU_TEST_INFO_NULL
};
CU_SuiteInfo out_gml_suite = {"GML Out Suite",  NULL,  NULL, out_gml_tests};