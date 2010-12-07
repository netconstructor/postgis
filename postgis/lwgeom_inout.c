#include "postgres.h"

#include "../postgis_config.h"

#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "access/gist.h"
#include "access/itup.h"

#include "fmgr.h"
#include "utils/elog.h"
#include "mb/pg_wchar.h"
# include "lib/stringinfo.h" /* for binary input */


#include "liblwgeom.h"



#include "lwgeom_pg.h"
#include "profile.h"

void elog_ERROR(const char* string);


Datum LWGEOM_in(PG_FUNCTION_ARGS);
Datum LWGEOM_out(PG_FUNCTION_ARGS);
Datum LWGEOM_to_text(PG_FUNCTION_ARGS);
Datum LWGEOM_to_bytea(PG_FUNCTION_ARGS);
Datum LWGEOM_from_bytea(PG_FUNCTION_ARGS);
Datum LWGEOM_asHEXEWKB(PG_FUNCTION_ARGS);
Datum parse_WKT_lwgeom(PG_FUNCTION_ARGS);
Datum LWGEOM_recv(PG_FUNCTION_ARGS);
Datum LWGEOM_send(PG_FUNCTION_ARGS);
Datum LWGEOM_to_latlon(PG_FUNCTION_ARGS);


/*
 * LWGEOM_in(cstring)
 * format is '[SRID=#;]wkt|wkb'
 *  LWGEOM_in( 'SRID=99;POINT(0 0)')
 *  LWGEOM_in( 'POINT(0 0)')            --> assumes SRID=-1
 *  LWGEOM_in( 'SRID=99;0101000000000000000000F03F000000000000004')
 *  LWGEOM_in( '0101000000000000000000F03F000000000000004')
 *  returns a PG_LWGEOM object
 */
PG_FUNCTION_INFO_V1(LWGEOM_in);
Datum LWGEOM_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	char *str = input;
	LWGEOM_PARSER_RESULT lwg_parser_result;
	LWGEOM *lwgeom;
	PG_LWGEOM *ret;
	int srid = 0;

	lwgeom_parser_result_init(&lwg_parser_result);

	/* Empty string. */
	if ( str[0] == '\0' )
		ereport(ERROR,(errmsg("parse error - invalid geometry")));

	/* Starts with "SRID=" */
	if( strncasecmp(str,"SRID=",5) == 0 )
	{
		/* Roll forward to semi-colon */
		char *tmp = str;
		while ( tmp && *tmp != ';' )
			tmp++;
		
		/* Check next character to see if we have WKB  */
		if ( tmp && *(tmp+1) == '0' )
		{
			/* Null terminate the SRID= string */
			*tmp = '\0';
			/* Set str to the start of the real WKB */
			str = tmp + 1;
			/* Move tmp to the start of the numeric part */
			tmp = input + 5;
			/* Parse out the SRID number */
			srid = atoi(tmp);
		}
	}
	
	/* WKB? Let's find out. */
	if ( str[0] == '0' )
	{
		size_t hexsize = strlen(str);
		unsigned char *wkb = bytes_from_hexbytes(str, hexsize);
		/* TODO: 20101206: No parser checks! This is inline with current 1.5 behavior, but needs discussion */
		lwgeom = lwgeom_from_wkb(wkb, hexsize/2, PARSER_CHECK_NONE);
		/* If we picked up an SRID at the head of the WKB set it manually */
		if ( srid ) lwgeom_set_srid(lwgeom, srid);
		/* Add a bbox if necessary */
		if ( lwgeom_needs_bbox(lwgeom) ) lwgeom_add_bbox(lwgeom);
		pfree(wkb);
		ret = pglwgeom_serialize(lwgeom);
		lwgeom_free(lwgeom);
	}
	/* WKT then. */
	else
	{
		if ( lwgeom_from_wkt(&lwg_parser_result, str, PARSER_CHECK_ALL) == LW_FAILURE )
		{
			PG_PARSER_ERROR(lwg_parser_result);
		}
		lwgeom = lwg_parser_result.geom;
		if ( lwgeom_needs_bbox(lwgeom) )
			lwgeom_add_bbox(lwgeom);		
		ret = pglwgeom_serialize(lwgeom);
		lwgeom_parser_result_free(&lwg_parser_result);
	}
	
	PG_RETURN_POINTER(ret);

}

