#include "stdafx.h"
#include "SqliteLoad.h"
#include <osg/Geode>
#include <unordered_map>

#include <Box.h>
#include <CircularTorus.h>
#include <CombineGeometry.h>
#include <Cone.h>
#include <Cylinder.h>
#include <Ellipsoid.h>
#include <Prism.h>
#include <Pyramid.h>
#include <RectangularTorus.h>
#include <RectCirc.h>
#include <Saddle.h>
#include <SCylinder.h>
#include <Snout.h>
#include <Sphere.h>
#include <Wedge.h>

osg::Vec4 CvtColor(int color);

SqliteLoad::SqliteLoad(osg::ref_ptr<osg::Group> &root, const std::string &filePath)
	: m_root(root)
	, m_filePath(filePath)
	, m_pDb(NULL)
{

}

SqliteLoad::~SqliteLoad()
{
	if (m_pDb != NULL)
	{
		sqlite3_close(m_pDb);
		m_pDb = NULL;
	}
}

bool SqliteLoad::doLoad()
{
	if ((m_errorCode = init()) != SQLITE_OK)
		return false;
	clock_t start = clock();
	if (!loadBox())
		return false;
	if (!loadCircularTorus())
		return false;
	if (!loadCone())
		return false;
	if (!loadCylinder())
		return false;
	if (!loadEllipsoid())
		return false;
	if (!loadPrism())
		return false;
	if (!loadPyramid())
		return false;
	if (!loadRectCirc())
		return false;
	if (!loadRectangularTorus())
		return false;
	if (!loadSCylinder())
		return false;
	if (!loadSnout())
		return false;
	if (!loadSphere())
		return false;
	if (!loadWedge())
		return false;
	if (!loadCombineGeometry())
		return false;

	clock_t end = clock();
	CString msg;
	msg.Format("Time = %lf", ((double)end - start) / 1000.0);
	//AfxMessageBox(msg);

	return true;
}

const char * SqliteLoad::getErrorMessage() const
{
	return sqlite3_errmsg(m_pDb);
}

int SqliteLoad::init()
{
	return sqlite3_open(m_filePath.c_str(), &m_pDb);
}

