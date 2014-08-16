#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

// ÃÏ∑Ωµÿ‘≤
class RectCirc :
	public BaseGeometry
{
public:
	RectCirc();
	~RectCirc();

	void setRectCenter(const osg::Vec3 &val);
	const osg::Vec3 &getRectCenter() const;
	void setXLen(const osg::Vec3 &val);
	const osg::Vec3 &getXLen() const;
	void setYLen(const double &val);
	const double &getYLen() const;
	void setHeight(const osg::Vec3 &val);
	const osg::Vec3 &getHeight() const;
	void setOffset(const osg::Vec3 &val);
	const osg::Vec3 &getOffset() const;
	void setRadius(const double &val);
	const double &getRadius() const;
	void setColor(const osg::Vec4 &val);
	const osg::Vec4 &getColor() const;

protected:
	virtual void subDraw();

private:
	osg::Vec3 m_rectCenter;
	osg::Vec3 m_xLen;
	double m_yLen;
	osg::Vec3 m_height;
	osg::Vec3 m_offset;
	double m_radius;
	osg::Vec4 m_color;
};



inline void RectCirc::setRectCenter(const osg::Vec3 &val)
{
	m_rectCenter = val;
}

inline const osg::Vec3 &RectCirc::getRectCenter() const
{
	return m_rectCenter;
}

inline void RectCirc::setXLen(const osg::Vec3 &val)
{
	m_xLen = val;
}

inline const osg::Vec3 &RectCirc::getXLen() const
{
	return m_xLen;
}

inline void RectCirc::setYLen(const double &val)
{
	m_yLen = val;
}

inline const double &RectCirc::getYLen() const
{
	return m_yLen;
}

inline void RectCirc::setHeight(const osg::Vec3 &val)
{
	m_height = val;
}

inline const osg::Vec3 &RectCirc::getHeight() const
{
	return m_height;
}

inline void RectCirc::setOffset(const osg::Vec3 &val)
{
	m_offset = val;
}

inline const osg::Vec3 &RectCirc::getOffset() const
{
	return m_offset;
}

inline void RectCirc::setRadius(const double &val)
{
	m_radius = val;
}

inline const double &RectCirc::getRadius() const
{
	return m_radius;
}

inline void RectCirc::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &RectCirc::getColor() const
{
	return m_color;
}

} // namespace Geometry