/*
 * LWGEOM_to_latlon(GEOMETRY, text)
 *  NOTE: Geometry must be a point.  It is assumed that the coordinates
 *        of the point are in a lat/lon projection, and they will be
 *        normalized in the output to -90-90 and -180-180.
 *
 *  The text parameter is a format string containing the format for the
 *  resulting text, similar to a date format string.  Valid tokens
 *  are "D" for degrees, "M" for minutes, "S" for seconds, and "C" for
 *  cardinal direction (NSEW).  DMS tokens may be repeated to indicate
 *  desired width and precision ("SSS.SSSS" means "  1.0023").
 *  "M", "S", and "C" are optional.  If "C" is omitted, degrees are
 *  shown with a "-" sign if south or west.  If "S" is omitted,
 *  minutes will be shown as decimal with as many digits of precision
 *  as you specify.  If "M" is omitted, degrees are shown as decimal
 *  with as many digits precision as you specify.
 *
 *  If the format string is omitted (null or 0-length) a default
 *  format will be used.
 *
 *  returns text
 */
PG_FUNCTION_INFO_V1(LWGEOM_to_latlon);
Datum LWGEOM_to_latlon(PG_FUNCTION_ARGS)
{
	/* Get the parameters */
	PG_LWGEOM *pg_lwgeom = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	text *format_text = PG_GETARG_TEXT_P(1);

	LWGEOM *lwgeom;
	char *format_str = NULL;
	char *format_str_utf8 = NULL;

	size_t str_size;

	char * formatted_str_utf8;
	char * formatted_str;
	char * formatted_text;

	/* Only supports points. */
	uchar geom_type = TYPE_GETTYPE(pg_lwgeom->type);
	if (POINTTYPE != geom_type)
	{
		lwerror("Only points are supported, you tried type %s.", lwtype_name(geom_type));
	}
	/* Convert to LWGEOM type */
	lwgeom = lwgeom_deserialize(SERIALIZED_FORM(pg_lwgeom));

	if (format_text != NULL)
	{
		str_size = VARSIZE(format_text)-VARHDRSZ; /* actual letters */
		format_str = palloc( str_size+1); /* +1 for null term */
		memcpy(format_str, VARDATA(format_text), str_size );
		format_str[str_size] = 0; /* null term */

		/* The input string supposedly will be in the database encoding, so convert to UTF-8. */
		format_str_utf8 = (char *)pg_do_encoding_conversion((uchar *)format_str, str_size, GetDatabaseEncoding(), PG_UTF8);
	}

	/* Produce the formatted string. */
	formatted_str_utf8 = lwpoint_to_latlon((LWPOINT *)lwgeom, format_str_utf8);

	/* Convert the formatted string from UTF-8 back to database encoding. */
	formatted_str = (char *)pg_do_encoding_conversion((uchar *)formatted_str_utf8, strlen(formatted_str_utf8), PG_UTF8, GetDatabaseEncoding());

	/* Convert to the postgres output string type. */
	str_size = strlen(formatted_str) + VARHDRSZ;
	formatted_text = palloc(str_size);
	memcpy(VARDATA(formatted_text), formatted_str, str_size - VARHDRSZ);
	SET_VARSIZE(formatted_text, str_size);

	/* clean up */
	if (format_str != NULL) pfree(format_str);
	/* If no encoding conversion happened, format_str_utf8 is just pointing at the same memory as format_str, so don't free it twice. */
	if ((format_str_utf8 != NULL) && (format_str_utf8 != format_str)) pfree(format_str_utf8);

	if (formatted_str != NULL) pfree(formatted_str);
	/* Again, don't free memory twice. */
	if ((formatted_str_utf8 != NULL) && (formatted_str_utf8 != formatted_str)) pfree(formatted_str_utf8);

	PG_RETURN_POINTER(formatted_text);
}

