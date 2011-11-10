#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h> /* for fabs */
#include <float.h> /* for FLT_EPSILON and DBL_EPSILON */

#include "rt_api.h"
#include "check.h"

static rt_band addBand(rt_raster raster, rt_pixtype pixtype, int hasnodata, double nodataval);
static void deepRelease(rt_raster raster);
static void testBand1BB(rt_band band);
static rt_raster fillRasterToPolygonize(int hasnodata, double nodatavalue);

static rt_band
addBand(rt_raster raster, rt_pixtype pixtype, int hasnodata, double nodataval)
{
    void* mem;
    int32_t bandNum;
    size_t datasize;
    uint16_t width = rt_raster_get_width(raster);
    uint16_t height = rt_raster_get_height(raster);

    datasize = rt_pixtype_size(pixtype)*width*height;
    mem = malloc(datasize);

    rt_band band = rt_band_new_inline(width, height,
                                      pixtype, hasnodata, nodataval, mem);
    assert(band);
    bandNum = rt_raster_add_band(raster, band, 100);
    assert(bandNum>=0);
    return band;
}

static void
deepRelease(rt_raster raster)
{
    uint16_t i, nbands=rt_raster_get_num_bands(raster);
    for (i=0; i<nbands; ++i)
    {
        rt_band band = rt_raster_get_band(raster, i);
        void* mem = rt_band_get_data(band);
        if ( mem ) free(mem);
        rt_band_destroy(band);
    }
    rt_raster_destroy(raster);
}


static rt_raster
fillRasterToPolygonize(int hasnodata, double nodatavalue)
{
    /* Create raster */

    /* First test raster */
    /*
    uint16_t width = 2;
    uint16_t height = 2;
    */

    /* Second test raster */
    uint16_t width = 9;
    uint16_t height = 9;

    /* Third test raster */
    /*
    uint16_t width = 5;
    uint16_t height = 5;
    */

    rt_raster raster = rt_raster_new(width, height);

    /* Fill raster. Option 1: simple raster */
    /*
    rt_band band = addBand(raster, PT_32BSI, 0, 0);

    rt_band_set_pixel(band, 0, 0, 1);
    rt_band_set_pixel(band, 0, 1, 1);
    rt_band_set_pixel(band, 1, 0, 1);
    rt_band_set_pixel(band, 1, 1, 1);
    */


    /* Fill raster. Option 2: 9x9, 1 band */
    rt_band band = addBand(raster, PT_32BF, hasnodata, nodatavalue);

    {
        int x, y;
        for (x = 0; x < rt_band_get_width(band); ++x)
            for (y = 0; y < rt_band_get_height(band); ++y)
                rt_band_set_pixel(band, x, y, 0.0);
    }
    
    /**/
    rt_band_set_pixel(band, 3, 1, 1.8);
    rt_band_set_pixel(band, 4, 1, 1.8);
    rt_band_set_pixel(band, 5, 1, 2.8);
    rt_band_set_pixel(band, 2, 2, 1.8);
    rt_band_set_pixel(band, 3, 2, 1.8);
    rt_band_set_pixel(band, 4, 2, 1.8);
    rt_band_set_pixel(band, 5, 2, 2.8);
    rt_band_set_pixel(band, 6, 2, 2.8);
    rt_band_set_pixel(band, 1, 3, 1.8);
    rt_band_set_pixel(band, 2, 3, 1.8);
    rt_band_set_pixel(band, 6, 3, 2.8);
    rt_band_set_pixel(band, 7, 3, 2.8);
    rt_band_set_pixel(band, 1, 4, 1.8);
    rt_band_set_pixel(band, 2, 4, 1.8);
    rt_band_set_pixel(band, 6, 4, 2.8);
    rt_band_set_pixel(band, 7, 4, 2.8);
    rt_band_set_pixel(band, 1, 5, 1.8);
    rt_band_set_pixel(band, 2, 5, 1.8);
    rt_band_set_pixel(band, 6, 5, 2.8);
    rt_band_set_pixel(band, 7, 5, 2.8);
    rt_band_set_pixel(band, 2, 6, 1.8);
    rt_band_set_pixel(band, 3, 6, 1.8);
    rt_band_set_pixel(band, 4, 6, 1.8);
    rt_band_set_pixel(band, 5, 6, 2.8);
    rt_band_set_pixel(band, 6, 6, 2.8);
    rt_band_set_pixel(band, 3, 7, 1.8);
    rt_band_set_pixel(band, 4, 7, 1.8);
    rt_band_set_pixel(band, 5, 7, 2.8);



    /* Fill raster. Option 3: 5x5, 1 band */
    /*
    rt_band band = addBand(raster, PT_8BUI, 1, 255);

    rt_band_set_pixel(band, 0, 0, 253);
    rt_band_set_pixel(band, 1, 0, 254);
    rt_band_set_pixel(band, 2, 0, 253);
    rt_band_set_pixel(band, 3, 0, 254);
    rt_band_set_pixel(band, 4, 0, 254);
    rt_band_set_pixel(band, 0, 1, 253);
    rt_band_set_pixel(band, 1, 1, 254);
    rt_band_set_pixel(band, 2, 1, 254);
    rt_band_set_pixel(band, 3, 1, 253);
    rt_band_set_pixel(band, 4, 1, 249);
    rt_band_set_pixel(band, 0, 2, 250);
    rt_band_set_pixel(band, 1, 2, 254);
    rt_band_set_pixel(band, 2, 2, 254);
    rt_band_set_pixel(band, 3, 2, 252);
    rt_band_set_pixel(band, 4, 2, 249);
    rt_band_set_pixel(band, 0, 3, 251);
    rt_band_set_pixel(band, 1, 3, 253);
    rt_band_set_pixel(band, 2, 3, 254);
    rt_band_set_pixel(band, 3, 3, 254);
    rt_band_set_pixel(band, 4, 3, 253);
    rt_band_set_pixel(band, 0, 4, 252);
    rt_band_set_pixel(band, 1, 4, 250);
    rt_band_set_pixel(band, 2, 4, 254);
    rt_band_set_pixel(band, 3, 4, 254);
    rt_band_set_pixel(band, 4, 4, 254);
    */

    return raster;
}

static void testBand1BB(rt_band band)
{
    int failure;
    double val = 0;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    //printf("1BB nodata is %g\n", rt_band_get_nodata(band));
    CHECK_EQUALS(rt_band_get_nodata(band), 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    //printf("1BB nodata is %g\n", rt_band_get_nodata(band));
    CHECK_EQUALS(rt_band_get_nodata(band), 0);

    failure = rt_band_set_nodata(band, 2);
    CHECK(failure); /* out of range value */

    failure = rt_band_set_nodata(band, 3);
    CHECK(failure); /* out of range value */

    failure = rt_band_set_pixel(band, 0, 0, 2);
    CHECK(failure); /* out of range value */

    failure = rt_band_set_pixel(band, 0, 0, 3);
    CHECK(failure); /* out of range value */

    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, 0);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 0);

            }
        }
    }

}

static void testBand2BUI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);
    CHECK(!failure);

    failure = rt_band_set_nodata(band, 0);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);
    CHECK(!failure);

    failure = rt_band_set_nodata(band, 2);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 2);
    CHECK(!failure);

    failure = rt_band_set_nodata(band, 3);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 3);
    CHECK(!failure);

    failure = rt_band_set_nodata(band, 4); /* invalid: out of range */
    CHECK(failure);

    failure = rt_band_set_nodata(band, 5); /* invalid: out of range */
    CHECK(failure);

    failure = rt_band_set_pixel(band, 0, 0, 4); /* out of range */
    CHECK(failure);

    failure = rt_band_set_pixel(band, 0, 0, 5); /* out of range */
    CHECK(failure);


    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, 2);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 2);

                failure = rt_band_set_pixel(band, x, y, 3);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 3);
            }
        }
    }

}

static void testBand4BUI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    val = rt_band_get_nodata(band);
    CHECK(!failure);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 2);
    val = rt_band_get_nodata(band);
    CHECK(!failure);
    CHECK_EQUALS(val, 2);

    failure = rt_band_set_nodata(band, 4);
    val = rt_band_get_nodata(band);
    CHECK(!failure);
    CHECK_EQUALS(val, 4);

    failure = rt_band_set_nodata(band, 8);
    val = rt_band_get_nodata(band);
    CHECK(!failure);
    CHECK_EQUALS(val, 8);

    failure = rt_band_set_nodata(band, 15);
    val = rt_band_get_nodata(band);
    CHECK(!failure);
    CHECK_EQUALS(val, 15);

    failure = rt_band_set_nodata(band, 16);  /* out of value range */
    CHECK(failure);

    failure = rt_band_set_nodata(band, 17);  /* out of value range */
    CHECK(failure);

    failure = rt_band_set_pixel(band, 0, 0, 35); /* out of value range */
    CHECK(failure);


    {
        int x, y;

        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, 3);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 3);

                failure = rt_band_set_pixel(band, x, y, 7);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 7);

                failure = rt_band_set_pixel(band, x, y, 15);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 15);
            }
        }
    }

}

