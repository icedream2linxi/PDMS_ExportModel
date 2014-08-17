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

SqliteLoad::SqliteLoad(osg::ref_ptr<osg::Group> &root, const std::wstring &filePath)
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
	loadBox();

	return true;
}

const wchar_t * SqliteLoad::getErrorMessage() const
{
	return (const wchar_t*)sqlite3_errmsg16(m_pDb);
}

int SqliteLoad::init()
{
	return sqlite3_open16(m_filePath.c_str(), &m_pDb);
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
			return false;

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
		
		boxGeode->addDrawable(box);
	}

	m_root->addChild(boxGeode);

	m_errorCode = SQLITE_OK;
	return true;
}