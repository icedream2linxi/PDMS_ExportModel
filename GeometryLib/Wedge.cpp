#include "stdafx.h"
#include "inc\Wedge.h"


Wedge::Wedge()
{
}


Wedge::~Wedge()
{
}

void Wedge::subDraw()
{
	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	// bottom
	osg::Vec3 bottomNormal = m_edge2 ^ m_edge1;
	bottomNormal.normalize();
	GLint first = vertexArr->size();
	vertexArr->push_back(m_org);
	vertexArr->push_back(m_org + m_edge1);
	vertexArr->push_back(m_org + m_edge2);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(bottomNormal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, first, vertexArr->size() - first));

	// top
	osg::Vec3 topNormal = -bottomNormal;
	first = vertexArr->size();
	vertexArr->push_back(m_org + m_height);
	vertexArr->push_back(m_org + m_edge1 + m_height);
	vertexArr->push_back(m_org + m_edge2 + m_height);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(topNormal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, first, vertexArr->size() - first));

	// face1
	osg::Vec3 normal = m_edge1 ^ m_height;
	normal.normalize();
	first = vertexArr->size();
	vertexArr->push_back((*vertexArr)[0]);
	vertexArr->push_back((*vertexArr)[1]);
	vertexArr->push_back((*vertexArr)[4]);
	vertexArr->push_back((*vertexArr)[3]);
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));

	// face2
	normal = m_height ^ m_edge2;
	normal.normalize();
	first = vertexArr->size();
	vertexArr->push_back((*vertexArr)[2]);
	vertexArr->push_back((*vertexArr)[0]);
	vertexArr->push_back((*vertexArr)[3]);
	vertexArr->push_back((*vertexArr)[5]);
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));

	// face3
	normal = (m_edge2 - m_edge1) ^ m_height;
	normal.normalize();
	first = vertexArr->size();
	vertexArr->push_back((*vertexArr)[1]);
	vertexArr->push_back((*vertexArr)[2]);
	vertexArr->push_back((*vertexArr)[5]);
	vertexArr->push_back((*vertexArr)[4]);
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));
}