static void testBand8BUI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 2);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 2);

    failure = rt_band_set_nodata(band, 4);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 4);

    failure = rt_band_set_nodata(band, 8);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 8);

    failure = rt_band_set_nodata(band, 15);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 15);

    failure = rt_band_set_nodata(band, 31);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 31);

    failure = rt_band_set_nodata(band, 255);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 255);

    failure = rt_band_set_nodata(band, 256); /* out of value range */
    CHECK(failure);

    failure = rt_band_set_pixel(band, 0, 0, 256); /* out of value range */
    CHECK(failure);

    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 31);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 31);

                failure = rt_band_set_pixel(band, x, y, 255);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 255);

                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);
            }
        }
    }
}

static void testBand8BSI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 2);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 2);

    failure = rt_band_set_nodata(band, 4);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 4);

    failure = rt_band_set_nodata(band, 8);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 8);

    failure = rt_band_set_nodata(band, 15);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 15);

    failure = rt_band_set_nodata(band, 31);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 31);

    failure = rt_band_set_nodata(band, -127);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, -127);

    failure = rt_band_set_nodata(band, 127);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 127);

    /* out of range (-127..127) */
    failure = rt_band_set_nodata(band, -129);
    CHECK(failure);

    /* out of range (-127..127) */
    failure = rt_band_set_nodata(band, 129);
    CHECK(failure);

    /* out of range (-127..127) */
    failure = rt_band_set_pixel(band, 0, 0, -129);
    CHECK(failure);

    /* out of range (-127..127) */
    failure = rt_band_set_pixel(band, 0, 0, 129);
    CHECK(failure);


    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 31);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 31);

                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, -127);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, -127);

                failure = rt_band_set_pixel(band, x, y, 127);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 127);

            }
        }
    }
}

static void testBand16BUI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 31);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 31);

    failure = rt_band_set_nodata(band, 255);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 255);

    failure = rt_band_set_nodata(band, 65535);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    //printf("set 65535 on %s band gets %g back\n", pixtypeName, val);
    CHECK_EQUALS(val, 65535);

    failure = rt_band_set_nodata(band, 65536); /* out of range */
    CHECK(failure);

    /* out of value range */
    failure = rt_band_set_pixel(band, 0, 0, 65536);
    CHECK(failure);

    /* out of dimensions range */
    failure = rt_band_set_pixel(band, rt_band_get_width(band), 0, 0);
    CHECK(failure);

    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 255);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 255);

                failure = rt_band_set_pixel(band, x, y, 65535);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 65535);
            }
        }
    }
}

static void testBand16BSI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 31);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 31);

    failure = rt_band_set_nodata(band, 255);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 255);

    failure = rt_band_set_nodata(band, -32767);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    //printf("set 65535 on %s band gets %g back\n", pixtypeName, val);
    CHECK_EQUALS(val, -32767);

    failure = rt_band_set_nodata(band, 32767);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    //printf("set 65535 on %s band gets %g back\n", pixtypeName, val);
    CHECK_EQUALS(val, 32767);

    /* out of range (-32767..32767) */
    failure = rt_band_set_nodata(band, -32769);
    CHECK(failure);

    /* out of range (-32767..32767) */
    failure = rt_band_set_nodata(band, 32769);
    CHECK(failure);

    /* out of range (-32767..32767) */
    failure = rt_band_set_pixel(band, 0, 0, -32769);
    CHECK(failure);

    /* out of range (-32767..32767) */
    failure = rt_band_set_pixel(band, 0, 0, 32769);
    CHECK(failure);

    /* out of dimensions range */
    failure = rt_band_set_pixel(band, rt_band_get_width(band), 0, 0);
    CHECK(failure);

    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 255);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 255);

                failure = rt_band_set_pixel(band, x, y, -32767);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, -32767);

                failure = rt_band_set_pixel(band, x, y, 32767);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 32767);
            }
        }
    }
}

static void testBand32BUI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 65535);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 65535);

    failure = rt_band_set_nodata(band, 4294967295UL);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 4294967295UL);

    /* out of range */
    failure = rt_band_set_nodata(band, 4294967296ULL);
    CHECK(failure);

    /* out of value range */
    failure = rt_band_set_pixel(band, 0, 0, 4294967296ULL);
    CHECK(failure);

    /* out of dimensions range */
    failure = rt_band_set_pixel(band, rt_band_get_width(band),
                                0, 4294967296ULL);
    CHECK(failure);

    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, 0);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 0);

                failure = rt_band_set_pixel(band, x, y, 65535);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 65535);

                failure = rt_band_set_pixel(band, x, y, 4294967295UL);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 4294967295UL);
            }
        }
    }
}

static void testBand32BSI(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 65535);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 65535);

    failure = rt_band_set_nodata(band, 2147483647);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    /*printf("32BSI pix is %ld\n", (long int)val);*/
    CHECK_EQUALS(val, 2147483647);

    failure = rt_band_set_nodata(band, 2147483648UL);
    /* out of range */
    CHECK(failure);

    /* out of value range */
    failure = rt_band_set_pixel(band, 0, 0, 2147483648UL);
    CHECK(failure);

    /* out of dimensions range */
    failure = rt_band_set_pixel(band, rt_band_get_width(band), 0, 0);
    CHECK(failure);


    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, 0);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 0);

                failure = rt_band_set_pixel(band, x, y, 65535);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 65535);

                failure = rt_band_set_pixel(band, x, y, 2147483647);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 2147483647);
            }
        }
    }
}

static void testBand32BF(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 65535.5);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    //printf("set 65535.56 on %s band gets %g back\n", pixtypeName, val);
    CHECK_EQUALS_DOUBLE(val, 65535.5);

    failure = rt_band_set_nodata(band, 0.006);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS_DOUBLE(val, 0.0060000000521540); /* XXX: Alternatively, use CHECK_EQUALS_DOUBLE_EX */

    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, 0);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 0);

                failure = rt_band_set_pixel(band, x, y, 65535.5);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS_DOUBLE(val, 65535.5);

                failure = rt_band_set_pixel(band, x, y, 0.006);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS_DOUBLE(val, 0.0060000000521540);

            }
        }
    }
}

static void testBand64BF(rt_band band)
{
    double val;
    int failure;

    failure = rt_band_set_nodata(band, 1);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 1);

    failure = rt_band_set_nodata(band, 0);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0);

    failure = rt_band_set_nodata(band, 65535.56);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 65535.56);

    failure = rt_band_set_nodata(band, 0.006);
    CHECK(!failure);
    val = rt_band_get_nodata(band);
    CHECK_EQUALS(val, 0.006);

    {
        int x, y;
        for (x=0; x<rt_band_get_width(band); ++x)
        {
            for (y=0; y<rt_band_get_height(band); ++y)
            {
                failure = rt_band_set_pixel(band, x, y, 1);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 1);

                failure = rt_band_set_pixel(band, x, y, 0);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 0);

                failure = rt_band_set_pixel(band, x, y, 65535.56);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 65535.56);

                failure = rt_band_set_pixel(band, x, y, 0.006);
                CHECK(!failure);
                failure = rt_band_get_pixel(band, x, y, &val);
                CHECK(!failure);
                CHECK_EQUALS(val, 0.006);

            }
        }
    }
}

static void testBandHasNoData(rt_band band)
{
    int flag;

    flag = rt_band_get_hasnodata_flag(band);
    CHECK_EQUALS(flag, 1);

    rt_band_set_hasnodata_flag(band, 0);
    flag = rt_band_get_hasnodata_flag(band);
    CHECK_EQUALS(flag, 0);

    rt_band_set_hasnodata_flag(band, 10);
    flag = rt_band_get_hasnodata_flag(band);
    CHECK_EQUALS(flag, 1);

    rt_band_set_hasnodata_flag(band, -10);
    flag = rt_band_get_hasnodata_flag(band);
    CHECK_EQUALS(flag, 1);
}

static void testRasterFromBand(rt_raster raster) {
	uint32_t bandNums[] = {1,3};
	int lenBandNums = 2;
	rt_raster rast;

	rast = rt_raster_from_band(raster, bandNums, lenBandNums);
	assert(rast);

	CHECK(rast);
	CHECK(!rt_raster_is_empty(rast));
	CHECK(!rt_raster_has_no_band(rast, 1));

	rt_raster_destroy(rast);
}

