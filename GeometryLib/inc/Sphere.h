#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

// Ô²Çò
class Sphere :
	public BaseGeometry
{
public:
	Sphere();
	~Sphere();

	void setCenter(const osg::Vec3 &val);
	const osg::Vec3 &getCenter() const;
	void setBottomNormal(const osg::Vec3 &val);
	const osg::Vec3 &getBottomNormal() const;
	void setRadius(const double &val);
	const double &getRadius() const;
	void setAngle(const double &val);
	const double &getAngle() const;
	void setColor(const osg::Vec4 &val);
	const osg::Vec4 &getColor() const;
	void setBottomVis(const bool &val);
	const bool &getBottomVis() const;

protected:
	virtual void subDraw();

private:
	osg::Vec3 m_center;
	osg::Vec3 m_bottomNormal;
	double m_radius;
	double m_angle;
	osg::Vec4 m_color;
	bool m_bottomVis;
};


inline void Sphere::setCenter(const osg::Vec3 &val)
{
	m_center = val;
}

inline const osg::Vec3 &Sphere::getCenter() const
{
	return m_center;
}

inline void Sphere::setBottomNormal(const osg::Vec3 &val)
{
	m_bottomNormal = val;
}

inline const osg::Vec3 &Sphere::getBottomNormal() const
{
	return m_bottomNormal;
}

inline void Sphere::setRadius(const double &val)
{
	m_radius = val;
}

inline const double &Sphere::getRadius() const
{
	return m_radius;
}

inline void Sphere::setAngle(const double &val)
{
	m_angle = val;
}

inline const double &Sphere::getAngle() const
{
	return m_angle;
}

inline void Sphere::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &Sphere::getColor() const
{
	return m_color;
}

inline void Sphere::setBottomVis(const bool &val)
{
	m_bottomVis = val;
}

inline const bool &Sphere::getBottomVis() const
{
	return m_bottomVis;
}

} // namespace Geometry