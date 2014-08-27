#pragma once
#include "BaseGeometry.h"

namespace Geometry
{

// ¾ØÐÎÔ²»·
class RectangularTorus :
	public BaseGeometry
{
public:
	RectangularTorus();
	~RectangularTorus();

	void setCenter(const osg::Vec3 &val);
	const osg::Vec3 &getCenter() const;
	void setStartPnt(const osg::Vec3 &val);
	const osg::Vec3 &getStartPnt() const;
	void setNormal(const osg::Vec3 &val);
	const osg::Vec3 &getNormal() const;
	void setStartWidth(const double &val);
	const double &getStartWidth() const;
	void setStartHeight(const double &val);
	const double &getStartHeight() const;
	void setEndWidth(const double &val);
	const double &getEndWidth() const;
	void setEndHeight(const double &val);
	const double &getEndHeight() const;
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
	virtual bool doCullAndUpdate(const osg::CullStack &cullStack);
	void computeAssistVar();

private:
	osg::Vec3 m_center;
	osg::Vec3 m_startPnt;
	osg::Vec3 m_normal;
	double m_startWidth;
	double m_startHeight;
	double m_endWidth;
	double m_endHeight;
	double m_angle;
	osg::Vec4 m_color;
	bool m_topVis;
	bool m_bottomVis;

	double m_radius;
};



inline void RectangularTorus::setCenter(const osg::Vec3 &val)
{
	m_center = val;
}

inline const osg::Vec3 &RectangularTorus::getCenter() const
{
	return m_center;
}

inline void RectangularTorus::setStartPnt(const osg::Vec3 &val)
{
	m_startPnt = val;
}

inline const osg::Vec3 &RectangularTorus::getStartPnt() const
{
	return m_startPnt;
}

inline void RectangularTorus::setNormal(const osg::Vec3 &val)
{
	m_normal = val;
}

inline const osg::Vec3 &RectangularTorus::getNormal() const
{
	return m_normal;
}

inline void RectangularTorus::setStartWidth(const double &val)
{
	m_startWidth = val;
}

inline const double &RectangularTorus::getStartWidth() const
{
	return m_startWidth;
}

inline void RectangularTorus::setStartHeight(const double &val)
{
	m_startHeight = val;
}

inline const double &RectangularTorus::getStartHeight() const
{
	return m_startHeight;
}

inline void RectangularTorus::setEndWidth(const double &val)
{
	m_endWidth = val;
}

inline const double &RectangularTorus::getEndWidth() const
{
	return m_endWidth;
}

inline void RectangularTorus::setEndHeight(const double &val)
{
	m_endHeight = val;
}

inline const double &RectangularTorus::getEndHeight() const
{
	return m_endHeight;
}

inline void RectangularTorus::setAngle(const double &val)
{
	m_angle = val;
}

inline const double &RectangularTorus::getAngle() const
{
	return m_angle;
}

inline void RectangularTorus::setColor(const osg::Vec4 &val)
{
	m_color = val;
}

inline const osg::Vec4 &RectangularTorus::getColor() const
{
	return m_color;
}

inline void RectangularTorus::setTopVis(const bool &val)
{
	m_topVis = val;
}

inline const bool &RectangularTorus::getTopVis() const
{
	return m_topVis;
}

inline void RectangularTorus::setBottomVis(const bool &val)
{
	m_bottomVis = val;
}

inline const bool &RectangularTorus::getBottomVis() const
{
	return m_bottomVis;
}

} // namespace Geometry