static void testBandStats() {
	rt_bandstats stats = NULL;
	rt_histogram histogram = NULL;
	double bin_width[] = {100};
	double quantiles[] = {0.1, 0.3, 0.5, 0.7, 0.9};
	double quantiles2[] = {0.66666667};
	rt_quantile quantile = NULL;
	uint32_t count = 0;

	rt_raster raster;
	rt_band band;
	uint32_t x;
	uint32_t xmax = 100;
	uint32_t y;
	uint32_t ymax = 100;
	uint32_t max_run;
	double nodata;
	int rtn;

	uint32_t values[] = {0, 91, 55, 86, 76, 41, 36, 97, 25, 63, 68, 2, 78, 15, 82, 47};
	struct quantile_llist *qlls = NULL;
	uint32_t qlls_count;

	raster = rt_raster_new(xmax, ymax);
	assert(raster);
	band = addBand(raster, PT_32BUI, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			rtn = rt_band_set_pixel(band, x, y, x + y);
			CHECK((rtn != -1));
		}
	}

	nodata = rt_band_get_nodata(band);
	CHECK_EQUALS(nodata, 0);

	stats = (rt_bandstats) rt_band_get_summary_stats(band, 1, 0, 1, NULL, NULL, NULL);
	CHECK(stats);
	CHECK_EQUALS(stats->min, 1);
	CHECK_EQUALS(stats->max, 198);

	quantile = (rt_quantile) rt_band_get_quantiles(stats, NULL, 0, &count);
	CHECK(quantile);
	rtdealloc(quantile);

	histogram = (rt_histogram) rt_band_get_histogram(stats, 0, NULL, 0, 0, 0, 0, &count);
	CHECK(histogram);
	rtdealloc(histogram);

	histogram = (rt_histogram) rt_band_get_histogram(stats, 0, NULL, 0, 1, 0, 0, &count);
	CHECK(histogram);
	rtdealloc(histogram);

	histogram = (rt_histogram) rt_band_get_histogram(stats, 0, bin_width, 1, 0, 0, 0, &count);
	CHECK(histogram);
	rtdealloc(histogram);

	rtdealloc(stats->values);
	rtdealloc(stats);

	stats = (rt_bandstats) rt_band_get_summary_stats(band, 1, 0.1, 1, NULL, NULL, NULL);
	CHECK(stats);

	quantile = (rt_quantile) rt_band_get_quantiles(stats, NULL, 0, &count);
	CHECK(quantile);
	rtdealloc(quantile);

	quantile = (rt_quantile) rt_band_get_quantiles(stats, quantiles, 5, &count);
	CHECK(quantile);
	CHECK((count == 5));
	rtdealloc(quantile);

	histogram = (rt_histogram) rt_band_get_histogram(stats, 0, NULL, 0, 0, 0, 0, &count);
	CHECK(histogram);
	rtdealloc(histogram);

	rtdealloc(stats->values);
	rtdealloc(stats);

	stats = (rt_bandstats) rt_band_get_summary_stats(band, 1, 0.15, 0, NULL, NULL, NULL);
	CHECK(stats);
	rtdealloc(stats);

	stats = (rt_bandstats) rt_band_get_summary_stats(band, 1, 0.2, 0, NULL, NULL, NULL);
	CHECK(stats);
	rtdealloc(stats);

	stats = (rt_bandstats) rt_band_get_summary_stats(band, 1, 0.25, 0, NULL, NULL, NULL);
	CHECK(stats);
	rtdealloc(stats);

	stats = (rt_bandstats) rt_band_get_summary_stats(band, 0, 0, 1, NULL, NULL, NULL);
	CHECK(stats);
	CHECK_EQUALS(stats->min, 0);
	CHECK_EQUALS(stats->max, 198);

	quantile = (rt_quantile) rt_band_get_quantiles(stats, NULL, 0, &count);
	CHECK(quantile);
	rtdealloc(quantile);

	rtdealloc(stats->values);
	rtdealloc(stats);

	stats = (rt_bandstats) rt_band_get_summary_stats(band, 0, 0.1, 1, NULL, NULL, NULL);
	CHECK(stats);

	quantile = (rt_quantile) rt_band_get_quantiles(stats, NULL, 0, &count);
	CHECK(quantile);
	rtdealloc(quantile);

	rtdealloc(stats->values);
	rtdealloc(stats);

	deepRelease(raster);

	xmax = 4;
	ymax = 4;
	raster = rt_raster_new(4, 4);
	assert(raster);
	band = addBand(raster, PT_8BUI, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			rtn = rt_band_set_pixel(band, x, y, values[(x * ymax) + y]);
			CHECK((rtn != -1));
		}
	}

	nodata = rt_band_get_nodata(band);
	CHECK_EQUALS(nodata, 0);

	quantile = (rt_quantile) rt_band_get_quantiles_stream(
		band, 1, 1, 15,
		&qlls, &qlls_count,
		quantiles2, 1,
		&count);
	CHECK(quantile);
	CHECK(count);
	CHECK((qlls_count > 0));
	CHECK(FLT_EQ(quantile[0].value, 78));
	rtdealloc(quantile);
	quantile_llist_destroy(&qlls, qlls_count);
	qlls = NULL;
	qlls_count = 0;

	deepRelease(raster);

	xmax = 100;
	ymax = 100;
	raster = rt_raster_new(xmax, ymax);
	assert(raster);
	band = addBand(raster, PT_64BF, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			rtn = rt_band_set_pixel(band, x, y, (((double) x * y) + (x + y) + (x + y * x)) / (x + y + 1));
			CHECK((rtn != -1));
		}
	}

	nodata = rt_band_get_nodata(band);
	CHECK_EQUALS(nodata, 0);

	max_run = 5;
	for (x = 0; x < max_run; x++) {
		quantile = (rt_quantile) rt_band_get_quantiles_stream(
			band, 1, 1, xmax * ymax * max_run,
			&qlls, &qlls_count,
			quantiles2, 1,
			&count);
		CHECK(quantile);
		CHECK(count);
		CHECK((qlls_count > 0));
		rtdealloc(quantile);
	}

	quantile_llist_destroy(&qlls, qlls_count);
	qlls = NULL;
	qlls_count = 0;

	deepRelease(raster);
}

static void testRasterReplaceBand() {
	rt_raster raster;
	rt_band band;
	rt_band rband;
	void* mem;
	size_t datasize;
	uint16_t width;
	uint16_t height;
	double nodata;

	raster = rt_raster_new(10, 10);
	assert(raster); /* or we're out of virtual memory */
	band = addBand(raster, PT_8BUI, 0, 0);
	CHECK(band);
	band = addBand(raster, PT_8BUI, 1, 255);
	CHECK(band);

	width = rt_raster_get_width(raster);
	height = rt_raster_get_height(raster);

	datasize = rt_pixtype_size(PT_8BUI) * width * height;
	mem = malloc(datasize);
	band = rt_band_new_inline(width, height, PT_8BUI, 1, 1, mem);
	assert(band);

	rband = rt_raster_replace_band(raster, band, 0);
	CHECK(rband);
	nodata = rt_band_get_nodata(rt_raster_get_band(raster, 0));
	CHECK((nodata == 1));

	deepRelease(raster);

	mem = rt_band_get_data(rband);
	rt_band_destroy(rband);
	if (mem) rtdealloc(mem);
}

