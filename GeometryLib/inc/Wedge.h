#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

// п╗пн 
class Wedge :
	public BaseGeometry
{
public:
	Wedge();
	~Wedge();

	void setOrg(const osg::Vec3 &val);
	const osg::Vec3 &getOrg() const;
	void setEdge1(const osg::Vec3 &val);
	const osg::Vec3 &getEdge1() const;
	void setEdge2(const osg::Vec3 &val);
	const osg::Vec3 &getEdge2() const;
	void setHeight(const osg::Vec3 &val);
	const osg::Vec3 &getHeight() const;
	void setColor(const osg::Vec4 &val);
	const osg::Vec4 &getColor() const;

protected:
	virtual void subDraw();

private:
	osg::Vec3 m_org;
	osg::Vec3 m_edge1;
	osg::Vec3 m_edge2;
	osg::Vec3 m_height;
	osg::Vec4 m_color;
};


inline void Wedge::setOrg(const osg::Vec3 &val)
{
	m_org = val;
}

inline const osg::Vec3 &Wedge::getOrg() const
{
	return m_org;
}

inline void Wedge::setEdge1(const osg::Vec3 &val)
{
	m_edge1 = val;
}

inline const osg::Vec3 &Wedge::getEdge1() const
{
	return m_edge1;
}

inline void Wedge::setEdge2(const osg::Vec3 &val)
{
	m_edge2 = val;
}

inline const osg::Vec3 &Wedge::getEdge2() const
{
	return m_edge2;
}

inline void Wedge::setHeight(const osg::Vec3 &val)
{
	m_height = val;
}

inline const osg::Vec3 &Wedge::getHeight() const
{
	return m_height;
}

inline void Wedge::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &Wedge::getColor() const
{
	return m_color;
}

} // namespace Geometry