bool SqliteLoad::loadBox()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, xlen_x, xlen_y, xlen_z, ylen_x, ylen_y, ylen_z, "
		L" zlen_x, zlen_y, zlen_z, color from box";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> boxGeode(new osg::Geode);

	osg::Vec3 org, xLen, yLen, zLen;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		xLen[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xLen[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xLen[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		yLen[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		yLen[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		yLen[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		zLen[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		zLen[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		zLen[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Box> box(new Geometry::Box);
		box->setOrg(org);
		box->setXLen(xLen);
		box->setYLen(yLen);
		box->setZLen(zLen);
		box->setColor(CvtColor(color));
		box->draw();
		
		boxGeode->addDrawable(box);
	}

	m_root->addChild(boxGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadCircularTorus()
{
	const wchar_t *zSql = L"select center_x, center_y, center_z, "
		L" start_pnt_x, start_pnt_y, start_pnt_z, "
		L" normal_x, normal_y, normal_z, "
		L" start_radius, end_radius, angle, color from circular_torus";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 center, startPnt, normal;
	double startRadius, endRadius, angle;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		center[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		startPnt[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		startPnt[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		startPnt[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		normal[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		normal[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		normal[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		startRadius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		endRadius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		angle = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::CircularTorus> ct(new Geometry::CircularTorus);
		ct->setCenter(center);
		ct->setStartPnt(startPnt);
		ct->setNormal(normal);
		ct->setStartRadius(startRadius);
		ct->setEndRadius(endRadius);
		ct->setAngle(angle);
		ct->setColor(CvtColor(color));
		ct->draw();

		ctGeode->addDrawable(ct);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadCone()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" height_x, height_y, height_z, "
		L" offset_x, offset_y, offset_z, "
		L" radius, color from cone";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 org, height, offset;
	double radius;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		offset[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		radius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Cone> cone(new Geometry::Cone);
		cone->setOrg(org);
		cone->setHeight(height);
		cone->setOffset(offset);
		cone->setRadius(radius);
		cone->setColor(CvtColor(color));
		cone->draw();

		ctGeode->addDrawable(cone);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadCylinder()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" height_x, height_y, height_z, "
		L" radius, color from cylinder";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 org, height;
	double radius;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		radius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Cylinder> cylinder(new Geometry::Cylinder);
		cylinder->setOrg(org);
		cylinder->setHeight(height);
		cylinder->setRadius(radius);
		cylinder->setColor(CvtColor(color));
		cylinder->draw();

		ctGeode->addDrawable(cylinder);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadEllipsoid()
{
	const wchar_t *zSql = L"select center_x, center_y, center_z, "
		L" a_len_x, a_len_y, a_len_z, "
		L" b_radius, angle, color from ellipsoid";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 center, aLen;
	double bRadius, angle;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		center[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		aLen[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		aLen[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		aLen[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		bRadius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		angle = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Ellipsoid> ellipsoid(new Geometry::Ellipsoid);
		ellipsoid->setCenter(center);
		ellipsoid->setALen(aLen);
		ellipsoid->setBRadius(bRadius);
		ellipsoid->setAngle(angle);
		ellipsoid->setColor(CvtColor(color));
		ellipsoid->draw();

		ctGeode->addDrawable(ellipsoid);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadPrism()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" height_x, height_y, height_z, "
		L" bottom_start_pnt_x, bottom_start_pnt_y, bottom_start_pnt_z, "
		L" edge_num, color from prism";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 org, height, bottomStartPnt;
	int edgeNum, color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		bottomStartPnt[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		bottomStartPnt[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		bottomStartPnt[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		edgeNum = sqlite3_column_int(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Prism> prism(new Geometry::Prism);
		prism->setOrg(org);
		prism->setHeight(height);
		prism->setBottomStartPnt(bottomStartPnt);
		prism->setEdgeNum(edgeNum);
		prism->setColor(CvtColor(color));
		prism->draw();

		ctGeode->addDrawable(prism);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadPyramid()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" height_x, height_y, height_z, "
		L" xaxis_x, xaxis_y, xaxis_z, "
		L" offset_x, offset_y, offset_z, "
		L" bottom_xlen, bottom_ylen, top_xlen, top_ylen, color from pyramid";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 org, height, xAxis, offset;
	double bottomXLen, bottomYLen, topXLen, topYLen;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		xAxis[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xAxis[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xAxis[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		offset[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		bottomXLen = sqlite3_column_double(pStmt, iCol);
		++iCol;
		bottomYLen = sqlite3_column_double(pStmt, iCol);
		++iCol;
		topXLen = sqlite3_column_double(pStmt, iCol);
		++iCol;
		topYLen = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Pyramid> pyramid(new Geometry::Pyramid);
		pyramid->setOrg(org);
		pyramid->setHeight(height);
		pyramid->setXAxis(xAxis);
		pyramid->setOffset(offset);
		pyramid->setBottomXLen(bottomXLen);
		pyramid->setBottomYLen(bottomYLen);
		pyramid->setTopXLen(topXLen);
		pyramid->setTopYLen(topYLen);
		pyramid->setColor(CvtColor(color));
		pyramid->draw();

		ctGeode->addDrawable(pyramid);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadRectCirc()
{
	const wchar_t *zSql = L"select rect_center_x, rect_center_y, rect_center_z, "
		L" xlen_x, xlen_y, xlen_z, "
		L" height_x, height_y, height_z, "
		L" offset_x, offset_y, offset_z, "
		L" ylen, radius, color from rect_circ";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 rectCenter, height, xLen, offset;
	double yLen, radius;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		rectCenter[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		rectCenter[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		rectCenter[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		xLen[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xLen[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xLen[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		offset[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		yLen = sqlite3_column_double(pStmt, iCol);
		++iCol;
		radius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::RectCirc> rectCirc(new Geometry::RectCirc);
		rectCirc->setRectCenter(rectCenter);
		rectCirc->setHeight(height);
		rectCirc->setXLen(xLen);
		rectCirc->setOffset(offset);
		rectCirc->setYLen(yLen);
		rectCirc->setRadius(radius);
		rectCirc->setColor(CvtColor(color));
		rectCirc->draw();

		ctGeode->addDrawable(rectCirc);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadRectangularTorus()
{
	const wchar_t *zSql = L"select center_x, center_y, center_z, "
		L" start_pnt_x, start_pnt_y, start_pnt_z, "
		L" normal_x, normal_y, normal_z, "
		L" start_width, start_height, end_width, end_height, angle, color from rectangular_torus";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 center, startPnt, normal;
	double startWidth, startHeight, endWidth, endHeight, angle;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		center[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		startPnt[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		startPnt[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		startPnt[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		normal[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		normal[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		normal[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		startWidth = sqlite3_column_double(pStmt, iCol);
		++iCol;
		startHeight = sqlite3_column_double(pStmt, iCol);
		++iCol;
		endWidth = sqlite3_column_double(pStmt, iCol);
		++iCol;
		endHeight = sqlite3_column_double(pStmt, iCol);
		++iCol;
		angle = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::RectangularTorus> rt(new Geometry::RectangularTorus);
		rt->setCenter(center);
		rt->setStartPnt(startPnt);
		rt->setNormal(normal);
		rt->setStartWidth(startWidth);
		rt->setStartHeight(startHeight);
		rt->setEndWidth(endWidth);
		rt->setEndHeight(endHeight);
		rt->setAngle(angle);
		rt->setColor(CvtColor(color));
		rt->draw();

		ctGeode->addDrawable(rt);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadSaddle()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" xlen_x, xlen_y, xlen_z, "
		L" zlen_x, zlen_y, zlen_z, "
		L" ylen, radius, color from saddle";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 org, xLen, zLen;
	double yLen, radius;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		xLen[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xLen[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		xLen[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		zLen[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		zLen[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		zLen[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		yLen = sqlite3_column_double(pStmt, iCol);
		++iCol;
		radius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Saddle> saddle(new Geometry::Saddle);
		saddle->setOrg(org);
		saddle->setXLen(xLen);
		saddle->setZLen(zLen);
		saddle->setYLen(yLen);
		saddle->setRadius(radius);
		saddle->setColor(CvtColor(color));
		saddle->draw();

		ctGeode->addDrawable(saddle);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadSCylinder()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" height_x, height_y, height_z, "
		L" bottom_normal_x, bottom_normal_y, bottom_normal_z, "
		L" radius, color from scylinder";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 org, height, bottomNormal;
	double radius;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		bottomNormal[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		bottomNormal[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		bottomNormal[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		radius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::SCylinder> scylinder(new Geometry::SCylinder);
		scylinder->setOrg(org);
		scylinder->setHeight(height);
		scylinder->setBottomNormal(bottomNormal);
		scylinder->setRadius(radius);
		scylinder->setColor(CvtColor(color));
		scylinder->draw();

		ctGeode->addDrawable(scylinder);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadSnout()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" height_x, height_y, height_z, "
		L" offset_x, offset_y, offset_z, "
		L" bottom_radius, top_radius, color from snout";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 org, height, offset;
	double bottomRadius, topRadius;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		offset[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		offset[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		bottomRadius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		topRadius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Snout> snout(new Geometry::Snout);
		snout->setOrg(org);
		snout->setHeight(height);
		snout->setOffset(offset);
		snout->setBottomRadius(bottomRadius);
		snout->setTopRadius(topRadius);
		snout->setColor(CvtColor(color));
		snout->draw();

		ctGeode->addDrawable(snout);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadSphere()
{
	const wchar_t *zSql = L"select center_x, center_y, center_z, "
		L" bottom_normal_x, bottom_normal_y, bottom_normal_z, "
		L" radius, angle, color from sphere";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> ctGeode(new osg::Geode);

	osg::Vec3 center, height, bottomNormal;
	double radius, angle;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		};

		int iCol = 0;
		center[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		center[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		bottomNormal[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		bottomNormal[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		bottomNormal[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		radius = sqlite3_column_double(pStmt, iCol);
		++iCol;
		angle = sqlite3_column_double(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Sphere> sphere(new Geometry::Sphere);
		sphere->setCenter(center);
		sphere->setBottomNormal(bottomNormal);
		sphere->setRadius(radius);
		sphere->setAngle(angle);
		sphere->setColor(CvtColor(color));
		sphere->draw();

		ctGeode->addDrawable(sphere);
	}

	m_root->addChild(ctGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadWedge()
{
	const wchar_t *zSql = L"select org_x, org_y, org_z, "
		L" edge1_x, edge1_y, edge1_z, "
		L" edge2_x, edge2_y, edge2_z, "
		L" height_x, height_y, height_z, "
		L" color from wedge";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::ref_ptr<osg::Geode> boxGeode(new osg::Geode);

	osg::Vec3 org, edge1, edge2, height;
	int color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		org[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		org[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		edge1[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		edge1[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		edge1[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		edge2[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		edge2[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		edge2[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		height[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		height[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;

		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::Wedge> wedge(new Geometry::Wedge);
		wedge->setOrg(org);
		wedge->setEdge1(edge1);
		wedge->setEdge2(edge2);
		wedge->setHeight(height);
		wedge->setColor(CvtColor(color));
		wedge->draw();

		boxGeode->addDrawable(wedge);
	}

	m_root->addChild(boxGeode);
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	return true;
}

bool SqliteLoad::loadCombineGeometry()
{
	std::unordered_map<int, osg::ref_ptr<Geometry::CombineGeometry>> cgMap;
	std::unordered_map<int, std::shared_ptr<Geometry::Shell>> shellMap;
	std::unordered_map<int, std::shared_ptr<Geometry::Mesh>> meshMap;
	std::unordered_map<int, std::shared_ptr<Geometry::Polygon>> polygonMap;

	const wchar_t *zCgSql = L"select id, color from combine_geometry";
	sqlite3_stmt *pStmt = NULL;
	const void *pzTail = NULL;
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zCgSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;
	
	int id, color;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		id = sqlite3_column_int(pStmt, iCol);
		++iCol;
		color = sqlite3_column_int(pStmt, iCol);

		osg::ref_ptr<Geometry::CombineGeometry> cg(new Geometry::CombineGeometry);
		cg->setColor(CvtColor(color));
		cgMap.insert(std::make_pair(id, cg));
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	const wchar_t *zShellSql = L"select id, combine_geometry_id from shell";
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zShellSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	int cgId;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		id = sqlite3_column_int(pStmt, iCol);
		++iCol;
		cgId = sqlite3_column_int(pStmt, iCol);

		std::shared_ptr<Geometry::Shell> shell(new Geometry::Shell);
		shellMap.insert(std::make_pair(id, shell));
		auto iter = cgMap.find(cgId);
		if (iter != cgMap.end())
			iter->second->addShell(shell);
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	const wchar_t *zShellFaceSql = L"select vertex_index, shell_id from shell_face order by id asc";
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zShellFaceSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	int vertexIndex, shellId;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		vertexIndex = sqlite3_column_int(pStmt, iCol);
		++iCol;
		shellId = sqlite3_column_int(pStmt, iCol);

		auto iter = shellMap.find(shellId);
		if (iter != shellMap.end())
			iter->second->faces.push_back(vertexIndex);
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	const wchar_t *zShellVertexSql = L"select pos_x, pos_y, pos_z, shell_id from shell_vertex order by id asc";
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zShellVertexSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	osg::Vec3 pos;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		pos[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		pos[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		pos[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		shellId = sqlite3_column_int(pStmt, iCol);

		auto iter = shellMap.find(shellId);
		if (iter != shellMap.end())
			iter->second->vertexs.push_back(pos);
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	const wchar_t *zMeshSql = L"select rows, columns, combine_geometry_id from mesh";
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zShellFaceSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	int rows, columns;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		rows = sqlite3_column_int(pStmt, iCol);
		++iCol;
		columns = sqlite3_column_int(pStmt, iCol);
		++iCol;
		cgId = sqlite3_column_int(pStmt, iCol);

		std::shared_ptr<Geometry::Mesh> mesh(new Geometry::Mesh);
		mesh->rows = rows;
		mesh->colums = columns;
		auto iter = cgMap.find(cgId);
		if (iter != cgMap.end())
			iter->second->addMesh(mesh);
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;


	const wchar_t *zMeshVertexSql = L"select pos_x, pos_y, pos_z, mesh_id from mesh_vertex order by id asc";
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zMeshVertexSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	int meshId;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		pos[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		pos[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		pos[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		meshId = sqlite3_column_int(pStmt, iCol);

		auto iter = meshMap.find(meshId);
		if (iter != meshMap.end())
			iter->second->vertexs.push_back(pos);
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	const wchar_t *zPolygonSql = L"select id, combine_geometry_id from polygon";
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zPolygonSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		id = sqlite3_column_int(pStmt, iCol);
		++iCol;
		cgId = sqlite3_column_int(pStmt, iCol);

		std::shared_ptr<Geometry::Polygon> polygon(new Geometry::Polygon);
		polygonMap.insert(std::make_pair(id, polygon));
		auto iter = cgMap.find(cgId);
		if (iter != cgMap.end())
			iter->second->addPolygon(polygon);
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	const wchar_t *zPolygonVertexSql = L"select pos_x, pos_y, pos_z, polygon_id from polygon_vertex order by id asc";
	if ((m_errorCode = sqlite3_prepare16(m_pDb, zPolygonVertexSql, -1, &pStmt, &pzTail)) != SQLITE_OK)
		return false;

	int polygonId;
	while ((m_errorCode = sqlite3_step(pStmt)) != SQLITE_DONE)
	{
		if (m_errorCode != SQLITE_ROW)
		{
			sqlite3_finalize(pStmt);
			return false;
		}

		int iCol = 0;
		pos[0] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		pos[1] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		pos[2] = sqlite3_column_double(pStmt, iCol);
		++iCol;
		polygonId = sqlite3_column_int(pStmt, iCol);

		auto iter = polygonMap.find(polygonId);
		if (iter != polygonMap.end())
			iter->second->vertexs.push_back(pos);
	}
	m_errorCode = sqlite3_finalize(pStmt);
	pStmt = NULL;

	osg::ref_ptr<osg::Geode> cgGeode(new osg::Geode);
	for each(auto &entry in cgMap)
	{
		entry.second->draw();
		cgGeode->addDrawable(entry.second);
	}
	m_root->addChild(cgGeode);
	return true;
}