static void testBandReclass() {
	rt_reclassexpr *exprset;

	rt_raster raster;
	rt_band band;
	uint16_t x;
	uint16_t y;
	double nodata;
	int cnt = 2;
	int i = 0;
	int rtn;
	rt_band newband;
	double val;
	void *mem = NULL;

	raster = rt_raster_new(100, 10);
	assert(raster); /* or we're out of virtual memory */
	band = addBand(raster, PT_16BUI, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	for (x = 0; x < 100; x++) {
		for (y = 0; y < 10; y++) {
			rtn = rt_band_set_pixel(band, x, y, x * y + (x + y));
			CHECK((rtn != -1));
		}
	}

	nodata = rt_band_get_nodata(band);
	CHECK_EQUALS(nodata, 0);

	exprset = malloc(cnt * sizeof(rt_reclassexpr));
	assert(exprset);

	for (i = 0; i < cnt; i++) {
		exprset[i] = malloc(sizeof(struct rt_reclassexpr_t));
		assert(exprset[i]);

		if (i == 0) {
			/* nodata */
			exprset[i]->src.min = 0;
			exprset[i]->src.inc_min = 0;
			exprset[i]->src.exc_min = 0;

			exprset[i]->src.max = 0;
			exprset[i]->src.inc_max = 0;
			exprset[i]->src.exc_max = 0;

			exprset[i]->dst.min = 0;
			exprset[i]->dst.max = 0;
		}
		else {
			/* range */
			exprset[i]->src.min = 0;
			exprset[i]->src.inc_min = 0;
			exprset[i]->src.exc_min = 0;

			exprset[i]->src.max = 1000;
			exprset[i]->src.inc_max = 1;
			exprset[i]->src.exc_max = 0;

			exprset[i]->dst.min = 1;
			exprset[i]->dst.max = 255;
		}
	}

	newband = rt_band_reclass(band, PT_8BUI, 0, 0, exprset, cnt);
	CHECK(newband);

	rtn = rt_band_get_pixel(newband, 0, 0, &val);
	CHECK((rtn != -1));
	CHECK_EQUALS(val, 0);

	rtn = rt_band_get_pixel(newband, 49, 5, &val);
	CHECK((rtn != -1));
	CHECK_EQUALS(val, 77);

	rtn = rt_band_get_pixel(newband, 99, 9, &val);
	CHECK((rtn != -1));
	CHECK_EQUALS(val, 255);

	for (i = cnt - 1; i >= 0; i--) free(exprset[i]);
	free(exprset);
	deepRelease(raster);

	mem = rt_band_get_data(newband);
	if (mem) free(mem);
	rt_band_destroy(newband);
}

static void testGDALDrivers() {
	int i;
	uint32_t size;
	rt_gdaldriver drv;

	drv = (rt_gdaldriver) rt_raster_gdal_drivers(&size);
	/*printf("size: %d\n", size);*/
	CHECK(size);

	for (i = 0; i < size; i++) {
		/*printf("gdal_driver: %s\n", drv[i].short_name);*/
		CHECK(drv[i].short_name);
		rtdealloc(drv[i].short_name);
		rtdealloc(drv[i].long_name);
		rtdealloc(drv[i].create_options);
	}

	rtdealloc(drv);
}

static void testRasterToGDAL() {
	rt_raster raster;
	rt_band band;
	uint32_t x;
	uint32_t xmax = 100;
	uint32_t y;
	uint32_t ymax = 100;
	int rtn = 0;
	char srs[] = "PROJCS[\"unnamed\",GEOGCS[\"unnamed ellipse\",DATUM[\"unknown\",SPHEROID[\"unnamed\",6370997,0]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Lambert_Azimuthal_Equal_Area\"],PARAMETER[\"latitude_of_center\",45],PARAMETER[\"longitude_of_center\",-100],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1],AUTHORITY[\"EPSG\",\"2163\"]]";

	uint64_t gdalSize;
	uint8_t *gdal = NULL;

	raster = rt_raster_new(xmax, ymax);
	assert(raster); /* or we're out of virtual memory */
	band = addBand(raster, PT_64BF, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	rt_raster_set_offsets(raster, -500000, 600000);
	rt_raster_set_scale(raster, 1000, -1000);

	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			rtn = rt_band_set_pixel(band, x, y, (((double) x * y) + (x + y) + (x + y * x)) / (x + y + 1));
			CHECK((rtn != -1));
		}
	}

	gdal = rt_raster_to_gdal(raster, srs, "GTiff", NULL, &gdalSize);
	/*printf("gdalSize: %d\n", (int) gdalSize);*/
	CHECK(gdalSize);

	/*
	FILE *fh = NULL;
	fh = fopen("/tmp/out.tif", "w");
	fwrite(gdal, sizeof(uint8_t), gdalSize, fh);
	fclose(fh);
	*/

	if (gdal) CPLFree(gdal);

	deepRelease(raster);
}

static void testValueCount() {
	rt_valuecount vcnts = NULL;

	rt_raster raster;
	rt_band band;
	uint32_t x;
	uint32_t xmax = 100;
	uint32_t y;
	uint32_t ymax = 100;
	uint32_t rtn = 0;

	double count[] = {3, 4, 5};

	raster = rt_raster_new(xmax, ymax);
	assert(raster); /* or we're out of virtual memory */
	band = addBand(raster, PT_64BF, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			rtn = rt_band_set_pixel(band, x, y, (((double) x * y) + (x + y) + (x + y * x)) / (x + y + 1));
			CHECK((rtn != -1));
		}
	}
	vcnts = rt_band_get_value_count(band, 1, NULL, 0, 0, NULL, &rtn);
	CHECK(vcnts);
	CHECK((rtn > 0));
	rtdealloc(vcnts);

	vcnts = rt_band_get_value_count(band, 1, NULL, 0, 0.01, NULL, &rtn);
	CHECK(vcnts);
	CHECK((rtn > 0));
	rtdealloc(vcnts);

	vcnts = rt_band_get_value_count(band, 1, NULL, 0, 0.1, NULL, &rtn);
	CHECK(vcnts);
	CHECK((rtn > 0));
	rtdealloc(vcnts);

	vcnts = rt_band_get_value_count(band, 1, NULL, 0, 1, NULL, &rtn);
	CHECK(vcnts);
	CHECK((rtn > 0));
	rtdealloc(vcnts);

	vcnts = rt_band_get_value_count(band, 1, NULL, 0, 10, NULL, &rtn);
	CHECK(vcnts);
	CHECK((rtn > 0));
	rtdealloc(vcnts);

	vcnts = rt_band_get_value_count(band, 1, count, 3, 1, NULL, &rtn);
	CHECK(vcnts);
	CHECK((rtn > 0));
	rtdealloc(vcnts);

	deepRelease(raster);
}

static void testGDALToRaster() {
	rt_raster raster;
	rt_raster rast;
	rt_band band;
	uint32_t x;
	uint32_t xmax = 100;
	uint32_t y;
	uint32_t ymax = 100;
	int rtn = 0;
	double value;

	GDALDriverH gddrv = NULL;
	GDALDatasetH gdds = NULL;

	raster = rt_raster_new(xmax, ymax);
	assert(raster); /* or we're out of virtual memory */
	band = addBand(raster, PT_64BF, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			rtn = rt_band_set_pixel(band, x, y, (((double) x * y) + (x + y) + (x + y * x)) / (x + y + 1));
			CHECK((rtn != -1));
		}
	}

	gdds = rt_raster_to_gdal_mem(raster, NULL, NULL, 0, &gddrv);
	CHECK(gddrv);
	CHECK(gdds);
	CHECK((GDALGetRasterXSize(gdds) == xmax));
	CHECK((GDALGetRasterYSize(gdds) == ymax));

	rast = rt_raster_from_gdal_dataset(gdds);
	CHECK(rast);
	CHECK((rt_raster_get_num_bands(rast) == 1));

	band = rt_raster_get_band(rast, 0);
	CHECK(band);

	rtn = rt_band_get_pixel(band, 0, 3, &value);
	CHECK((rtn != -1));
	CHECK(FLT_EQ(value, 0.75));

	rtn = rt_band_get_pixel(band, 99, 0, &value);
	CHECK((rtn != -1));
	CHECK(FLT_EQ(value, 1.98));

	rtn = rt_band_get_pixel(band, 95, 4, &value);
	CHECK((rtn != -1));
	CHECK(FLT_EQ(value, 9.54));

	GDALClose(gdds);
	GDALDeregisterDriver(gddrv);
	GDALDestroyDriver(gddrv);

	deepRelease(rast);
	deepRelease(raster);
}

static void testGDALWarp() {
	rt_raster raster;
	rt_raster rast;
	rt_band band;
	uint32_t x;
	uint32_t xmax = 100;
	uint32_t y;
	uint32_t ymax = 100;
	int rtn = 0;
	double value = 0;

	char src_srs[] = "PROJCS[\"unnamed\",GEOGCS[\"unnamed ellipse\",DATUM[\"unknown\",SPHEROID[\"unnamed\",6370997,0]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Lambert_Azimuthal_Equal_Area\"],PARAMETER[\"latitude_of_center\",45],PARAMETER[\"longitude_of_center\",-100],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1],AUTHORITY[\"EPSG\",\"2163\"]]";

	char dst_srs[] = "PROJCS[\"NAD83 / California Albers\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4269\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Albers_Conic_Equal_Area\"],PARAMETER[\"standard_parallel_1\",34],PARAMETER[\"standard_parallel_2\",40.5],PARAMETER[\"latitude_of_center\",0],PARAMETER[\"longitude_of_center\",-120],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",-4000000],AUTHORITY[\"EPSG\",\"3310\"],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH]]";

	raster = rt_raster_new(xmax, ymax);
	assert(raster); /* or we're out of virtual memory */
	band = addBand(raster, PT_64BF, 0, 0);
	CHECK(band);
	rt_band_set_nodata(band, 0);

	rt_raster_set_offsets(raster, -500000, 600000);
	rt_raster_set_scale(raster, 1000, -1000);

	for (x = 0; x < xmax; x++) {
		for (y = 0; y < ymax; y++) {
			rtn = rt_band_set_pixel(band, x, y, (((double) x * y) + (x + y) + (x + y * x)) / (x + y + 1));
			CHECK((rtn != -1));
		}
	}

	rast = rt_raster_gdal_warp(
		raster,
		src_srs, dst_srs,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		GRA_NearestNeighbour, -1
	);
	CHECK(rast);
	CHECK((rt_raster_get_width(rast) == 122));
	CHECK((rt_raster_get_height(rast) == 116));
	CHECK((rt_raster_get_num_bands(rast) != 0));

	band = rt_raster_get_band(rast, 0);
	CHECK(band);

	CHECK(rt_band_get_hasnodata_flag(band));
	CHECK(FLT_EQ(rt_band_get_nodata(band), 0.));

	CHECK(rt_band_get_pixel(band, 0, 0, &value) == 0);
	CHECK(FLT_EQ(value, 0.));

	deepRelease(rast);
	deepRelease(raster);
}

