#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

// 圆台
class Snout :
	public BaseGeometry
{
public:
	Snout();
	~Snout();

	void setOrg(const osg::Vec3 &org);
	const osg::Vec3 &getOrg() const;
	void setHeight(const osg::Vec3 &height);
	const osg::Vec3 &getHeight() const;
	void setOffset(const osg::Vec3 &offset);
	const osg::Vec3 &getOffset() const;
	void setBottomRadius(double radius);
	double getBottomRadius() const;
	void setTopRadius(double radius);
	double getTopRadius() const;
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
	osg::Vec3 m_offset;
	double m_bottomRadius;
	double m_topRadius;
	osg::Vec4 m_color;

	bool m_bottomVis;
	bool m_topVis;
};



inline void Snout::setOrg(const osg::Vec3 &org)
{
	m_org = org;
}

inline const osg::Vec3 & Snout::getOrg() const
{
	return m_org;
}

inline void Snout::setHeight(const osg::Vec3 &height)
{
	m_height = height;
}

inline const osg::Vec3 & Snout::getHeight() const
{
	return m_height;
}

inline void Snout::setOffset(const osg::Vec3 &offset)
{
	m_offset = offset;
}

inline const osg::Vec3 &Snout::getOffset() const
{
	return m_offset;
}

inline void Snout::setBottomRadius(double radius)
{
	m_bottomRadius = radius;
}

inline double Snout::getBottomRadius() const
{
	return m_bottomRadius;
}

inline void Snout::setTopRadius(double radius)
{
	m_topRadius = radius;
}

inline double Snout::getTopRadius() const
{
	return m_topRadius;
}

inline void Snout::setColor(const osg::Vec4 &color)
{
	m_color = color;
}

inline const osg::Vec4 & Snout::getColor() const
{
	return m_color;
}

inline void Snout::setBottomVisible(bool visible)
{
	m_bottomVis = visible;
}

inline bool Snout::isBottomVisible() const
{
	return m_bottomVis;
}

inline void Snout::setTopVisible(bool visible)
{
	m_topVis = visible;
}

inline bool Snout::isTopVisible() const
{
	return m_topVis;
}

} // namespace Geometry