/*
 * LWGEOM_out(lwgeom) --> cstring
 * output is 'SRID=#;<wkb in hex form>'
 * ie. 'SRID=-99;0101000000000000000000F03F0000000000000040'
 * WKB is machine endian
 * if SRID=-1, the 'SRID=-1;' will probably not be present.
 */
PG_FUNCTION_INFO_V1(LWGEOM_out);
Datum LWGEOM_out(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom = (PG_LWGEOM*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	LWGEOM *lwgeom;
	char *hexwkb;
	size_t hexwkb_size;

	lwgeom = pglwgeom_deserialize(geom);
	hexwkb = (char*)lwgeom_to_wkb(lwgeom, WKB_EXTENDED | WKB_HEX , &hexwkb_size);
	lwgeom_free(lwgeom);
	
	PG_RETURN_CSTRING(hexwkb);
}

/*
 * AsHEXEWKB(geom, string)
 */
PG_FUNCTION_INFO_V1(LWGEOM_asHEXEWKB);
Datum LWGEOM_asHEXEWKB(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom = (PG_LWGEOM*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	LWGEOM *lwgeom;
	char *hexwkb;
	size_t hexwkb_size;
	uchar variant = 0;
	text *result;
	text *type;
	size_t text_size;

	/* If user specified endianness, respect it */
	if ( (PG_NARGS()>1) && (!PG_ARGISNULL(1)) )
	{
		type = PG_GETARG_TEXT_P(1);

		if  ( ! strncmp(VARDATA(type), "xdr", 3) ||
		      ! strncmp(VARDATA(type), "XDR", 3) )
		{
			variant = variant | WKB_XDR;
		}
		else
		{
			variant = variant | WKB_NDR;
		}
	}

	/* Create WKB hex string */
	lwgeom = pglwgeom_deserialize(geom);
	hexwkb = (char*)lwgeom_to_wkb(lwgeom, variant | WKB_EXTENDED | WKB_HEX , &hexwkb_size);
	lwgeom_free(lwgeom);
	
	/* Prepare the PgSQL text return type */
	text_size = hexwkb_size - 1 + VARHDRSZ;
	result = palloc(text_size);
	memcpy(VARDATA(result), hexwkb, hexwkb_size - 1);
	SET_VARSIZE(result, text_size);
	
	/* Clean up and return */
	pfree(hexwkb);
	PG_FREE_IF_COPY(geom, 0);
	PG_RETURN_TEXT_P(result);
}


/*
 * LWGEOM_to_text(lwgeom) --> text
 * output is 'SRID=#;<wkb in hex form>'
 * ie. 'SRID=-99;0101000000000000000000F03F0000000000000040'
 * WKB is machine endian
 * if SRID=-1, the 'SRID=-1;' will probably not be present.
 */
PG_FUNCTION_INFO_V1(LWGEOM_to_text);
Datum LWGEOM_to_text(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom = (PG_LWGEOM*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	LWGEOM *lwgeom;
	char *hexwkb;
	size_t hexwkb_size;
	text *result;
	size_t text_size;

	/* Generate WKB hex text */
	lwgeom = pglwgeom_deserialize(geom);
	hexwkb = (char*)lwgeom_to_wkb(lwgeom, WKB_EXTENDED | WKB_HEX , &hexwkb_size);
	lwgeom_free(lwgeom);
	
	/* Prepare the PgSQL text return type, which doesn't include a null terminator */
	text_size = hexwkb_size-1+VARHDRSZ;
	result = palloc(text_size);
	memcpy(VARDATA(result), hexwkb, hexwkb_size-1);
	SET_VARSIZE(result, text_size);
	
	/* Clean up and return */
	pfree(hexwkb);
	PG_FREE_IF_COPY(geom, 0);
	PG_RETURN_TEXT_P(result);
}

/*
 * LWGEOMFromWKB(wkb,  [SRID] )
 * NOTE: wkb is in *binary* not hex form.
 *
 * NOTE: this function parses EWKB (extended form)
 *       which also contains SRID info. 
 */
PG_FUNCTION_INFO_V1(LWGEOMFromWKB);
Datum LWGEOMFromWKB(PG_FUNCTION_ARGS)
{
	bytea *bytea_wkb = (bytea*)PG_GETARG_BYTEA_P(0);
	int32 srid = 0;
	PG_LWGEOM *geom;
	LWGEOM *lwgeom;
	uchar *wkb = (uchar*)VARDATA(bytea_wkb);
	
	lwgeom = lwgeom_from_wkb(wkb, VARSIZE(bytea_wkb)-VARHDRSZ, PARSER_CHECK_ALL);
	
	if (  ( PG_NARGS()>1) && ( ! PG_ARGISNULL(1) ))
	{
		srid = PG_GETARG_INT32(1);
		lwgeom_set_srid(lwgeom, srid);
	}

	if ( lwgeom_needs_bbox(lwgeom) )
		lwgeom_add_bbox(lwgeom);

	geom = pglwgeom_serialize(lwgeom);
	lwgeom_free(lwgeom);
	PG_FREE_IF_COPY(bytea_wkb, 0);
	PG_RETURN_POINTER(geom);
}

/*
 * WKBFromLWGEOM(lwgeom) --> wkb
 * this will have no 'SRID=#;'
 */
PG_FUNCTION_INFO_V1(WKBFromLWGEOM);
Datum WKBFromLWGEOM(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom = (PG_LWGEOM*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	LWGEOM *lwgeom;
	uchar *wkb;
	size_t wkb_size;
	uchar variant = 0;
 	bytea *result;
	text *type;

	/* If user specified endianness, respect it */
	if ( (PG_NARGS()>1) && (!PG_ARGISNULL(1)) )
	{
		type = PG_GETARG_TEXT_P(1);

		if  ( ! strncmp(VARDATA(type), "xdr", 3) ||
		      ! strncmp(VARDATA(type), "XDR", 3) )
		{
			variant = variant | WKB_XDR;
		}
		else
		{
			variant = variant | WKB_NDR;
		}
	}

	/* Create WKB hex string */
	lwgeom = pglwgeom_deserialize(geom);
	wkb = lwgeom_to_wkb(lwgeom, variant | WKB_EXTENDED , &wkb_size);
	lwgeom_free(lwgeom);
	
	/* Prepare the PgSQL text return type */
	result = palloc(wkb_size + VARHDRSZ);
	memcpy(VARDATA(result), wkb, wkb_size);
	SET_VARSIZE(result, wkb_size+VARHDRSZ);
	
	/* Clean up and return */
	pfree(wkb);
	PG_FREE_IF_COPY(geom, 0);
	PG_RETURN_BYTEA_P(result);
}


/* puts a bbox inside the geometry */
PG_FUNCTION_INFO_V1(LWGEOM_addBBOX);
Datum LWGEOM_addBBOX(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *lwgeom = (PG_LWGEOM *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *result;
	BOX2DFLOAT4	box;
	uchar	old_type;
	int		size;

	POSTGIS_DEBUG(2, "in LWGEOM_addBBOX");

	if (lwgeom_hasBBOX( lwgeom->type ) )
	{
		POSTGIS_DEBUG(3, "LWGEOM_addBBOX  -- already has bbox");

		/* easy - already has one.  Just copy! */
		result = palloc (VARSIZE(lwgeom));
		SET_VARSIZE(result, VARSIZE(lwgeom));
		memcpy(VARDATA(result), VARDATA(lwgeom), VARSIZE(lwgeom)-VARHDRSZ);
		PG_RETURN_POINTER(result);
	}

	POSTGIS_DEBUG(3, "LWGEOM_addBBOX  -- giving it a bbox");

	/* construct new one */
	if ( ! getbox2d_p(SERIALIZED_FORM(lwgeom), &box) )
	{
		/* Empty geom, no bbox to add */
		result = palloc (VARSIZE(lwgeom));
		SET_VARSIZE(result, VARSIZE(lwgeom));
		memcpy(VARDATA(result), VARDATA(lwgeom), VARSIZE(lwgeom)-VARHDRSZ);
		PG_RETURN_POINTER(result);
	}
	old_type = lwgeom->type;

	size = VARSIZE(lwgeom)+sizeof(BOX2DFLOAT4);

	result = palloc(size); /* 16 for bbox2d */
	SET_VARSIZE(result, size);

	result->type = lwgeom_makeType_full(
	                   TYPE_HASZ(old_type),
	                   TYPE_HASM(old_type),
	                   lwgeom_hasSRID(old_type), lwgeom_getType(old_type), 1);

	/* copy in bbox */
	memcpy(result->data, &box, sizeof(BOX2DFLOAT4));

	POSTGIS_DEBUGF(3, "result->type hasbbox: %d", TYPE_HASBBOX(result->type));
	POSTGIS_DEBUG(3, "LWGEOM_addBBOX  -- about to copy serialized form");

	/* everything but the type and length */
	memcpy((char *)VARDATA(result)+sizeof(BOX2DFLOAT4)+1, (char *)VARDATA(lwgeom)+1, VARSIZE(lwgeom)-VARHDRSZ-1);

	PG_RETURN_POINTER(result);
}

char
is_worth_caching_pglwgeom_bbox(const PG_LWGEOM *in)
{
#if ! POSTGIS_AUTOCACHE_BBOX
	return false;
#endif
	if ( TYPE_GETTYPE(in->type) == POINTTYPE ) return false;
	return true;
}

char
is_worth_caching_serialized_bbox(const uchar *in)
{
#if ! POSTGIS_AUTOCACHE_BBOX
	return false;
#endif
	if ( TYPE_GETTYPE((uchar)in[0]) == POINTTYPE ) return false;
	return true;
}

char
is_worth_caching_lwgeom_bbox(const LWGEOM *in)
{
#if ! POSTGIS_AUTOCACHE_BBOX
	return false;
#endif
	if ( TYPE_GETTYPE(in->type) == POINTTYPE ) return false;
	return true;
}

/* removes a bbox from a geometry */
PG_FUNCTION_INFO_V1(LWGEOM_dropBBOX);
Datum LWGEOM_dropBBOX(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *lwgeom = (PG_LWGEOM *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *result;
	uchar old_type;
	int size;

	POSTGIS_DEBUG(2, "in LWGEOM_dropBBOX");

	if (!lwgeom_hasBBOX( lwgeom->type ) )
	{
		POSTGIS_DEBUG(3, "LWGEOM_dropBBOX  -- doesnt have a bbox already");

		result = palloc (VARSIZE(lwgeom));
		SET_VARSIZE(result, VARSIZE(lwgeom));
		memcpy(VARDATA(result), VARDATA(lwgeom), VARSIZE(lwgeom)-VARHDRSZ);
		PG_RETURN_POINTER(result);
	}

	POSTGIS_DEBUG(3, "LWGEOM_dropBBOX  -- dropping the bbox");

	/* construct new one */
	old_type = lwgeom->type;

	size = VARSIZE(lwgeom)-sizeof(BOX2DFLOAT4);

	result = palloc(size); /* 16 for bbox2d */
	SET_VARSIZE(result, size);

	result->type = lwgeom_makeType_full(
	                   TYPE_HASZ(old_type),
	                   TYPE_HASM(old_type),
	                   lwgeom_hasSRID(old_type), lwgeom_getType(old_type), 0);

	/* everything but the type and length */
	memcpy((char *)VARDATA(result)+1, ((char *)(lwgeom->data))+sizeof(BOX2DFLOAT4), size-VARHDRSZ-1);

	PG_RETURN_POINTER(result);
}


/* for the wkt parser */
void elog_ERROR(const char* string)
{
	elog(ERROR, "%s", string);
}

/*
* This just does the same thing as the _in function,
* except it has to handle a 'text' input. First
* unwrap the text into a cstring, then call
* geometry_in
*/
PG_FUNCTION_INFO_V1(parse_WKT_lwgeom);
Datum parse_WKT_lwgeom(PG_FUNCTION_ARGS)
{
	text *wkt_text = PG_GETARG_TEXT_P(0);
	char *wkt;
	size_t wkt_size;
	Datum result;

	/* Unwrap the PgSQL text type into a cstring */
	wkt_size = VARSIZE(wkt_text)-VARHDRSZ; /* size of VARDATA only */
	wkt = palloc(wkt_size+1); /* +1 for null terminator */
	memcpy(wkt, VARDATA(wkt_text), wkt_size );
	wkt[wkt_size] = 0; /* null terminator */
	
	/* Now we call over to the geometry_in function */
	result = DirectFunctionCall1(LWGEOM_in, CStringGetDatum(wkt));

	/* Return null on null */
	if ( ! result ) 
		PG_RETURN_NULL();

	PG_RETURN_DATUM(result);
}


/*
 * This function must advance the StringInfo.cursor pointer
 * and leave it at the end of StringInfo.buf. If it fails
 * to do so the backend will raise an exception with message:
 * ERROR:  incorrect binary data format in bind parameter #
 *
 */
PG_FUNCTION_INFO_V1(LWGEOM_recv);
Datum LWGEOM_recv(PG_FUNCTION_ARGS)
{
	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	PG_LWGEOM *geom;
	LWGEOM *lwgeom;
	
	lwgeom = lwgeom_from_wkb((uchar*)buf->data, buf->len, PARSER_CHECK_ALL);
	
	if ( lwgeom_needs_bbox(lwgeom) )
		lwgeom_add_bbox(lwgeom);

	/* Set cursor to the end of buffer (so the backend is happy) */
	buf->cursor = buf->len;

	geom = pglwgeom_serialize(lwgeom);
	lwgeom_free(lwgeom);
	PG_RETURN_POINTER(geom);
}



PG_FUNCTION_INFO_V1(LWGEOM_send);
Datum LWGEOM_send(PG_FUNCTION_ARGS)
{
	POSTGIS_DEBUG(2, "LWGEOM_send called");

	PG_RETURN_POINTER(
	  DatumGetPointer(
	    DirectFunctionCall1(
	      WKBFromLWGEOM, 
	      PG_GETARG_DATUM(0)
	    )));
}

PG_FUNCTION_INFO_V1(LWGEOM_to_bytea);
Datum LWGEOM_to_bytea(PG_FUNCTION_ARGS)
{
	POSTGIS_DEBUG(2, "LWGEOM_to_bytea called");

	PG_RETURN_POINTER(
	  DatumGetPointer(
	    DirectFunctionCall1(
	      WKBFromLWGEOM, 
	      PG_GETARG_DATUM(0)
	    )));
}

PG_FUNCTION_INFO_V1(LWGEOM_from_bytea);
Datum LWGEOM_from_bytea(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *result;

	POSTGIS_DEBUG(2, "LWGEOM_from_bytea start");

	result = (PG_LWGEOM *)DatumGetPointer(DirectFunctionCall1(
	                                          LWGEOMFromWKB, PG_GETARG_DATUM(0)));

	PG_RETURN_POINTER(result);
}

