#include "stdafx.h"
#include "inc\DynamicLOD.h"
#include <algorithm>
#include <osg/CullStack>
#include <osg/Geode>
#include "inc\BaseGeometry.h"
using namespace osg;

namespace Geometry
{

DynamicLOD::DynamicLOD()
{
}

void DynamicLOD::traverse(osg::NodeVisitor& nv)
{
	//__super::traverse(nv);
	//return;
	osg::CullStack *cullStack = dynamic_cast<osg::CullStack*>(&nv);
	if (cullStack == NULL)
		return;

	std::for_each(_children.begin(), _children.end(), [&](ref_ptr<Node> &node) {
		if (typeid(*node) == typeid(Group))
			node->asGroup()->traverse(nv);
		else if (typeid(*node) != typeid(Geode))
			node->accept(nv);
		else
		{
			Geode *geode = node->asGeode();
			for (unsigned int i = 0; i < geode->getNumDrawables(); ++i)
			{
				BaseGeometry *geo = dynamic_cast<BaseGeometry*>(geode->getDrawable(i));
				if (!geo->cullAndUpdate(*cullStack))
				{
					node->accept(nv);
					break;
				}
			}
		}
	});
}

} // namespace Geometry