#include "stdafx.h"
#include "SqliteLoad.h"
#include <osg/Geode>

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
	if (!loadBox())
		return false;
	if (!loadCircularTorus())
		return false;

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
	return true;
}