static void testGDALRasterize() {
	rt_raster raster;
	char srs[] = "PROJCS[\"unnamed\",GEOGCS[\"unnamed ellipse\",DATUM[\"unknown\",SPHEROID[\"unnamed\",6370997,0]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Lambert_Azimuthal_Equal_Area\"],PARAMETER[\"latitude_of_center\",45],PARAMETER[\"longitude_of_center\",-100],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1],AUTHORITY[\"EPSG\",\"2163\"]]";
	const char wkb_hex[] = "010300000001000000050000000000000080841ec100000000600122410000000080841ec100000000804f22410000000040e81dc100000000804f22410000000040e81dc100000000600122410000000080841ec10000000060012241";
	const char *pos = wkb_hex;
	unsigned char *wkb = NULL;
	int wkb_len = 0;
	int i;
	double scale_x = 100;
	double scale_y = 100;

	rt_pixtype pixtype[] = {PT_8BUI};
	double init[] = {0};
	double value[] = {1};
	double nodata[] = {0};
	uint8_t nodata_mask[] = {1};

	/* hex to byte */
	wkb_len = (int) ceil(((double) strlen(wkb_hex)) / 2);
	wkb = (unsigned char *) malloc(sizeof(unsigned char) * wkb_len);
	for (i = 0; i < wkb_len; i++) {
		sscanf(pos, "%2hhx", &wkb[i]);
		pos += 2;
	}

	raster = rt_raster_gdal_rasterize(
		wkb,
		wkb_len, srs,
		1, pixtype,
		init, value,
		nodata, nodata_mask,
		NULL, NULL,
		&scale_x, &scale_y,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL
	);

	free(wkb);

	CHECK(raster);
	CHECK((rt_raster_get_width(raster) == 100));
	CHECK((rt_raster_get_height(raster) == 100));
	CHECK((rt_raster_get_num_bands(raster) != 0));
	CHECK((rt_raster_get_x_offset(raster) == -500000));
	CHECK((rt_raster_get_y_offset(raster) == 600000));

	deepRelease(raster);
}

static void testIntersects() {
	rt_raster rast1;
	rt_raster rast2;
	rt_band band1;
	rt_band band2;
	double nodata;
	int rtn;
	int intersects;

	/*
		rast1

		(-1, -1)
						+-+-+
						|1|1|
						+-+-+
						|1|1|
						+-+-+
								(1, 1)
	*/
	rast1 = rt_raster_new(2, 2);
	assert(rast1);
	rt_raster_set_offsets(rast1, -1, -1);

	band1 = addBand(rast1, PT_8BUI, 1, 0);
	CHECK(band1);
	rt_band_set_nodata(band1, 0);
	rtn = rt_band_set_pixel(band1, 0, 0, 1);
	rtn = rt_band_set_pixel(band1, 0, 1, 1);
	rtn = rt_band_set_pixel(band1, 1, 0, 1);
	rtn = rt_band_set_pixel(band1, 1, 1, 1);

	nodata = rt_band_get_nodata(band1);
	CHECK_EQUALS(nodata, 0);

	/*
		rast2

		(0, 0)
						+-+-+
						|1|1|
						+-+-+
						|1|1|
						+-+-+
								(2, 2)
	*/
	rast2 = rt_raster_new(2, 2);
	assert(rast2);

	band2 = addBand(rast2, PT_8BUI, 1, 0);
	CHECK(band2);
	rt_band_set_nodata(band2, 0);
	rtn = rt_band_set_pixel(band2, 0, 0, 1);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 1);

	nodata = rt_band_get_nodata(band2);
	CHECK_EQUALS(nodata, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	rtn = rt_raster_intersects(
		rast1, -1,
		rast2, -1,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(0, 0)
						+-+-+
						|0|1|
						+-+-+
						|1|1|
						+-+-+
								(2, 2)
	*/
	rtn = rt_band_set_pixel(band2, 0, 0, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(0, 0)
						+-+-+
						|1|0|
						+-+-+
						|1|1|
						+-+-+
								(2, 2)
	*/
	rtn = rt_band_set_pixel(band2, 0, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(0, 0)
						+-+-+
						|0|0|
						+-+-+
						|0|1|
						+-+-+
								(2, 2)
	*/
	rtn = rt_band_set_pixel(band2, 0, 0, 0);
	rtn = rt_band_set_pixel(band2, 1, 0, 0);
	rtn = rt_band_set_pixel(band2, 0, 1, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(0, 0)
						+-+-+
						|0|0|
						+-+-+
						|0|0|
						+-+-+
								(2, 2)
	*/
	rtn = rt_band_set_pixel(band2, 0, 0, 0);
	rtn = rt_band_set_pixel(band2, 1, 0, 0);
	rtn = rt_band_set_pixel(band2, 0, 1, 0);
	rtn = rt_band_set_pixel(band2, 1, 1, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects != 1));

	/*
		rast2

		(2, 0)
						+-+-+
						|1|1|
						+-+-+
						|1|1|
						+-+-+
								(4, 2)
	*/
	rt_raster_set_offsets(rast2, 2, 0);

	rtn = rt_band_set_pixel(band2, 0, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 1);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects != 1));

	/*
		rast2

		(0.1, 0.1)
						+-+-+
						|1|1|
						+-+-+
						|1|1|
						+-+-+
								(0.9, 0.9)
	*/
	rt_raster_set_offsets(rast2, 0.1, 0.1);
	rt_raster_set_scale(rast2, 0.4, 0.4);

	rtn = rt_band_set_pixel(band2, 0, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 1);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(-0.1, 0.1)
						+-+-+
						|1|1|
						+-+-+
						|1|1|
						+-+-+
								(0.9, 0.9)
	*/
	rt_raster_set_offsets(rast2, -0.1, 0.1);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	deepRelease(rast2);

	/*
		rast2

		(0, 0)
						+-+-+-+
						|1|1|1|
						+-+-+-+
						|1|1|1|
						+-+-+-+
						|1|1|1|
						+-+-+-+
									(3, 3)
	*/
	rast2 = rt_raster_new(3, 3);
	assert(rast2);

	band2 = addBand(rast2, PT_8BUI, 1, 0);
	CHECK(band2);
	rt_band_set_nodata(band2, 0);
	rtn = rt_band_set_pixel(band2, 0, 0, 1);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 0, 2, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 1);
	rtn = rt_band_set_pixel(band2, 1, 2, 1);
	rtn = rt_band_set_pixel(band2, 2, 0, 1);
	rtn = rt_band_set_pixel(band2, 2, 1, 1);
	rtn = rt_band_set_pixel(band2, 2, 2, 1);

	nodata = rt_band_get_nodata(band2);
	CHECK_EQUALS(nodata, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(-2, -2)
						+-+-+-+
						|1|1|1|
						+-+-+-+
						|1|1|1|
						+-+-+-+
						|1|1|1|
						+-+-+-+
									(1, 1)
	*/
	rt_raster_set_offsets(rast2, -2, -2);

	rtn = rt_band_set_pixel(band2, 0, 0, 1);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 0, 2, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 1);
	rtn = rt_band_set_pixel(band2, 1, 2, 1);
	rtn = rt_band_set_pixel(band2, 2, 0, 1);
	rtn = rt_band_set_pixel(band2, 2, 1, 1);
	rtn = rt_band_set_pixel(band2, 2, 2, 1);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(-2, -2)
						+-+-+-+
						|0|1|1|
						+-+-+-+
						|1|0|1|
						+-+-+-+
						|1|1|0|
						+-+-+-+
									(1, 1)
	*/
	rtn = rt_band_set_pixel(band2, 0, 0, 0);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 0, 2, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 0);
	rtn = rt_band_set_pixel(band2, 1, 2, 1);
	rtn = rt_band_set_pixel(band2, 2, 0, 1);
	rtn = rt_band_set_pixel(band2, 2, 1, 1);
	rtn = rt_band_set_pixel(band2, 2, 2, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(-2, -2)
						+-+-+-+
						|0|1|1|
						+-+-+-+
						|1|0|0|
						+-+-+-+
						|1|0|0|
						+-+-+-+
									(1, 1)
	*/
	rtn = rt_band_set_pixel(band2, 0, 0, 0);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 0, 2, 1);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 0);
	rtn = rt_band_set_pixel(band2, 1, 2, 0);
	rtn = rt_band_set_pixel(band2, 2, 0, 1);
	rtn = rt_band_set_pixel(band2, 2, 1, 0);
	rtn = rt_band_set_pixel(band2, 2, 2, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/*
		rast2

		(-2, -2)
						+-+-+-+
						|0|1|0|
						+-+-+-+
						|1|0|0|
						+-+-+-+
						|0|0|0|
						+-+-+-+
									(1, 1)
	*/
	rtn = rt_band_set_pixel(band2, 0, 0, 0);
	rtn = rt_band_set_pixel(band2, 0, 1, 1);
	rtn = rt_band_set_pixel(band2, 0, 2, 0);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 0);
	rtn = rt_band_set_pixel(band2, 1, 2, 0);
	rtn = rt_band_set_pixel(band2, 2, 0, 0);
	rtn = rt_band_set_pixel(band2, 2, 1, 0);
	rtn = rt_band_set_pixel(band2, 2, 2, 0);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	deepRelease(rast2);

	/* skew tests */
	/* rast2 (skewed by -0.5, 0.5) */
	rast2 = rt_raster_new(3, 3);
	assert(rast2);
	rt_raster_set_skews(rast2, -0.5, 0.5);

	band2 = addBand(rast2, PT_8BUI, 1, 0);
	CHECK(band2);
	rt_band_set_nodata(band2, 0);
	rtn = rt_band_set_pixel(band2, 0, 0, 1);
	rtn = rt_band_set_pixel(band2, 0, 1, 2);
	rtn = rt_band_set_pixel(band2, 0, 2, 3);
	rtn = rt_band_set_pixel(band2, 1, 0, 1);
	rtn = rt_band_set_pixel(band2, 1, 1, 2);
	rtn = rt_band_set_pixel(band2, 1, 2, 3);
	rtn = rt_band_set_pixel(band2, 2, 0, 1);
	rtn = rt_band_set_pixel(band2, 2, 1, 2);
	rtn = rt_band_set_pixel(band2, 2, 2, 3);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/* rast2 (skewed by -1, 1) */
	rt_raster_set_skews(rast2, -1, 1);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	/* rast2 (skewed by 1, -1) */
	rt_raster_set_skews(rast2, 1, -1);

	rtn = rt_raster_intersects(
		rast1, 0,
		rast2, 0,
		&intersects
	);
	CHECK((rtn != 0));
	CHECK((intersects == 1));

	deepRelease(rast2);
	deepRelease(rast1);
}

