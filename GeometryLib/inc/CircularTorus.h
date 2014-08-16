#pragma once
#include "BaseGeometry.h"

// Ô²»·
class CircularTorus :
	public BaseGeometry
{
public:
	CircularTorus();
	~CircularTorus();

	void setCenter(const osg::Vec3 &val);
	const osg::Vec3 &getCenter() const;
	void setStartPnt(const osg::Vec3 &val);
	const osg::Vec3 &getStartPnt() const;
	void setNormal(const osg::Vec3 &val);
	const osg::Vec3 &getNormal() const;
	void setStartRadius(const double &val);
	const double &getStartRadius() const;
	void setEndRadius(const double &val);
	const double &getEndRadius() const;
	void setAngle(const double &val);
	const double &getAngle() const;
	void setColor(const osg::Vec4 &val);
	const osg::Vec4 &getColor() const;
	void setTopVis(const bool &val);
	const bool &getTopVis() const;
	void setBottomVis(const bool &val);
	const bool &getBottomVis() const;
	
protected:
	virtual void subDraw();

private:
	osg::Vec3 m_center;
	osg::Vec3 m_startPnt;
	osg::Vec3 m_normal;
	double m_startRadius;
	double m_endRadius;
	double m_angle;
	osg::Vec4 m_color;
	bool m_topVis;
	bool m_bottomVis;
};



inline void CircularTorus::setCenter(const osg::Vec3 &val)
{
	m_center = val;
}

inline const osg::Vec3 &CircularTorus::getCenter() const
{
	return m_center;
}

inline void CircularTorus::setStartPnt(const osg::Vec3 &val)
{
	m_startPnt = val;
}

inline const osg::Vec3 &CircularTorus::getStartPnt() const
{
	return m_startPnt;
}

inline void CircularTorus::setNormal(const osg::Vec3 &val)
{
	m_normal = val;
}

inline const osg::Vec3 &CircularTorus::getNormal() const
{
	return m_normal;
}

inline void CircularTorus::setStartRadius(const double &val)
{
	m_startRadius = val;
}

inline const double &CircularTorus::getStartRadius() const
{
	return m_startRadius;
}

inline void CircularTorus::setEndRadius(const double &val)
{
	m_endRadius = val;
}

inline const double &CircularTorus::getEndRadius() const
{
	return m_endRadius;
}

inline void CircularTorus::setAngle(const double &val)
{
	m_angle = val;
}

inline const double &CircularTorus::getAngle() const
{
	return m_angle;
}

inline void CircularTorus::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &CircularTorus::getColor() const
{
	return m_color;
}

inline void CircularTorus::setTopVis(const bool &val)
{
	m_topVis = val;
}

inline const bool &CircularTorus::getTopVis() const
{
	return m_topVis;
}

inline void CircularTorus::setBottomVis(const bool &val)
{
	m_bottomVis = val;
}

inline const bool &CircularTorus::getBottomVis() const
{
	return m_bottomVis;
}