#pragma once
#include "BaseGeometry.h"

// Âí°°ÐÎ
class Saddle :
	public BaseGeometry
{
public:
	Saddle();
	~Saddle();

	void setOrg(const osg::Vec3 &val);
	const osg::Vec3 &getOrg() const;
	void setXLen(const osg::Vec3 &val);
	const osg::Vec3 &getXLen() const;
	void setYLen(const double &val);
	const double &getYLen() const;
	void setZLen(const osg::Vec3 &val);
	const osg::Vec3 &getZLen() const;
	void setRadius(const double &val);
	const double &getRadius() const;
	void setColor(const osg::Vec4 &val);
	const osg::Vec4 &getColor() const;

protected:
	virtual void subDraw();

private:
	osg::Vec3 m_org;
	osg::Vec3 m_xLen;
	double m_yLen;
	osg::Vec3 m_zLen;
	double m_radius;
	osg::Vec4 m_color;
};



inline void Saddle::setOrg(const osg::Vec3 &val)
{
	m_org = val;
}

inline const osg::Vec3 &Saddle::getOrg() const
{
	return m_org;
}

inline void Saddle::setXLen(const osg::Vec3 &val)
{
	m_xLen = val;
}

inline const osg::Vec3 &Saddle::getXLen() const
{
	return m_xLen;
}

inline void Saddle::setYLen(const double &val)
{
	m_yLen = val;
}

inline const double &Saddle::getYLen() const
{
	return m_yLen;
}

inline void Saddle::setZLen(const osg::Vec3 &val)
{
	m_zLen = val;
}

inline const osg::Vec3 &Saddle::getZLen() const
{
	return m_zLen;
}

inline void Saddle::setRadius(const double &val)
{
	m_radius = val;
}

inline const double &Saddle::getRadius() const
{
	return m_radius;
}

inline void Saddle::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &Saddle::getColor() const
{
	return m_color;
}