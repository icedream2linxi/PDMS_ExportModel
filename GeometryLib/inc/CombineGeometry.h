#pragma once
#include "BaseGeometry.h"
#include <vector>
#include <memory>

namespace Geometry
{

struct Mesh
{
	int rows;
	int colums;
	std::vector<osg::Vec3> vertexs;
};

struct Shell
{
	std::vector<osg::Vec3> vertexs;
	std::vector<int> faces;
};

struct Polygon
{
	std::vector<osg::Vec3> vertexs;
};

// 组合型体元
class CombineGeometry :
	public BaseGeometry
{
public:
	CombineGeometry();
	~CombineGeometry();

	void setMeshs(const std::vector<std::shared_ptr<Mesh>> &val);
	const std::vector<std::shared_ptr<Mesh>> &getMeshs() const;
	void setShells(const std::vector<std::shared_ptr<Shell>> &val);
	const std::vector<std::shared_ptr<Shell>> &getShells() const;
	void setPolygons(const std::vector<std::shared_ptr<Polygon>> &val);
	const std::vector<std::shared_ptr<Polygon>> &getPolygons() const;
	void setColor(const osg::Vec4 &val);
	const osg::Vec4 &getColor() const;

protected:
	virtual void subDraw();

private:
	std::vector<std::shared_ptr<Mesh>> m_meshs;
	std::vector<std::shared_ptr<Shell>> m_shells;
	std::vector<std::shared_ptr<Polygon>> m_polygons;
	osg::Vec4 m_color;
};



inline void CombineGeometry::setMeshs(const std::vector<std::shared_ptr<Mesh>> &val)
{
	m_meshs = val;
}

inline const std::vector<std::shared_ptr<Mesh>> &CombineGeometry::getMeshs() const
{
	return m_meshs;
}

inline void CombineGeometry::setShells(const std::vector<std::shared_ptr<Shell>> &val)
{
	m_shells = val;
}

inline const std::vector<std::shared_ptr<Shell>> &CombineGeometry::getShells() const
{
	return m_shells;
}

inline void CombineGeometry::setPolygons(const std::vector<std::shared_ptr<Polygon>> &val)
{
	m_polygons = val;
}

inline const std::vector<std::shared_ptr<Polygon>> &CombineGeometry::getPolygons() const
{
	return m_polygons;
}

inline void CombineGeometry::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &CombineGeometry::getColor() const
{
	return m_color;
}

} // namespace Geometry