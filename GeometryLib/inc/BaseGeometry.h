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

protected:
	virtual void subDraw();
	typedef std::function<float(const osg::Vec3& pos, float radius)> ClampedPixelSizeFun;

protected:
	unsigned int m_division;
};

double GetEpsilon();

} // namespace Geometry
