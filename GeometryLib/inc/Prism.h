#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

// ∂‡¿‚÷˘
class Prism :
	public BaseGeometry
{
public:
	Prism();
	~Prism();

	void setOrg(const osg::Vec3 &val);
	const osg::Vec3 &getOrg() const;
	void setHeight(const osg::Vec3 &val);
	const osg::Vec3 &getHeight() const;
	void setBottomStartPnt(const osg::Vec3 &val);
	const osg::Vec3 &getBottomStartPnt() const;
	void setEdgeNum(const int &val);
	const int &getEdgeNum() const;
	void setColor(const osg::Vec4 &val);
	const osg::Vec4 &getColor() const;

protected:
	virtual void subDraw();
	virtual bool doCullAndUpdate(const osg::CullStack &cullStack);
	void computeAssistVar();

private:
	osg::Vec3 m_org;
	osg::Vec3 m_height;
	osg::Vec3 m_bottomStartPnt;
	int m_edgeNum;
	osg::Vec4 m_color;

	double m_radius;
};



inline void Prism::setOrg(const osg::Vec3 &val)
{
	m_org = val;
}

inline const osg::Vec3 &Prism::getOrg() const
{
	return m_org;
}

inline void Prism::setHeight(const osg::Vec3 &val)
{
	m_height = val;
}

inline const osg::Vec3 &Prism::getHeight() const
{
	return m_height;
}

inline void Prism::setBottomStartPnt(const osg::Vec3 &val)
{
	m_bottomStartPnt = val;
}

inline const osg::Vec3 &Prism::getBottomStartPnt() const
{
	return m_bottomStartPnt;
}

inline void Prism::setEdgeNum(const int &val)
{
	m_edgeNum = val;
}

inline const int &Prism::getEdgeNum() const
{
	return m_edgeNum;
}

inline void Prism::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &Prism::getColor() const
{
	return m_color;
}

} // namespace Geometry