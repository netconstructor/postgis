/*
 * BinReader.java
 * 
 * (C) 14.01.2005 Markus Schaber, Logi-Track ag, CH 8001 Zuerich
 * 
 * This File is Licensed under the GNU GPL.
 * 
 * $Id$
 */
package org.postgis.binary;

import org.postgis.Geometry;
import org.postgis.GeometryCollection;
import org.postgis.LineString;
import org.postgis.LinearRing;
import org.postgis.MultiLineString;
import org.postgis.MultiPoint;
import org.postgis.MultiPolygon;
import org.postgis.Point;
import org.postgis.Polygon;
import org.postgis.binary.ByteGetter.BinaryByteGetter;
import org.postgis.binary.ByteGetter.StringByteGetter;

/**
 * Parse binary representation of geometries. Currently, only text rep (hexed)
 * implementation is tested.
 * 
 * It should be easy to add char[] and CharSequence ByteGetter instances,
 * although the latter one is not compatible with older jdks.
 * 
 * I did not implement real unsigned 32-bit integers or emulate them with long,
 * as both java Arrays and Strings currently can have only 2^31-1 elements
 * (bytes), so we cannot even get or build Geometries with more than approx.
 * 2^28 coordinates (8 bytes each).
 * 
 * @author markus.schaber@logi-track.com
 *  
 */
public class BinaryParser {

    /**
     * Get the appropriate ValueGetter for my endianness
     * 
     * @param bytes The appropriate Byte Getter
     * 
     * @return the ValueGetter
     */
    public static ValueGetter valueGetterForEndian(ByteGetter bytes) {
        if (bytes.get(0) == ValueGetter.XDR.NUMBER) { // XDR
            return new ValueGetter.XDR(bytes);
        } else if (bytes.get(0) == ValueGetter.NDR.NUMBER) {
            return new ValueGetter.NDR(bytes);
        } else {
            throw new IllegalArgumentException("Unknown Endian type:" + bytes.get(0));
        }
    }

    /**
     * Parse a hex encoded geometry
     * 
     * Is synchronized to protect offset counter. (Unfortunately, Java does not
     * have neither call by reference nor multiple return values.)
     */
    public synchronized Geometry parse(String value) {
        StringByteGetter bytes = new ByteGetter.StringByteGetter(value);
        return parseGeometry(valueGetterForEndian(bytes));
    }

    /**
     * Parse a binary encoded geometry.
     * 
     * Is synchronized to protect offset counter. (Unfortunately, Java does not
     * have neither call by reference nor multiple return values.)
     */
    public synchronized Geometry parse(byte[] value) {
        BinaryByteGetter bytes = new ByteGetter.BinaryByteGetter(value);
        return parseGeometry(valueGetterForEndian(bytes));
    }

    /** Parse a geometry starting at offset. */
    protected Geometry parseGeometry(ValueGetter data) {
        byte endian = data.getByte(); //skip and test endian flag
        if (endian != data.endian) {
            throw new IllegalArgumentException("Endian inconsistency!");
        }
        int typeword = data.getInt();

        int realtype = typeword & 0x1FFFFFFF; //cut off high flag bits

        boolean haveZ = (typeword & 0x80000000) != 0;
        boolean haveM = (typeword & 0x40000000) != 0;
        boolean haveS = (typeword & 0x20000000) != 0;

        int srid = -1;

        if (haveS) {
            srid = data.getInt();
        }
        Geometry result1;
        switch (realtype) {
        case Geometry.POINT :
            result1 = parsePoint(data, haveZ, haveM);
            break;
        case Geometry.LINESTRING :
            result1 = parseLineString(data, haveZ, haveM);
            break;
        case Geometry.POLYGON :
            result1 = parsePolygon(data, haveZ, haveM);
            break;
        case Geometry.MULTIPOINT :
            result1 = parseMultiPoint(data);
            break;
        case Geometry.MULTILINESTRING :
            result1 = parseMultiLineString(data);
            break;
        case Geometry.MULTIPOLYGON :
            result1 = parseMultiPolygon(data);
            break;
        case Geometry.GEOMETRYCOLLECTION :
            result1 = parseCollection(data);
            break;
        default :
            throw new IllegalArgumentException("Unknown Geometry Type!");
        }

        Geometry result = result1;

        if (haveS) {
            result.setSrid(srid);
        }
        return result;
    }

    private Point parsePoint(ValueGetter data, boolean haveZ, boolean haveM) {
        double X = data.getDouble();
        double Y = data.getDouble();
        Point result;
        if (haveZ) {
            double Z = data.getDouble();
            result = new Point(X, Y, Z);
        } else {
            result = new Point(X, Y);
        }

        if (haveM) {
            result.setM(data.getDouble());
        }

        return result;
    }

    /** Parse an Array of "full" Geometries */
    private void parseGeometryArray(ValueGetter data, Geometry[] container) {
        for (int i = 0; i < container.length; i++) {
            container[i] = parseGeometry(data);
        }
    }

    /**
     * Parse an Array of "slim" Points (without endianness and type, part of
     * LinearRing and Linestring, but not MultiPoint!
     * 
     * @param haveZ
     * @param haveM
     */
    private Point[] parsePointArray(ValueGetter data, boolean haveZ, boolean haveM) {
        int count = data.getInt();
        Point[] result = new Point[count];
        for (int i = 0; i < count; i++) {
            result[i] = parsePoint(data, haveZ, haveM);
        }
        return result;
    }

    private MultiPoint parseMultiPoint(ValueGetter data) {
        Point[] points = new Point[data.getInt()];
        parseGeometryArray(data, points);
        return new MultiPoint(points);
    }

    private LineString parseLineString(ValueGetter data, boolean haveZ, boolean haveM) {
        Point[] points = parsePointArray(data, haveZ, haveM);
        return new LineString(points);
    }

    private LinearRing parseLinearRing(ValueGetter data, boolean haveZ, boolean haveM) {
        Point[] points = parsePointArray(data, haveZ, haveM);
        return new LinearRing(points);
    }

    private Polygon parsePolygon(ValueGetter data, boolean haveZ, boolean haveM) {
        int count = data.getInt();
        LinearRing[] rings = new LinearRing[count];
        for (int i = 0; i < count; i++) {
            rings[i] = parseLinearRing(data, haveZ, haveM);
        }
        return new Polygon(rings);
    }

    private MultiLineString parseMultiLineString(ValueGetter data) {
        int count = data.getInt();
        LineString[] strings = new LineString[count];
        parseGeometryArray(data, strings);
        return new MultiLineString(strings);
    }

    private MultiPolygon parseMultiPolygon(ValueGetter data) {
        int count = data.getInt();
        Polygon[] polys = new Polygon[count];
        parseGeometryArray(data, polys);
        return new MultiPolygon(polys);
    }

    private GeometryCollection parseCollection(ValueGetter data) {
        int count = data.getInt();
        Geometry[] geoms = new Geometry[count];
        parseGeometryArray(data, geoms);
        return new GeometryCollection(geoms);
    }
}