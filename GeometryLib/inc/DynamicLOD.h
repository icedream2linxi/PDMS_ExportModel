#pragma once
#include <osg/Group>
#include "BaseGeometry.h"

namespace Geometry
{

class DynamicLOD :
	public osg::Group
{
public:
	DynamicLOD();

	virtual void traverse(osg::NodeVisitor& nv);

private:
	void cullTraverse(osg::NodeVisitor& nv);
	void updateTraverse(osg::NodeVisitor& nv);
};

class DynamicLODUpdateCallback : public osg::NodeCallback
{
public:
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osg::Group *group = node->asGroup();
		if (group != NULL)
		{
			if (typeid(*group) == typeid(DynamicLOD))
				node->traverse(*nv);
			else
			{
				for (unsigned int i = 0; i < group->getNumChildren(); ++i)
					this->operator()(group->getChild(i), nv);
			}
		}
		else
			node->traverse(*nv);
	}
};

class RedrawCallback : public osg::Drawable::UpdateCallback
{
public:
	virtual void update(osg::NodeVisitor *nv, osg::Drawable *drawable)
	{
		BaseGeometry *geo = dynamic_cast<BaseGeometry*>(drawable);
		if (geo->needRedraw())
			geo->draw();
	}
};

} // namespace Geometry

