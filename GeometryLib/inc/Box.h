#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

class Box :
	public BaseGeometry
{
public:
	Box();
	~Box();

	void setOrg(const osg::Vec3 &org);
	const osg::Vec3 &getOrg() const;

	void setXLen(const osg::Vec3 &xLen);
	const osg::Vec3 &getXLen() const;

	void setYLen(const osg::Vec3 &yLen);
	const osg::Vec3 &getYLen() const;

	void setZLen(const osg::Vec3 &zLen);
	const osg::Vec3 &getZLen() const;

	void setColor(const osg::Vec4 &color);
	const osg::Vec4 &getColor() const;

	virtual bool cullAndUpdate(const osg::CullStack &cullStack);

protected:
	virtual void subDraw();
	void computeAssistVar();

private:
	osg::Vec3 m_org;
	osg::Vec3 m_xLen;
	osg::Vec3 m_yLen;
	osg::Vec3 m_zLen;
	osg::Vec4 m_color;

	double m_dblXLen;
	double m_dblYLen;
	double m_dblZLen;
	osg::Vec3 m_center;
};


inline void Box::setOrg(const osg::Vec3 &org)
{
	m_org = org;
}

inline const osg::Vec3 & Box::getOrg() const
{
	return m_org;
}

inline void Box::setXLen(const osg::Vec3 &xLen)
{
	m_xLen = xLen;
}

inline const osg::Vec3 & Box::getXLen() const
{
	return m_xLen;
}

inline void Box::setYLen(const osg::Vec3 &yLen)
{
	m_yLen = yLen;
}

inline const osg::Vec3 & Box::getYLen() const
{
	return m_yLen;
}

inline void Box::setZLen(const osg::Vec3 &zLen)
{
	m_zLen = zLen;
}

inline const osg::Vec3 & Box::getZLen() const
{
	return m_zLen;
}

inline void Box::setColor(const osg::Vec4 &color)
{
	m_color = color;
}

inline const osg::Vec4 & Box::getColor() const
{
	return m_color;
}

} // namespace Geometry

