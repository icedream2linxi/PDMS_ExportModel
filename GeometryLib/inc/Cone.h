#pragma once
#include "BaseGeometry.h"

// Բ׶
class Cone :
	public BaseGeometry
{
public:
	Cone();
	~Cone();

	void setOrg(const osg::Vec3 &org);
	const osg::Vec3 &getOrg() const;
	void setHeight(const osg::Vec3 &height);
	const osg::Vec3 &getHeight() const;
	void setOffset(const osg::Vec3 &offset);
	const osg::Vec3 &getOffset() const;
	void setRadius(double radius);
	double getRadius() const;
	void setColor(const osg::Vec4 &color);
	const osg::Vec4 &getColor() const;
	void setBottomVisible(bool visible);
	bool isBottomVisible() const;

protected:
	virtual void subDraw();

private:
	osg::Vec3 m_org;
	osg::Vec3 m_height;
	osg::Vec3 m_offset;
	double m_radius;
	osg::Vec4 m_color;

	bool m_bottomVis;
};




inline void Cone::setOrg(const osg::Vec3 &org)
{
	m_org = org;
}

inline const osg::Vec3 & Cone::getOrg() const
{
	return m_org;
}

inline void Cone::setHeight(const osg::Vec3 &height)
{
	m_height = height;
}

inline const osg::Vec3 & Cone::getHeight() const
{
	return m_height;
}

inline void Cone::setOffset(const osg::Vec3 &offset)
{
	m_offset = offset;
}

inline const osg::Vec3 &Cone::getOffset() const
{
	return m_offset;
}

inline void Cone::setRadius(double radius)
{
	m_radius = radius;
}

inline double Cone::getRadius() const
{
	return m_radius;
}

inline void Cone::setColor(const osg::Vec4 &color)
{
	m_color = color;
}

inline const osg::Vec4 & Cone::getColor() const
{
	return m_color;
}

inline void Cone::setBottomVisible(bool visible)
{
	m_bottomVis = visible;
}

inline bool Cone::isBottomVisible() const
{
	return m_bottomVis;
}
