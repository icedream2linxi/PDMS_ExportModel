#include "stdafx.h"
#include "inc\Pyramid.h"


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

	osg::Vec3 p1 = m_org - m_xAxis * m_bottomXLen / 2.0 - yAxis * m_bottomYLen / 2.0;
	osg::Vec3 p2 = m_org + m_xAxis * m_bottomXLen / 2.0 - yAxis * m_bottomYLen / 2.0;
	osg::Vec3 p3 = m_org + m_xAxis * m_bottomXLen / 2.0 + yAxis * m_bottomYLen / 2.0;
	osg::Vec3 p4 = m_org - m_xAxis * m_bottomXLen / 2.0 + yAxis * m_bottomYLen / 2.0;

	osg::Vec3 p5 = topOrg - m_xAxis * m_topXLen / 2.0 - yAxis * m_topYLen / 2.0;
	osg::Vec3 p6 = topOrg + m_xAxis * m_topXLen / 2.0 - yAxis * m_topYLen / 2.0;
	osg::Vec3 p7 = topOrg + m_xAxis * m_topXLen / 2.0 + yAxis * m_topYLen / 2.0;
	osg::Vec3 p8 = topOrg - m_xAxis * m_topXLen / 2.0 + yAxis * m_topYLen / 2.0;

	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	vertexArr->push_back(p1);
	vertexArr->push_back(p2);
	vertexArr->push_back(p3);
	vertexArr->push_back(p4);
	normalArr->push_back(-zAxis);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(p5);
	vertexArr->push_back(p6);
	vertexArr->push_back(p7);
	vertexArr->push_back(p8);
	normalArr->push_back(zAxis);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(p1);
	vertexArr->push_back(p2);
	vertexArr->push_back(p6);
	vertexArr->push_back(p5);
	normalArr->push_back((p6 - p1) ^ (p5 - p2));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(p2);
	vertexArr->push_back(p3);
	vertexArr->push_back(p7);
	vertexArr->push_back(p6);
	normalArr->push_back((p7 - p2) ^ (p6 - p3));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(p3);
	vertexArr->push_back(p4);
	vertexArr->push_back(p8);
	vertexArr->push_back(p7);
	normalArr->push_back((p8 - p3) ^ (p7 - p4));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	vertexArr->push_back(p4);
	vertexArr->push_back(p1);
	vertexArr->push_back(p5);
	vertexArr->push_back(p8);
	normalArr->push_back((p5 - p4) ^ (p8 - p1));
	normalArr->back().normalize();
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normalArr->back());

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertexArr->size()));
}