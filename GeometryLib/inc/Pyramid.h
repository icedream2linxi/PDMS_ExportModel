#pragma once
#include "BaseGeometry.h"

class Pyramid :
	public BaseGeometry
{
public:
	Pyramid();
	~Pyramid();

	void setOrg(const osg::Vec3 &org);
	const osg::Vec3 &getOrg() const;
	void setHeight(const osg::Vec3 &height);
	const osg::Vec3 &getHeight() const;
	void setXAxis(const osg::Vec3 &xAxis);
	const osg::Vec3 &getXAxis() const;
	void setOffset(const osg::Vec3 &offset);
	const osg::Vec3 &getOffset() const;
	void setBottomXLen(double len);
	double getBottomXLen() const;
	void setBottomYLen(double len);
	double getBottomYLen() const;
	void setTopXLen(double len);
	double getTopXLen() const;
	void setTopYLen(double len);
	double getTopYLen() const;
	void setColor(const osg::Vec4 &color);
	const osg::Vec4 &getColor() const;

protected:
	virtual void subDraw();

private:
	osg::Vec3 m_org;
	osg::Vec3 m_height;
	osg::Vec3 m_xAxis;
	osg::Vec3 m_offset;
	double m_bottomXLen;
	double m_bottomYLen;
	double m_topXLen;
	double m_topYLen;
	osg::Vec4 m_color;
};



inline void Pyramid::setOrg(const osg::Vec3 &org)
{
	m_org = org;
}

inline const osg::Vec3 & Pyramid::getOrg() const
{
	return m_org;
}

inline void Pyramid::setHeight(const osg::Vec3 &height)
{
	m_height = height;
}

inline const osg::Vec3 & Pyramid::getHeight() const
{
	return m_height;
}

inline void Pyramid::setXAxis(const osg::Vec3 &xAxis)
{
	m_xAxis = xAxis;
}

inline const osg::Vec3 &Pyramid::getXAxis() const
{
	return m_xAxis;
}

inline void Pyramid::setOffset(const osg::Vec3 &offset)
{
	m_offset = offset;
}

inline const osg::Vec3 &Pyramid::getOffset() const
{
	return m_offset;
}

inline void Pyramid::setBottomXLen(double len)
{
	m_bottomXLen = len;
}

inline double Pyramid::getBottomXLen() const
{
	return m_bottomXLen;
}

inline void Pyramid::setBottomYLen(double len)
{
	m_bottomYLen = len;
}

inline double Pyramid::getBottomYLen() const
{
	return m_bottomYLen;
}

inline void Pyramid::setTopXLen(double len)
{
	m_topXLen = len;
}

inline double Pyramid::getTopXLen() const
{
	return m_topXLen;
}

inline void Pyramid::setTopYLen(double len)
{
	m_topYLen = len;
}

inline double Pyramid::getTopYLen() const
{
	return m_topYLen;
}

inline void Pyramid::setColor(const osg::Vec4 &color)
{
	m_color = color;
}

inline const osg::Vec4 & Pyramid::getColor() const
{
	return m_color;
}
