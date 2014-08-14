#pragma once
#include "BaseGeometry.h"

class Ellipsoid :
	public BaseGeometry
{
public:
	Ellipsoid();
	~Ellipsoid();

	void setCenter(const osg::Vec3 &val);
	const osg::Vec3 &getCenter() const;
	void setALen(const osg::Vec3 &val);
	const osg::Vec3 &getALen() const;
	void setBRadius(const double &val);
	const double &getBRadius() const;
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
	osg::Vec3 m_aLen;
	double m_bRadius;
	double m_angle;
	osg::Vec4 m_color;
	bool m_bottomVis;
};


inline void Ellipsoid::setCenter(const osg::Vec3 &val)
{
	m_center = val;
}

inline const osg::Vec3 &Ellipsoid::getCenter() const
{
	return m_center;
}

inline void Ellipsoid::setALen(const osg::Vec3 &val)
{
	m_aLen = val;
}

inline const osg::Vec3 &Ellipsoid::getALen() const
{
	return m_aLen;
}

inline void Ellipsoid::setBRadius(const double &val)
{
	m_bRadius = val;
}

inline const double &Ellipsoid::getBRadius() const
{
	return m_bRadius;
}

inline void Ellipsoid::setAngle(const double &val)
{
	m_angle = val;
}

inline const double &Ellipsoid::getAngle() const
{
	return m_angle;
}

inline void Ellipsoid::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &Ellipsoid::getColor() const
{
	return m_color;
}

inline void Ellipsoid::setBottomVis(const bool &val)
{
	m_bottomVis = val;
}

inline const bool &Ellipsoid::getBottomVis() const
{
	return m_bottomVis;
}