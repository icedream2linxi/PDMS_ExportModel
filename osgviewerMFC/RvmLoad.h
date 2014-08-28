#pragma once
#include <osgDB/fstream>
#include <osg/ref_ptr>
#include <ViewCenterManipulator.h>
#include <DynamicLOD.h>

class RvmLoad
{
public:
	RvmLoad(osg::ref_ptr<osg::Group> &root, const std::string &filePath, osg::ref_ptr<ViewCenterManipulator> &mani);
	~RvmLoad();

	void load();

private:
	bool skipLine(size_t num);
	bool loadProject(const std::string &name);
	bool loadSite(const std::string &name);
	bool loadZone(const std::string &name, int siteColor);
	bool loadFacility(const std::string &name, int zoneColor);
	bool loadItem(int parentColor);
	bool loadPrim(int color);

private:
	std::string m_filePath;
	osg::ref_ptr<osg::Group> m_root;
	osg::ref_ptr<ViewCenterManipulator> m_mani;
	osg::ref_ptr<Geometry::DynamicLOD> m_lod;

	osgDB::ifstream m_fin;
};

