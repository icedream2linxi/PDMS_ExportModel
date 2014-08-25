#include "stdafx.h"
#include "inc\Pyramid.h"


namespace Geometry
{

Pyramid::Pyramid()
{
}


Pyramid::~Pyramid()
{
}

void Pyramid::subDraw()
{
	osg::Vec3 yAxis = m_height ^ m_xAxis;
	yAxis.normalize();
	osg::Vec3 zAxis = m_height / m_height.length();
	osg::Vec3 topOrg = m_org + m_height + m_offset;

	osg::Vec3 bp1 = m_org - m_xAxis * m_bottomXLen / 2.0 - yAxis * m_bottomYLen / 2.0;
	osg::Vec3 bp2 = m_org + m_xAxis * m_bottomXLen / 2.0 - yAxis * m_bottomYLen / 2.0;
	osg::Vec3 bp3 = m_org + m_xAxis * m_bottomXLen / 2.0 + yAxis * m_bottomYLen / 2.0;
	osg::Vec3 bp4 = m_org - m_xAxis * m_bottomXLen / 2.0 + yAxis * m_bottomYLen / 2.0;

	osg::Vec3 tp1 = topOrg - m_xAxis * m_topXLen / 2.0 - yAxis * m_topYLen / 2.0;
	osg::Vec3 tp2 = topOrg + m_xAxis * m_topXLen / 2.0 - yAxis * m_topYLen / 2.0;
	osg::Vec3 tp3 = topOrg + m_xAxis * m_topXLen / 2.0 + yAxis * m_topYLen / 2.0;
	osg::Vec3 tp4 = topOrg - m_xAxis * m_topXLen / 2.0 + yAxis * m_topYLen / 2.0;

	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	vertexArr->push_back(bp4);
	vertexArr->push_back(bp3);
	vertexArr->push_back(bp2);
	vertexArr->push_back(bp1);
	normalArr->push_back(-zAxis);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(tp1);
	vertexArr->push_back(tp2);
	vertexArr->push_back(tp3);
	vertexArr->push_back(tp4);
	normalArr->push_back(zAxis);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(bp1);
	vertexArr->push_back(bp2);
	vertexArr->push_back(tp2);
	vertexArr->push_back(tp1);
	normalArr->push_back((tp2 - bp1) ^ (tp1 - bp2));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(bp2);
	vertexArr->push_back(bp3);
	vertexArr->push_back(tp3);
	vertexArr->push_back(tp2);
	normalArr->push_back((tp3 - bp2) ^ (tp2 - bp3));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(bp3);
	vertexArr->push_back(bp4);
	vertexArr->push_back(tp4);
	vertexArr->push_back(tp3);
	normalArr->push_back((tp4 - bp3) ^ (tp3 - bp4));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(bp4);
	vertexArr->push_back(bp1);
	vertexArr->push_back(tp1);
	vertexArr->push_back(tp4);
	normalArr->push_back((tp1 - bp4) ^ (tp4 - bp1));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertexArr->size()));
}

} // namespace Geometry