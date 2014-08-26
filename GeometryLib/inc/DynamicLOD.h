#pragma once
#include <osg/Group>

namespace Geometry
{

class DynamicLOD :
	public osg::Group
{
public:
	DynamicLOD();

	virtual void traverse(osg::NodeVisitor& nv);
};

} // namespace Geometry

