#include "stdafx.h"
#include "inc\Saddle.h"


Saddle::Saddle()
{
}


Saddle::~Saddle()
{
}

void Saddle::subDraw()
{
	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	int count = (int)getDivision();
	double incAng = 2 * M_PI / count;

	osg::Vec3 yNormal = m_zLen ^ m_xLen;
	yNormal.normalize();
	osg::Vec3 yVec = yNormal * m_yLen;
	osg::Vec3 bp1 = m_org - m_xLen / 2.0 - yVec / 2.0;
	osg::Vec3 bp2 = m_org + m_xLen / 2.0 - yVec / 2.0;
	osg::Vec3 bp3 = m_org + m_xLen / 2.0 + yVec / 2.0;
	osg::Vec3 bp4 = m_org - m_xLen / 2.0 + yVec / 2.0;

	osg::Vec3 tp1 = bp1 + m_zLen, tp2 = bp2 + m_zLen, tp3 = bp3 + m_zLen, tp4 = bp4 + m_zLen;

	// bottom
	size_t first = vertexArr->size();
	vertexArr->push_back(bp1);
	vertexArr->push_back(bp2);
	vertexArr->push_back(bp3);
	vertexArr->push_back(bp4);
	osg::Vec3 normal = -m_zLen;
	normal.normalize();
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	// top
	double m_yLen_2 = m_yLen / 2.0;
	osg::Vec3 width = yNormal * (m_yLen_2 - m_radius);
	bool isCircLessThenRect = m_radius < m_yLen_2;
	if (isCircLessThenRect)
	{
		// 1
		vertexArr->push_back(tp1);
		vertexArr->push_back(tp2);
		vertexArr->push_back(tp2 + width);
		vertexArr->push_back(tp1 + width);

		// 2
		vertexArr->push_back(tp4 - width);
		vertexArr->push_back(tp3 - width);
		vertexArr->push_back(tp3);
		vertexArr->push_back(tp4);

		normal = -normal;
		for (int i = 0; i < 8; ++i)
			normalArr->push_back(normal);
	}

	// front
	vertexArr->push_back(tp4);
	vertexArr->push_back(tp3);
	vertexArr->push_back(bp3);
	vertexArr->push_back(bp4);
	normal = yNormal;
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	// back
	vertexArr->push_back(bp1);
	vertexArr->push_back(bp2);
	vertexArr->push_back(tp2);
	vertexArr->push_back(tp1);
	normal = -normal;
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));

	// calc top circ
	double angle = M_PI;
	osg::Vec3 circCenter;
	if (m_radius > m_yLen_2)
	{
		angle = asin(m_yLen_2 / m_radius);
		osg::Vec3 vec = m_zLen;
		vec.normalize();
		vec *= cos(angle) * m_radius;
		circCenter = tp1 + yVec / 2.0 + vec;
		angle *= 2.0;
	}
	else
	{
		circCenter = tp1 + yVec / 2.0;
	}
	count = (int)ceil(angle / incAng);
	incAng = angle / count;

	first = vertexArr->size();
	osg::Vec3 circPnt(tp1);
	if (isCircLessThenRect)
		circPnt = tp1 + width;
	osg::Vec3 circVec = circCenter - circPnt;
	osg::Quat quat(incAng, m_xLen);
	for (int i = 0; i < count + 1; ++i)
	{
		vertexArr->push_back(circCenter - circVec);
		vertexArr->push_back(vertexArr->back() + m_xLen);
		normalArr->push_back(circVec);
		normalArr->back().normalize();
		normalArr->push_back(normalArr->back());

		circVec = quat * circVec;
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

	// left
	size_t leftFirst = vertexArr->size();
	double rectLen = m_yLen;
	osg::Vec3 rectPnt(bp1);
	if (isCircLessThenRect)
	{
		vertexArr->push_back(bp1);
		vertexArr->push_back(tp1);
		rectPnt = bp1 + width;
		rectLen = m_radius * 2.0;
	}
	osg::Vec3 rectVec = yNormal * rectLen / count;
	for (int i = 0; i < count + 1; ++i)
	{
		vertexArr->push_back(rectPnt);
		vertexArr->push_back((*vertexArr)[first + i * 2]);
		rectPnt += rectVec;
	}
	if (isCircLessThenRect)
	{
		vertexArr->push_back(bp4);
		vertexArr->push_back(tp4);
	}
	size_t leftEnd = vertexArr->size();
	normal = -m_xLen;
	normal.normalize();
	for (size_t i = leftFirst; i < leftEnd; ++i)
		normalArr->push_back(normal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, leftFirst, vertexArr->size() - leftFirst));

	// right
	first = vertexArr->size();
	normal = -normal;
	for (size_t i = leftFirst; i < leftEnd; ++i)
	{
		vertexArr->push_back((*vertexArr)[i] + m_xLen);
		normalArr->push_back(normal);
	}

	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));
}