#pragma once
#include <osg/Geometry>
#include <osg/CullStack>
#include <functional>

namespace Geometry
{

extern const int g_defaultDivision;

class BaseGeometry :
	public osg::Geometry
{
public:
	BaseGeometry();
	virtual ~BaseGeometry();

	void draw();
	unsigned int getDivision();
	bool needRedraw() const;
	bool cullAndUpdate(const osg::CullStack &cullStack);
	bool isCulled() const;

protected:
	virtual void subDraw();
	virtual bool doCullAndUpdate(const osg::CullStack &cullStack);
	void updateDivision(float pixelSize);
	int computeDivision(float pixelSize);

protected:
	unsigned int m_division;
	bool m_needRedraw;
	bool m_isCulled;
};

double GetEpsilon();

inline bool BaseGeometry::needRedraw() const
{
	return m_needRedraw;
}

inline bool BaseGeometry::isCulled() const
{
	return m_isCulled;
}

} // namespace Geometry
