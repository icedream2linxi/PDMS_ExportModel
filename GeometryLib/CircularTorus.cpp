#include "stdafx.h"
#include "inc\CircularTorus.h"


namespace Geometry
{

CircularTorus::CircularTorus()
	: m_topVis(true)
	, m_bottomVis(true)
{
}


CircularTorus::~CircularTorus()
{
}

void CircularTorus::subDraw()
{
	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	bool isFull = osg::equivalent(m_angle, 2 * M_PI, GetEpsilon());
	if (isFull)
	{
		m_angle = 2 * M_PI;
	}

	int mainCount = (int)ceil(m_angle / (2 * M_PI / getDivision()));
	double mainIncAngle = m_angle / mainCount;

	int subCount = (int)getDivision();
	double subIncAngle = 2 * M_PI / subCount;

	osg::Vec3 mainVec = m_startPnt - m_center;
	osg::Quat mainQuat(mainIncAngle, m_normal);

	osg::Vec3 torusNormal = mainVec;
	torusNormal.normalize();

	// 第一圈
	osg::Vec3 faceNormal = torusNormal ^ m_normal;
	osg::Quat torusQuat(subIncAngle, -faceNormal);
	osg::Vec3 subVec = torusNormal;
	double subRadius = m_startRadius;
	osg::Vec3 subCenter = m_startPnt;
	osg::Vec3 tmpSubVec = subVec * subRadius;
	for (int i = 0; i < subCount; ++i)
	{
		vertexArr->push_back(subCenter + tmpSubVec);
		tmpSubVec = torusQuat * tmpSubVec;
	}

	// 中间
	osg::DrawElementsUShort *pDrawEle = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, 0);
	pDrawEle->push_back(subCount - 1);
	double factor = (m_endRadius - m_startRadius) / mainCount;
	for (int i = 1; i < mainCount; ++i)
	{
		faceNormal = mainQuat * faceNormal;
		mainVec = mainQuat * mainVec;
		subVec = mainQuat * subVec;
		subCenter = m_center + mainVec;
		torusQuat.makeRotate(subIncAngle, -faceNormal);
		subRadius += factor;

		tmpSubVec = subVec * subRadius;
		osg::Vec3 tangNormal = m_normal;
		for (int j = 0; j < subCount; ++j)
		{
			vertexArr->push_back(subCenter + tmpSubVec);
			tmpSubVec = torusQuat * tmpSubVec;

			int size = vertexArr->size();
			normalArr->push_back(tangNormal ^ ((*vertexArr)[size - subCount - 1] - vertexArr->back()));
			normalArr->back().normalize();
			tangNormal = torusQuat * tangNormal;

			pDrawEle->push_back(size - subCount - 1);
			pDrawEle->push_back(size - 1);
		}
	}

	// 最后一圈
	osg::Quat fullQuat(m_angle, m_normal);
	faceNormal = fullQuat * (torusNormal ^ m_normal);
	mainVec = fullQuat * (m_startPnt - m_center);
	subVec = fullQuat * (torusNormal * m_endRadius);
	subCenter = m_center + mainVec;

	torusQuat.makeRotate(subIncAngle, -faceNormal);
	tmpSubVec = subVec;
	osg::Vec3 tangNormal = m_normal;
	for (int j = 0; j < subCount; ++j)
	{
		vertexArr->push_back(subCenter + tmpSubVec);
		tmpSubVec = torusQuat * tmpSubVec;

		int size = vertexArr->size();
		normalArr->push_back(tangNormal ^ ((*vertexArr)[size - subCount - 1] - vertexArr->back()));
		normalArr->back().normalize();
		tangNormal = torusQuat * tangNormal;

		pDrawEle->push_back(size - subCount - 1);
		pDrawEle->push_back(size - 1);
	}
	pDrawEle->push_back(vertexArr->size() - subCount);
	// 最后一圈法向
	for (int i = 0; i < subCount; ++i)
		normalArr->push_back((*normalArr)[normalArr->size() - subCount]);
	addPrimitiveSet(pDrawEle);

	if (m_topVis)
	{
		size_t first = vertexArr->size();
		size_t base = first - subCount;
		osg::Vec3 topNormal = -faceNormal;
		vertexArr->push_back(subCenter);
		normalArr->push_back(topNormal);
		for (int i = 0; i < subCount; ++i)
		{
			vertexArr->push_back((*vertexArr)[base + i]);
			normalArr->push_back(topNormal);
		}
		vertexArr->push_back((*vertexArr)[vertexArr->size() - subCount]);
		normalArr->push_back(topNormal);
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}

	if (m_bottomVis)
	{
		size_t first = vertexArr->size();
		osg::Vec3 bottomNormal = torusNormal ^ m_normal;
		vertexArr->push_back(m_startPnt);
		normalArr->push_back(bottomNormal);
		for (int i = subCount - 1; i >= 0; --i)
		{
			vertexArr->push_back((*vertexArr)[i]);
			normalArr->push_back(bottomNormal);
		}
		vertexArr->push_back((*vertexArr)[vertexArr->size() - subCount]);
		normalArr->push_back(bottomNormal);
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}
}

} // namespace Geometry