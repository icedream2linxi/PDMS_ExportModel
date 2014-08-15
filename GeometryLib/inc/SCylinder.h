#pragma once
#include "BaseGeometry.h"

class SCylinder :
	public BaseGeometry
{
public:
	SCylinder();
	~SCylinder();

	void setOrg(const osg::Vec3 &org);
	const osg::Vec3 &getOrg() const;
	void setHeight(const osg::Vec3 &height);
	const osg::Vec3 &getHeight() const;
	void setBottomNormal(const osg::Vec3 &val);
	const osg::Vec3 &getBottomNormal() const;
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

private:
	osg::Vec3 m_org;
	osg::Vec3 m_height;
	osg::Vec3 m_bottomNormal;
	double m_radius;
	osg::Vec4 m_color;

	bool m_bottomVis;
	bool m_topVis;
};



inline void SCylinder::setOrg(const osg::Vec3 &org)
{
	m_org = org;
}

inline const osg::Vec3 & SCylinder::getOrg() const
{
	return m_org;
}

inline void SCylinder::setHeight(const osg::Vec3 &height)
{
	m_height = height;
}

inline const osg::Vec3 & SCylinder::getHeight() const
{
	return m_height;
}

inline void SCylinder::setBottomNormal(const osg::Vec3 &val)
{
	m_bottomNormal = val;
}

inline const osg::Vec3 &SCylinder::getBottomNormal() const
{
	return m_bottomNormal;
}

inline void SCylinder::setRadius(double radius)
{
	m_radius = radius;
}

inline double SCylinder::getRadius() const
{
	return m_radius;
}

inline void SCylinder::setColor(const osg::Vec4 &color)
{
	m_color = color;
}

inline const osg::Vec4 & SCylinder::getColor() const
{
	return m_color;
}

inline void SCylinder::setBottomVisible(bool visible)
{
	m_bottomVis = visible;
}

inline bool SCylinder::isBottomVisible() const
{
	return m_bottomVis;
}

inline void SCylinder::setTopVisible(bool visible)
{
	m_topVis = visible;
}

inline bool SCylinder::isTopVisible() const
{
	return m_topVis;
}