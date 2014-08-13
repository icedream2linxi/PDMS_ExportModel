#include "stdafx.h"
#include "Box.h"

namespace Geometry
{

Box::Box()
{
}


Box::~Box()
{
}

void Box::subDraw()
{
	osg::Vec3 bp1 = m_org;
	osg::Vec3 bp2 = bp1 + m_xLen;
	osg::Vec3 bp3 = bp2 + m_yLen;
	osg::Vec3 bp4 = bp1 + m_yLen;

	osg::Vec3 tp1 = m_org + m_zLen;
	osg::Vec3 tp2 = tp1 + m_xLen;
	osg::Vec3 tp3 = tp2 + m_yLen;
	osg::Vec3 tp4 = tp1 + m_yLen;

	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;

	// bottom
	vertexArr->push_back(bp1);
	vertexArr->push_back(bp4);
	vertexArr->push_back(bp3);
	vertexArr->push_back(bp2);
	osg::Vec3 normal = (bp4 - bp1) ^ (bp3 - bp4);
	normal.normalize();
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	// top
	vertexArr->push_back(tp1);
	vertexArr->push_back(tp2);
	vertexArr->push_back(tp3);
	vertexArr->push_back(tp4);
	normal = -normal;
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	// front
	vertexArr->push_back(bp1);
	vertexArr->push_back(bp2);
	vertexArr->push_back(tp2);
	vertexArr->push_back(tp1);
	normal = (bp2 - bp1) ^ (tp2 - bp2);
	normal.normalize();
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	// back
	vertexArr->push_back(bp3);
	vertexArr->push_back(bp4);
	vertexArr->push_back(tp4);
	vertexArr->push_back(tp3);
	normal = -normal;
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	// left
	vertexArr->push_back(bp4);
	vertexArr->push_back(bp1);
	vertexArr->push_back(tp1);
	vertexArr->push_back(tp4);
	normal = (bp1 - bp4) ^ (tp1 - bp1);
	normal.normalize();
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	// right
	vertexArr->push_back(bp2);
	vertexArr->push_back(bp3);
	vertexArr->push_back(tp3);
	vertexArr->push_back(tp2);
	normal = -normal;
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertexArr->size()));
}

} // namespace Geometry