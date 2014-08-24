#pragma once
#include <string>
#include <osg/ref_ptr>
#include <osg/Group>
#include "sqlite3.h"

class SqliteLoad
{
public:
	SqliteLoad(osg::ref_ptr<osg::Group> &root, const std::string &filePath);
	~SqliteLoad();
	bool doLoad();
	const char *getErrorMessage() const;

private:
	int init();
	bool loadBox();
	bool loadCircularTorus();
	bool loadCone();
	bool loadCylinder();
	bool loadEllipsoid();
	bool loadPrism();
	bool loadPyramid();
	bool loadRectCirc();
	bool loadRectangularTorus();
	bool loadSaddle();
	bool loadSCylinder();
	bool loadSnout();
	bool loadSphere();
	bool loadWedge();

private:
	std::string m_filePath;
	osg::ref_ptr<osg::Group> m_root;

	sqlite3 *m_pDb;
	int m_errorCode;
};