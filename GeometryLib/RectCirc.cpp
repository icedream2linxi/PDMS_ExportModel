#include "stdafx.h"
#include "inc\RectCirc.h"


namespace Geometry
{

RectCirc::RectCirc()
{
}


RectCirc::~RectCirc()
{
}

void RectCirc::subDraw()
{
	computeAssistVar();
	getPrimitiveSetList().clear();

	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	osg::Vec3 topNormal = m_height;
	topNormal.normalize();
	osg::Vec3 yVec = m_height ^ m_xLen;
	yVec.normalize();
	yVec *= m_yLen;
	osg::Vec3 circVec = -yVec;
	circVec.normalize();
	circVec *= m_radius;
	osg::Vec3 circCenter = m_rectCenter + m_height + m_offset;

	int count = (int)getDivision();
	int t = count % 4;
	if (t != 0)
		count += 4 - t;
	double incAng = M_PI * 2.0 / count;
	osg::Quat quat(incAng, topNormal);

	// top
	size_t topFirst = vertexArr->size();
	vertexArr->push_back(circCenter);
	normalArr->push_back(topNormal);
	for (int i = 0; i < count; ++i)
	{
		vertexArr->push_back(circCenter + circVec);
		normalArr->push_back(topNormal);
		circVec = quat * circVec;
	}
	vertexArr->push_back((*vertexArr)[topFirst + 1]);
	normalArr->push_back(topNormal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, topFirst, vertexArr->size() - topFirst));

	// bottom
	size_t first = vertexArr->size();
	osg::Vec3 bp1 = m_rectCenter - m_xLen / 2.0 - yVec / 2.0;
	osg::Vec3 bp2 = m_rectCenter + m_xLen / 2.0 - yVec / 2.0;
	osg::Vec3 bp3 = m_rectCenter + m_xLen / 2.0 + yVec / 2.0;
	osg::Vec3 bp4 = m_rectCenter - m_xLen / 2.0 + yVec / 2.0;
	vertexArr->push_back(bp4);
	vertexArr->push_back(bp3);
	vertexArr->push_back(bp2);
	vertexArr->push_back(bp1);
	osg::Vec3 normal = -topNormal;
	for (int i = 0; i < 4; ++i)
		normalArr->push_back(normal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));

	int count_4 = count >> 2;
	first = vertexArr->size();
	// front
	normal = yVec;
	normal.normalize();
	vertexArr->push_back((*vertexArr)[count_4 * 2 + topFirst + 1]);
	vertexArr->push_back(bp3);
	vertexArr->push_back(bp4);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normal);

	// back
	normal = -normal;
	vertexArr->push_back(bp1);
	vertexArr->push_back(bp2);
	vertexArr->push_back((*vertexArr)[topFirst + 1]);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normal);

	// right
	normal = m_xLen;
	normal.normalize();
	vertexArr->push_back(bp2);
	vertexArr->push_back(bp3);
	vertexArr->push_back((*vertexArr)[count_4 + topFirst + 1]);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normal);

	// left
	normal = -normal;
	vertexArr->push_back(bp4);
	vertexArr->push_back(bp1);
	vertexArr->push_back((*vertexArr)[count_4 * 3 + topFirst + 1]);
	for (int i = 0; i < 3; ++i)
		normalArr->push_back(normal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, first, vertexArr->size() - first));

	// back right
	first = vertexArr->size();
	size_t vertexIdx = topFirst + 1;
	osg::Vec3 vec = m_xLen;
	vec.normalize();
	for (int i = 0; i < count_4 + 1; ++i, ++vertexIdx)
	{
		vertexArr->push_back((*vertexArr)[vertexIdx]);
		vertexArr->push_back(bp2);
		normalArr->push_back((bp2 - (*vertexArr)[vertexIdx]) ^ vec);
		normalArr->back().normalize();
		normalArr->push_back(normalArr->back());

		vec = quat * vec;
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

	// front right
	first = vertexArr->size();
	--vertexIdx;
	vec = yVec;
	vec.normalize();
	for (int i = 0; i < count_4 + 1; ++i, ++vertexIdx)
	{
		vertexArr->push_back((*vertexArr)[vertexIdx]);
		vertexArr->push_back(bp3);
		normalArr->push_back((bp3 - (*vertexArr)[vertexIdx]) ^ vec);
		normalArr->back().normalize();
		normalArr->push_back(normalArr->back());

		vec = quat * vec;
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

	// front left
	first = vertexArr->size();
	--vertexIdx;
	vec = -m_xLen;
	vec.normalize();
	for (int i = 0; i < count_4 + 1; ++i, ++vertexIdx)
	{
		vertexArr->push_back((*vertexArr)[vertexIdx]);
		vertexArr->push_back(bp4);
		normalArr->push_back((bp4 - (*vertexArr)[vertexIdx]) ^ vec);
		normalArr->back().normalize();
		normalArr->push_back(normalArr->back());

		vec = quat * vec;
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

	// back left
	first = vertexArr->size();
	--vertexIdx;
	vec = -yVec;
	vec.normalize();
	for (int i = 0; i < count_4 + 1; ++i, ++vertexIdx)
	{
		vertexArr->push_back((*vertexArr)[vertexIdx]);
		vertexArr->push_back(bp1);
		normalArr->push_back((bp1 - (*vertexArr)[vertexIdx]) ^ vec);
		normalArr->back().normalize();
		normalArr->push_back(normalArr->back());

		vec = quat * vec;
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));
}

bool RectCirc::doCullAndUpdate(const osg::CullStack &cullStack)
{
	float psr = cullStack.clampedPixelSize(m_rectCenter + m_height + m_offset, m_radius * 2.0);
	float psl = cullStack.clampedPixelSize(m_rectCenter, m_assistLen * 2.0);
	float ps = osg::maximum(psr, psl);
	if (ps <= cullStack.getSmallFeatureCullingPixelSize())
		return true;

	updateDivision(psr);
	return false;
}

void RectCirc::computeAssistVar()
{
	osg::Vec3 yVec = m_height ^ m_xLen;
	yVec.normalize();
	yVec *= m_yLen;
	m_assistLen = (m_xLen + yVec).length();
}
} // namespace Geometry