#pragma once
#include <osg/Geometry>

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

protected:
	virtual void subDraw();

protected:
	unsigned int m_division;
};

double GetEpsilon();

} // namespace Geometry
