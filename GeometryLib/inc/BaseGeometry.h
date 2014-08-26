#pragma once
#include <osg/Geometry>
#include <osg/CullStack>
#include <functional>

namespace Geometry
{

class BaseGeometry :
	public osg::Geometry
{
public:
	BaseGeometry();
	virtual ~BaseGeometry();

	void draw();
	unsigned int getDivision();
	virtual bool cullAndUpdate(const osg::CullStack &cullStack);
	bool needRedraw() const;

protected:
	virtual void subDraw();
	void updateDivision(float pixelSize);

protected:
	unsigned int m_division;
	bool m_needRedraw;
};

double GetEpsilon();

inline bool BaseGeometry::needRedraw() const
{
	return m_needRedraw;
}

} // namespace Geometry