static void testAlignment() {
	rt_raster rast1;
	rt_raster rast2;
	int rtn;
	int aligned;

	rast1 = rt_raster_new(2, 2);
	assert(rast1);
	rt_raster_set_scale(rast1, 1, 1);

	rast2 = rt_raster_new(10, 10);
	assert(rast2);
	rt_raster_set_scale(rast2, 1, 1);

	rtn = rt_raster_same_alignment(rast1, rast2, &aligned);
	CHECK((rtn != 0));
	CHECK((aligned != 0));

	rt_raster_set_scale(rast2, 0.1, 0.1);
	rtn = rt_raster_same_alignment(rast1, rast2, &aligned);
	CHECK((rtn != 0));
	CHECK((aligned == 0));
	rt_raster_set_scale(rast2, 1, 1);

	rt_raster_set_skews(rast2, -0.5, 0.5);
	rtn = rt_raster_same_alignment(rast1, rast2, &aligned);
	CHECK((rtn != 0));
	CHECK((aligned == 0));
	rt_raster_set_skews(rast2, 0, 0);

	rt_raster_set_offsets(rast2, 1, 1);
	rtn = rt_raster_same_alignment(rast1, rast2, &aligned);
	CHECK((rtn != 0));
	CHECK((aligned != 0));

	rt_raster_set_offsets(rast2, 2, 3);
	rtn = rt_raster_same_alignment(rast1, rast2, &aligned);
	CHECK((rtn != 0));
	CHECK((aligned != 0));

	rt_raster_set_offsets(rast2, 0.1, 0.1);
	rtn = rt_raster_same_alignment(rast1, rast2, &aligned);
	CHECK((rtn != 0));
	CHECK((aligned == 0));

	deepRelease(rast2);
	deepRelease(rast1);
}

static void testFromTwoRasters() {
	rt_raster rast1;
	rt_raster rast2;
	rt_raster rast = NULL;
	int err;
	double offset[4] = {0.};

	rast1 = rt_raster_new(4, 4);
	assert(rast1);
	rt_raster_set_scale(rast1, 1, 1);
	rt_raster_set_offsets(rast1, -2, -2);

	rast2 = rt_raster_new(2, 2);
	assert(rast2);
	rt_raster_set_scale(rast2, 1, 1);

	rast = rt_raster_from_two_rasters(
		rast1, rast2,
		ET_FIRST,
		&err,
		offset
	);
	CHECK((err != 0));
	CHECK(rast);
	CHECK((rt_raster_get_width(rast) == 4));
	CHECK((rt_raster_get_height(rast) == 4));
	CHECK(FLT_EQ(offset[0], 0));
	CHECK(FLT_EQ(offset[1], 0));
	CHECK(FLT_EQ(offset[2], 2));
	CHECK(FLT_EQ(offset[3], 2));
	deepRelease(rast);

	rast = rt_raster_from_two_rasters(
		rast1, rast2,
		ET_SECOND,
		&err,
		offset
	);
	CHECK((err != 0));
	CHECK(rast);
	CHECK((rt_raster_get_width(rast) == 2));
	CHECK((rt_raster_get_height(rast) == 2));
	CHECK(FLT_EQ(offset[0], -2));
	CHECK(FLT_EQ(offset[1], -2));
	CHECK(FLT_EQ(offset[2], 0));
	CHECK(FLT_EQ(offset[3], 0));
	deepRelease(rast);

	rast = rt_raster_from_two_rasters(
		rast1, rast2,
		ET_INTERSECTION,
		&err,
		offset
	);
	CHECK((err != 0));
	CHECK(rast);
	CHECK((rt_raster_get_width(rast) == 2));
	CHECK((rt_raster_get_height(rast) == 2));
	CHECK(FLT_EQ(offset[0], -2));
	CHECK(FLT_EQ(offset[1], -2));
	CHECK(FLT_EQ(offset[2], 0));
	CHECK(FLT_EQ(offset[3], 0));
	deepRelease(rast);

	rast = rt_raster_from_two_rasters(
		rast1, rast2,
		ET_UNION,
		&err,
		offset
	);
	CHECK((err != 0));
	CHECK(rast);
	CHECK((rt_raster_get_width(rast) == 4));
	CHECK((rt_raster_get_height(rast) == 4));
	CHECK(FLT_EQ(offset[0], 0));
	CHECK(FLT_EQ(offset[1], 0));
	CHECK(FLT_EQ(offset[2], 2));
	CHECK(FLT_EQ(offset[3], 2));
	deepRelease(rast);

	rt_raster_set_scale(rast2, 1, 0.1);
	rast = rt_raster_from_two_rasters(
		rast1, rast2,
		ET_UNION,
		&err,
		offset
	);
	CHECK((err == 0));
	rt_raster_set_scale(rast2, 1, 1);

	rt_raster_set_srid(rast2, 9999);
	rast = rt_raster_from_two_rasters(
		rast1, rast2,
		ET_UNION,
		&err,
		offset
	);
	CHECK((err == 0));
	rt_raster_set_srid(rast2, 0);

	rt_raster_set_skews(rast2, -1, 1);
	rast = rt_raster_from_two_rasters(
		rast1, rast2,
		ET_UNION,
		&err,
		offset
	);
	CHECK((err == 0));

	deepRelease(rast2);
	deepRelease(rast1);
}

