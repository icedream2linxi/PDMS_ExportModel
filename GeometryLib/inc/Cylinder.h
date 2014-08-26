#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

// Ô²Öù
class Cylinder :
	public BaseGeometry
{
public:
	Cylinder();
	~Cylinder();

	void setOrg(const osg::Vec3 &org);
	const osg::Vec3 &getOrg() const;
	void setHeight(const osg::Vec3 &height);
	const osg::Vec3 &getHeight() const;
	void setRadius(double radius);
	double getRadius() const;
	void setColor(const osg::Vec4 &color);
	const osg::Vec4 &getColor() const;
	void setBottomVisible(bool visible);
	bool isBottomVisible() const;
	void setTopVisible(bool visible);
	bool isTopVisible() const;

protected:
	virtual void subDraw();
	virtual bool doCullAndUpdate(const osg::CullStack &cullStack);

private:
	osg::Vec3 m_org;
	osg::Vec3 m_height;
	double m_radius;
	osg::Vec4 m_color;

	bool m_bottomVis;
	bool m_topVis;
};



inline void Cylinder::setOrg(const osg::Vec3 &org)
{
	m_org = org;
}

inline const osg::Vec3 & Cylinder::getOrg() const
{
	return m_org;
}

inline void Cylinder::setHeight(const osg::Vec3 &height)
{
	m_height = height;
}

inline const osg::Vec3 & Cylinder::getHeight() const
{
	return m_height;
}

inline void Cylinder::setRadius(double radius)
{
	m_radius = radius;
}

inline double Cylinder::getRadius() const
{
	return m_radius;
}

inline void Cylinder::setColor(const osg::Vec4 &color)
{
	m_color = color;
}

inline const osg::Vec4 & Cylinder::getColor() const
{
	return m_color;
}

inline void Cylinder::setBottomVisible(bool visible)
{
	m_bottomVis = visible;
}

inline bool Cylinder::isBottomVisible() const
{
	return m_bottomVis;
}

inline void Cylinder::setTopVisible(bool visible)
{
	m_topVis = visible;
}

inline bool Cylinder::isTopVisible() const
{
	return m_topVis;
}

} // namespace Geometry
