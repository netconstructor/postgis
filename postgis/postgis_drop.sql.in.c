-- $Id$
-- First drop old aggregates
DROP AGGREGATE IF EXISTS memgeomunion(geometry);
DROP AGGREGATE IF EXISTS geomunion(geometry);
DROP AGGREGATE IF EXISTS st_geomunion(geometry);
DROP AGGREGATE IF EXISTS accum_old(geometry);
DROP AGGREGATE IF EXISTS st_accum_old(geometry);

-- BEGIN Management functions that now have default param for typmod --
DROP FUNCTION IF EXISTS AddGeometryColumn(varchar,varchar,varchar,varchar,integer,varchar,integer);
DROP FUNCTION IF EXISTS AddGeometryColumn(varchar,varchar,varchar,integer,varchar,integer);
DROP FUNCTION IF EXISTS AddGeometryColumn(varchar,varchar,integer,varchar,integer);
DROP FUNCTION IF EXISTS populate_geometry_columns();
DROP FUNCTION IF EXISTS populate_geometry_columns(oid);

-- END Management functions now have default parameter for typmod --
-- Then drop old functions
DROP FUNCTION IF EXISTS box2d_overleft(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_overright(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_left(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_right(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_contain(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_contained(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_overlap(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_same(box2d, box2d);
DROP FUNCTION IF EXISTS box2d_intersects(box2d, box2d);
DROP FUNCTION IF EXISTS st_area(geography); -- this one changed to use default parameters
DROP FUNCTION IF EXISTS st_asx3d(geometry); -- this one changed to use default parameters so full function deals with it
DROP FUNCTION IF EXISTS st_box2d_overleft(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_overright(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_left(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_right(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_contain(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_contained(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_overlap(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_same(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_intersects(box2d, box2d);
DROP FUNCTION IF EXISTS st_box2d_in(cstring);
DROP FUNCTION IF EXISTS st_box2d_out(box2d);
DROP FUNCTION IF EXISTS st_box2d(geometry);
DROP FUNCTION IF EXISTS st_box2d(box3d);
DROP FUNCTION IF EXISTS st_box2d(box3d_extent);
DROP FUNCTION IF EXISTS st_box3d(box2d);
DROP FUNCTION IF EXISTS st_box(box3d);
DROP FUNCTION IF EXISTS st_box3d(geometry);
DROP FUNCTION IF EXISTS st_box(geometry);
DROP FUNCTION IF EXISTS ST_ConcaveHull(geometry,float); -- this one changed to use default parameters
DROP FUNCTION IF EXISTS st_text(geometry);
DROP FUNCTION IF EXISTS st_geometry(box2d);
DROP FUNCTION IF EXISTS st_geometry(box3d);
DROP FUNCTION IF EXISTS st_geometry(text);
DROP FUNCTION IF EXISTS st_geometry(bytea);
DROP FUNCTION IF EXISTS st_bytea(geometry);
DROP FUNCTION IF EXISTS st_box3d_extent(box3d_extent);
DROP FUNCTION IF EXISTS st_geometry(box3d_extent);
DROP FUNCTION IF EXISTS st_addbbox(geometry);
DROP FUNCTION IF EXISTS st_dropbbox(geometry); 
DROP FUNCTION IF EXISTS st_hasbbox(geometry); 
DROP FUNCTION IF EXISTS cache_bbox();
DROP FUNCTION IF EXISTS st_cache_bbox();
DROP FUNCTION IF EXISTS st_length(geography); -- this one changed to use default parameters
DROP FUNCTION IF EXISTS st_perimeter(geography); -- this one changed to use default parameters
DROP FUNCTION IF EXISTS transform_geometry(geometry,text,text,int);
DROP FUNCTION IF EXISTS collector(geometry, geometry);
DROP FUNCTION IF EXISTS st_collector(geometry, geometry);
DROP FUNCTION IF EXISTS geom_accum (geometry[],geometry);
DROP FUNCTION IF EXISTS st_geom_accum (geometry[],geometry);
DROP FUNCTION IF EXISTS collect_garray (geometry[]);
DROP FUNCTION IF EXISTS st_collect_garray (geometry[]);
DROP FUNCTION IF EXISTS geosnoop(geometry);
DROP FUNCTION IF EXISTS jtsnoop(geometry);
DROP FUNCTION IF EXISTS st_noop(geometry);
DROP FUNCTION IF EXISTS st_max_distance(geometry, geometry);
DROP FUNCTION IF EXISTS  ST_MinimumBoundingCircle(geometry); --changed to use default parameters
-- Drop internals that should never have existed --
DROP FUNCTION IF EXISTS st_geometry_analyze(internal);
DROP FUNCTION IF EXISTS st_geometry_in(cstring);
DROP FUNCTION IF EXISTS st_geometry_out(geometry);
DROP FUNCTION IF EXISTS st_geometry_recv(internal);
DROP FUNCTION IF EXISTS st_geometry_send(geometry);
DROP FUNCTION IF EXISTS st_spheroid_in(cstring);
DROP FUNCTION IF EXISTS st_spheroid_out(spheroid);
DROP FUNCTION IF EXISTS st_geometry_lt(geometry, geometry);
DROP FUNCTION IF EXISTS st_geometry_gt(geometry, geometry);
DROP FUNCTION IF EXISTS st_geometry_ge(geometry, geometry);
DROP FUNCTION IF EXISTS st_geometry_eq(geometry, geometry);
DROP FUNCTION IF EXISTS st_geometry_cmp(geometry, geometry);
DROP FUNCTION IF EXISTS SnapToGrid(geometry, float8, float8);