int
main()
{
	int i;
    rt_raster raster;
    rt_band band_1BB, band_2BUI, band_4BUI,
            band_8BSI, band_8BUI, band_16BSI, band_16BUI,
            band_32BSI, band_32BUI, band_32BF,
            band_64BF;

    raster = rt_raster_new(256, 256);
    assert(raster); /* or we're out of virtual memory */

	printf("Checking empty and hasnoband functions...\n");
	{ /* Check isEmpty and hasnoband */
		CHECK(!rt_raster_is_empty(raster));

		/* Create a dummy empty raster to test the opposite
		 * to the previous sentence
		 */
		rt_raster emptyraster = rt_raster_new(0, 0);
		CHECK(rt_raster_is_empty(emptyraster));
		rt_raster_destroy(emptyraster);

		/* Once we add a band to this raster, we'll check the opposite */
		CHECK(rt_raster_has_no_band(raster, 1));
	}


	printf("Checking raster properties...\n");
    { /* Check scale */
        float scale;
        scale = rt_raster_get_x_scale(raster);
        CHECK_EQUALS(scale, 1);
        scale = rt_raster_get_y_scale(raster);
        CHECK_EQUALS(scale, 1);
    }

    { /* Check offsets */
        float off;

        off = rt_raster_get_x_offset(raster);
        CHECK_EQUALS(off, 0.5);

        off = rt_raster_get_y_offset(raster);
        CHECK_EQUALS(off, 0.5);

        rt_raster_set_offsets(raster, 30, 70);

        off = rt_raster_get_x_offset(raster);
        CHECK_EQUALS(off, 30);

        off = rt_raster_get_y_offset(raster);
        CHECK_EQUALS(off, 70);

        rt_raster_set_offsets(raster, 0.5, 0.5);
    }

    { /* Check skew */
        float off;

        off = rt_raster_get_x_skew(raster);
        CHECK_EQUALS(off, 0);

        off = rt_raster_get_y_skew(raster);
        CHECK_EQUALS(off, 0);

        rt_raster_set_skews(raster, 4, 5);

        off = rt_raster_get_x_skew(raster);
        CHECK_EQUALS(off, 4);

        off = rt_raster_get_y_skew(raster);
        CHECK_EQUALS(off, 5);

        rt_raster_set_skews(raster, 0, 0);
    }

    { /* Check SRID */
        int32_t srid;
        srid = rt_raster_get_srid(raster);
        CHECK_EQUALS(srid, 0);

        rt_raster_set_srid(raster, 65546);
        srid = rt_raster_get_srid(raster);
        CHECK_EQUALS(srid, 65546);
    }

    printf("Raster starts with %d bands\n",
        rt_raster_get_num_bands(raster));

    { /* Check convex hull, based on offset, scale and rotation */
        LWPOLY *convexhull;
        POINTARRAY *ring;
        POINT4D pt;

        /* will rotate the raster to see difference with the envelope */
        rt_raster_set_skews(raster, 4, 5);

        convexhull = rt_raster_get_convex_hull(raster);
        CHECK_EQUALS(convexhull->srid, rt_raster_get_srid(raster));
        CHECK_EQUALS(convexhull->nrings, 1);
        ring = convexhull->rings[0];
        CHECK(ring);
        CHECK_EQUALS(ring->npoints, 5);

        getPoint4d_p(ring, 0, &pt);
        printf("First point on convexhull ring is %g,%g\n", pt.x, pt.y);
        CHECK_EQUALS_DOUBLE(pt.x, 0.5);
        CHECK_EQUALS_DOUBLE(pt.y, 0.5);

        getPoint4d_p(ring, 1, &pt);
        printf("Second point on convexhull ring is %g,%g\n", pt.x, pt.y);
        CHECK_EQUALS_DOUBLE(pt.x, 256.5);
        CHECK_EQUALS_DOUBLE(pt.y, 1280.5);

        getPoint4d_p(ring, 2, &pt);
        printf("Third point on convexhull ring is %g,%g\n", pt.x, pt.y);
        CHECK_EQUALS_DOUBLE(pt.x, 1280.5);
        CHECK_EQUALS_DOUBLE(pt.y, 1536.5);

        getPoint4d_p(ring, 3, &pt);
        printf("Fourth point on convexhull ring is %g,%g\n", pt.x, pt.y);
        CHECK_EQUALS_DOUBLE(pt.x, 1024.5);
        CHECK_EQUALS_DOUBLE(pt.y, 256.5);

        getPoint4d_p(ring, 4, &pt);
        printf("Fifth point on convexhull ring is %g,%g\n", pt.x, pt.y);
        CHECK_EQUALS_DOUBLE(pt.x, 0.5);
        CHECK_EQUALS_DOUBLE(pt.y, 0.5);

        lwpoly_free(convexhull);

        rt_raster_set_skews(raster, 0, 0);
    }

    {   /* Check ST_AsPolygon */
        printf("Testing polygonize function\n");

		/* First test: NODATA value = -1 */
        rt_raster rt = fillRasterToPolygonize(1, -1.0);

		/* We can check rt_raster_has_no_band here too */
		CHECK(!rt_raster_has_no_band(rt, 0));

        int nPols = 0;

        rt_geomval gv = (rt_geomval) rt_raster_dump_as_wktpolygons(rt, 0, &nPols);

	/*
		for (i = 0; i < nPols; i++) {
			printf("(i, val, geom) = (%d, %f, %s)\n", i, gv[i].val, gv[i].geom);
		}
	*/


#if GDALFPOLYGONIZE == 1
		CHECK(FLT_EQ(gv[0].val, 1.8));
#else
		CHECK(FLT_EQ(gv[0].val, 2.0));
#endif

		CHECK(!strcmp(gv[0].geom, "POLYGON ((3 1,3 2,2 2,2 3,1 3,1 6,2 6,2 7,3 7,3 8,5 8,5 6,3 6,3 3,4 3,5 3,5 1,3 1))"));

		CHECK_EQUALS_DOUBLE(gv[1].val, 0.0);
		CHECK(!strcmp(gv[1].geom, "POLYGON ((3 3,3 6,6 6,6 3,3 3))"));

#if GDALFPOLYGONIZE == 1
		CHECK(FLT_EQ(gv[2].val, 2.8));
#else
		CHECK(FLT_EQ(gv[2].val, 3.0));
#endif

		CHECK(!strcmp(gv[2].geom, "POLYGON ((5 1,5 3,6 3,6 6,5 6,5 8,6 8,6 7,7 7,7 6,8 6,8 3,7 3,7 2,6 2,6 1,5 1))"));

		CHECK_EQUALS_DOUBLE(gv[3].val, 0.0);
		CHECK(!strcmp(gv[3].geom, "POLYGON ((0 0,0 9,9 9,9 0,0 0),(6 7,6 8,3 8,3 7,2 7,2 6,1 6,1 3,2 3,2 2,3 2,3 1,6 1,6 2,7 2,7 3,8 3,8 6,7 6,7 7,6 7))"));


	for (i = 0; i < nPols; i++) rtdealloc(gv[i].geom);
	rtdealloc(gv);
        deepRelease(rt);


		/* Second test: NODATA value = 1.8 */
#if GDALFPOLYGONIZE == 1
		rt = fillRasterToPolygonize(1, 1.8);
#else
		rt = fillRasterToPolygonize(1, 2.0);
#endif


		/* We can check rt_raster_has_no_band here too */
		CHECK(!rt_raster_has_no_band(rt, 0));

    	nPols = 0;

    	gv = (rt_geomval) rt_raster_dump_as_wktpolygons(rt, 0, &nPols);

	/*
		for (i = 0; i < nPols; i++) {
			printf("(i, val, geom) = (%d, %f, %s)\n", i, gv[i].val, gv[i].geom);
		}
	*/

#if GDALFPOLYGONIZE == 1
		CHECK_EQUALS_DOUBLE(gv[1].val, 0.0);
		CHECK(!strcmp(gv[1].geom, "POLYGON ((3 3,3 6,6 6,6 3,3 3))"));

		CHECK(FLT_EQ(gv[2].val, 2.8));
		CHECK(!strcmp(gv[2].geom, "POLYGON ((5 1,5 3,6 3,6 6,5 6,5 8,6 8,6 7,7 7,7 6,8 6,8 3,7 3,7 2,6 2,6 1,5 1))"));

		CHECK_EQUALS_DOUBLE(gv[3].val, 0.0);
		CHECK(!strcmp(gv[3].geom, "POLYGON ((0 0,0 9,9 9,9 0,0 0),(6 7,6 8,3 8,3 7,2 7,2 6,1 6,1 3,2 3,2 2,3 2,3 1,6 1,6 2,7 2,7 3,8 3,8 6,7 6,7 7,6 7))"));
#else
		CHECK_EQUALS_DOUBLE(gv[0].val, 0.0);
		CHECK(!strcmp(gv[0].geom, "POLYGON ((3 3,3 6,6 6,6 3,3 3))"));

		CHECK(FLT_EQ(gv[1].val, 3.0));
    CHECK(!strcmp(gv[1].geom, "POLYGON ((5 1,5 3,6 3,6 6,5 6,5 8,6 8,6 7,7 7,7 6,8 6,8 3,7 3,7 2,6 2,6 1,5 1))"));

		CHECK_EQUALS_DOUBLE(gv[2].val, 0.0);
		CHECK(!strcmp(gv[2].geom, "POLYGON ((0 0,0 9,9 9,9 0,0 0),(6 7,6 8,3 8,3 7,2 7,2 6,1 6,1 3,2 3,2 2,3 2,3 1,6 1,6 2,7 2,7 3,8 3,8 6,7 6,7 7,6 7))"));
#endif

	for (i = 0; i < nPols; i++) rtdealloc(gv[i].geom);
	rtdealloc(gv);
        deepRelease(rt);

		/* Third test: NODATA value = 2.8 */
#if GDALFPOLYGONIZE == 1
    	rt = fillRasterToPolygonize(1, 2.8);
#else	
    	rt = fillRasterToPolygonize(1, 3.0);
#endif

		/* We can check rt_raster_has_no_band here too */
		CHECK(!rt_raster_has_no_band(rt, 0));

		nPols = 0;

    	gv = (rt_geomval) rt_raster_dump_as_wktpolygons(rt, 0, &nPols);

	/*
		for (i = 0; i < nPols; i++) {
			printf("(i, val, geom) = (%d, %f, %s)\n", i, gv[i].val, gv[i].geom);
		}
	*/

#if GDALFPOLYGONIZE == 1
    	CHECK(FLT_EQ(gv[0].val, 1.8));

		CHECK_EQUALS_DOUBLE(gv[3].val, 0.0);
		CHECK(!strcmp(gv[3].geom, "POLYGON ((0 0,0 9,9 9,9 0,0 0),(6 7,6 8,3 8,3 7,2 7,2 6,1 6,1 3,2 3,2 2,3 2,3 1,6 1,6 2,7 2,7 3,8 3,8 6,7 6,7 7,6 7))"));
#else
   	CHECK(FLT_EQ(gv[0].val, 2.0));

		CHECK_EQUALS_DOUBLE(gv[2].val, 0.0);
		CHECK(!strcmp(gv[2].geom, "POLYGON ((0 0,0 9,9 9,9 0,0 0),(6 7,6 8,3 8,3 7,2 7,2 6,1 6,1 3,2 3,2 2,3 2,3 1,6 1,6 2,7 2,7 3,8 3,8 6,7 6,7 7,6 7))"));
#endif

    CHECK(!strcmp(gv[0].geom, "POLYGON ((3 1,3 2,2 2,2 3,1 3,1 6,2 6,2 7,3 7,3 8,5 8,5 6,3 6,3 3,4 3,5 3,5 1,3 1))"));

		CHECK_EQUALS_DOUBLE(gv[1].val, 0.0);
		CHECK(!strcmp(gv[1].geom, "POLYGON ((3 3,3 6,6 6,6 3,3 3))"));

	for (i = 0; i < nPols; i++) rtdealloc(gv[i].geom);
	rtdealloc(gv);
        deepRelease(rt);

		/* Fourth test: NODATA value = 0 */
    	rt = fillRasterToPolygonize(1, 0.0);

        /* We can check rt_raster_has_no_band here too */
		CHECK(!rt_raster_has_no_band(rt, 0));

		nPols = 0;

   		gv = (rt_geomval) rt_raster_dump_as_wktpolygons(rt, 0, &nPols);

		/*
		for (i = 0; i < nPols; i++) {
			printf("(i, val, geom) = (%d, %f, %s)\n", i, gv[i].val, gv[i].geom);
		}
		*/

#if GDALFPOLYGONIZE == 1
    	CHECK(FLT_EQ(gv[0].val, 1.8));
#else
    	CHECK(FLT_EQ(gv[0].val, 2.0));
#endif

	   	CHECK(!strcmp(gv[0].geom, "POLYGON ((3 1,3 2,2 2,2 3,1 3,1 6,2 6,2 7,3 7,3 8,5 8,5 6,3 6,3 3,4 3,5 3,5 1,3 1))"));

#if GDALFPOLYGONIZE == 1
    	CHECK(FLT_EQ(gv[1].val, 2.8));
#else
    	CHECK(FLT_EQ(gv[1].val, 3.0));
#endif

	    CHECK(!strcmp(gv[1].geom, "POLYGON ((5 1,5 3,6 3,6 6,5 6,5 8,6 8,6 7,7 7,7 6,8 6,8 3,7 3,7 2,6 2,6 1,5 1))"));

	for (i = 0; i < nPols; i++) rtdealloc(gv[i].geom);
	rtdealloc(gv);
        deepRelease(rt);

    	/* Last test: There is no NODATA value (all values are valid) */
    	rt = fillRasterToPolygonize(0, 0.0);

		/* We can check rt_raster_has_no_band here too */
		CHECK(!rt_raster_has_no_band(rt, 0));

	    nPols = 0;

    	gv = (rt_geomval) rt_raster_dump_as_wktpolygons(rt, 0, &nPols);

	/*
		for (i = 0; i < nPols; i++) {
			printf("(i, val, geom) = (%d, %f, %s)\n", i, gv[i].val, gv[i].geom);
		}
	*/

#if GDALFPOLYGONIZE == 1
    	CHECK(FLT_EQ(gv[0].val, 1.8));
#else
    	CHECK(FLT_EQ(gv[0].val, 2.0));
#endif

   		CHECK(!strcmp(gv[0].geom, "POLYGON ((3 1,3 2,2 2,2 3,1 3,1 6,2 6,2 7,3 7,3 8,5 8,5 6,3 6,3 3,4 3,5 3,5 1,3 1))"));

		CHECK_EQUALS_DOUBLE(gv[1].val, 0.0);
		CHECK(!strcmp(gv[1].geom, "POLYGON ((3 3,3 6,6 6,6 3,3 3))"));

#if GDALFPOLYGONIZE == 1
    	CHECK(FLT_EQ(gv[2].val, 2.8));
#else
    	CHECK(FLT_EQ(gv[2].val, 3.0));
#endif

	    CHECK(!strcmp(gv[2].geom, "POLYGON ((5 1,5 3,6 3,6 6,5 6,5 8,6 8,6 7,7 7,7 6,8 6,8 3,7 3,7 2,6 2,6 1,5 1))"));

		CHECK_EQUALS_DOUBLE(gv[3].val, 0.0);
		CHECK(!strcmp(gv[3].geom, "POLYGON ((0 0,0 9,9 9,9 0,0 0),(6 7,6 8,3 8,3 7,2 7,2 6,1 6,1 3,2 3,2 2,3 2,3 1,6 1,6 2,7 2,7 3,8 3,8 6,7 6,7 7,6 7))"));

	for (i = 0; i < nPols; i++) rtdealloc(gv[i].geom);
	rtdealloc(gv);
        deepRelease(rt);

    }

    printf("Testing 1BB band\n");
    band_1BB = addBand(raster, PT_1BB, 0, 0);
    testBand1BB(band_1BB);

    printf("Testing 2BB band\n");
    band_2BUI = addBand(raster, PT_2BUI, 0, 0);
    testBand2BUI(band_2BUI);

    printf("Testing 4BUI band\n");
    band_4BUI = addBand(raster, PT_4BUI, 0, 0);
    testBand4BUI(band_4BUI);

    printf("Testing 8BUI band\n");
    band_8BUI = addBand(raster, PT_8BUI, 0, 0);
    testBand8BUI(band_8BUI);

    printf("Testing 8BSI band\n");
    band_8BSI = addBand(raster, PT_8BSI, 0, 0);
    testBand8BSI(band_8BSI);

    printf("Testing 16BSI band\n");
    band_16BSI = addBand(raster, PT_16BSI, 0, 0);
    testBand16BSI(band_16BSI);

    printf("Testing 16BUI band\n");
    band_16BUI = addBand(raster, PT_16BUI, 0, 0);
    testBand16BUI(band_16BUI);

    printf("Testing 32BUI band\n");
    band_32BUI = addBand(raster, PT_32BUI, 0, 0);
    testBand32BUI(band_32BUI);

    printf("Testing 32BSI band\n");
    band_32BSI = addBand(raster, PT_32BSI, 0, 0);
    testBand32BSI(band_32BSI);

    printf("Testing 32BF band\n");
    band_32BF = addBand(raster, PT_32BF, 0, 0);
    testBand32BF(band_32BF);

    printf("Testing 64BF band\n");
    band_64BF = addBand(raster, PT_64BF, 0, 0);
    testBand64BF(band_64BF);

    printf("Testing band hasnodata flag\n");
    testBandHasNoData(band_64BF);

		printf("Testing rt_raster_from_band\n");
		testRasterFromBand(raster);
		printf("Successfully tested rt_raster_from_band\n");

		printf("Testing band stats\n");
		testBandStats();
		printf("Successfully tested band stats\n");

		printf("Testing rt_raster_replace_band\n");
		testRasterReplaceBand();
		printf("Successfully tested rt_raster_replace_band\n");

		printf("Testing rt_band_reclass\n");
		testBandReclass();
		printf("Successfully tested rt_band_reclass\n");

		printf("Testing rt_raster_to_gdal\n");
		testRasterToGDAL();
		printf("Successfully tested rt_raster_to_gdal\n");

		printf("Testing rt_raster_gdal_drivers\n");
		testGDALDrivers();
		printf("Successfully tested rt_raster_gdal_drivers\n");

		printf("Testing rt_band_get_value_count\n");
		testValueCount();
		printf("Successfully tested rt_band_get_value_count\n");

		printf("Testing rt_raster_from_gdal_dataset\n");
		testGDALToRaster();
		printf("Successfully tested rt_raster_from_gdal_dataset\n");

		printf("Testing rt_raster_gdal_warp\n");
		testGDALWarp();
		printf("Successfully tested rt_raster_gdal_warp\n");

		printf("Testing rt_raster_gdal_rasterize\n");
		testGDALRasterize();
		printf("Successfully tested rt_raster_gdal_rasterize\n");

		printf("Testing rt_raster_intersects\n");
		testIntersects();
		printf("Successfully tested rt_raster_intersects\n");

		printf("Testing rt_raster_same_alignment\n");
		testAlignment();
		printf("Successfully tested rt_raster_same_alignment\n");

		printf("Testing rt_raster_from_two_rasters\n");
		testFromTwoRasters();
		printf("Successfully tested rt_raster_from_two_rasters\n");

    deepRelease(raster);

    return EXIT_SUCCESS;
}

/* This is needed by liblwgeom */
void
lwgeom_init_allocators(void)
{
    lwgeom_install_default_allocators();
}


void rt_init_allocators(void)
{
    rt_install_default_allocators();
}
