#include "stdafx.h"
#include "inc\DynamicLOD.h"
#include <algorithm>
#include <osg/CullStack>
#include <osg/Geode>
#include "inc\BaseGeometry.h"
using namespace osg;

namespace Geometry
{

//osg::ref_ptr<RedrawCallback> updateCallback(new RedrawCallback);

DynamicLOD::DynamicLOD()
	: m_manipulator(NULL)
{
}

DynamicLOD::DynamicLOD(ViewCenterManipulator *manipulator)
	: m_manipulator(manipulator)
{

}

DynamicLOD::DynamicLOD(const DynamicLOD& lod, const CopyOp& copyop /*= CopyOp::SHALLOW_COPY*/)
	: Group(lod, copyop)
	, m_manipulator(lod.m_manipulator)
{

}

void DynamicLOD::traverse(osg::NodeVisitor& nv)
{
	if (m_manipulator != NULL && !m_manipulator->isMouseRelease())
	{
		if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
			quickTraverse(nv);
		else
			__super::traverse(nv);
		return;
	}

	switch (nv.getVisitorType())
	{
	case osg::NodeVisitor::UPDATE_VISITOR:
		updateTraverse(nv);
		break;
	case osg::NodeVisitor::CULL_VISITOR:
		cullTraverse(nv);
		break;
	default:
		__super::traverse(nv);
		break;
	}
}

void DynamicLOD::cullTraverse(osg::NodeVisitor& nv)
{
	osg::CullStack *cullStack = dynamic_cast<osg::CullStack*>(&nv);
	if (cullStack == NULL)
		return;

	std::for_each(_children.begin(), _children.end(), [&](ref_ptr<Node> &node) {
		if (node->asGroup() != NULL)
			node->asGroup()->traverse(nv);
		else if (node->asGeode() == NULL)
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

void DynamicLOD::updateTraverse(osg::NodeVisitor& nv)
{
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
				if (geo->needRedraw())
					geo->draw();
					//geo->setUpdateCallback(updateCallback);
			}
			node->accept(nv);
		}
	});
}

void DynamicLOD::quickTraverse(osg::NodeVisitor& nv)
{
	std::for_each(_children.begin(), _children.end(), [&](ref_ptr<Node> &node) {
		if (node->asGroup() != NULL)
			node->asGroup()->traverse(nv);
		else if (node->asGeode() == NULL)
			node->accept(nv);
		else
		{
			Geode *geode = node->asGeode();
			for (unsigned int i = 0; i < geode->getNumDrawables(); ++i)
			{
				BaseGeometry *geo = dynamic_cast<BaseGeometry*>(geode->getDrawable(i));
				if (!geo->isCulled())
				{
					node->accept(nv);
					break;
				}
			}
		}
	});
}
} // namespace Geometry