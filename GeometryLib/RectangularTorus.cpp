#include "stdafx.h"
#include "inc\RectangularTorus.h"


namespace Geometry
{

RectangularTorus::RectangularTorus()
	: m_topVis(true)
	, m_bottomVis(true)
{
}


RectangularTorus::~RectangularTorus()
{
}

void RectangularTorus::subDraw()
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

	osg::Vec3 mainVec = m_startPnt - m_center;
	osg::Quat mainQuat(mainIncAngle, m_normal);

	osg::Vec3 torusNormal = mainVec;
	torusNormal.normalize();

	// 第一圈
	osg::DrawElementsUShort *pOutSideDrawEle = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, 0);
	osg::DrawElementsUShort *pTopDrawEle = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, 0);
	osg::DrawElementsUShort *pInSideDrawEle = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, 0);
	osg::DrawElementsUShort *pBottomDrawEle = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, 0);
	double width = m_startWidth, height = m_startHeight;
	osg::Vec3 subCenter = m_startPnt;
	osg::Vec3 halfWidthVec = torusNormal * width / 2.0;
	osg::Vec3 halfHeightVec = m_normal * height / 2.0;

	vertexArr->push_back(subCenter - halfWidthVec - halfHeightVec);
	pBottomDrawEle->push_back(vertexArr->size() - 1);

	vertexArr->push_back(vertexArr->back());
	pInSideDrawEle->push_back(vertexArr->size() - 1);

	vertexArr->push_back(subCenter + halfWidthVec - halfHeightVec);
	pBottomDrawEle->push_back(vertexArr->size() - 1);

	vertexArr->push_back(vertexArr->back());
	pOutSideDrawEle->push_back(vertexArr->size() - 1);

	vertexArr->push_back(subCenter + halfWidthVec + halfHeightVec);
	pOutSideDrawEle->push_back(vertexArr->size() - 1);

	vertexArr->push_back(vertexArr->back());
	pTopDrawEle->push_back(vertexArr->size() - 1);

	vertexArr->push_back(subCenter - halfWidthVec + halfHeightVec);
	pTopDrawEle->push_back(vertexArr->size() - 1);

	vertexArr->push_back(vertexArr->back());
	pInSideDrawEle->push_back(vertexArr->size() - 1);

	double widthFactor = (m_endWidth - m_startWidth) / mainCount;
	double heightFactor = (m_endHeight - m_startHeight) / mainCount;
	for (int i = 1; i < mainCount; ++i)
	{
		mainVec = mainQuat * mainVec;
		torusNormal = mainQuat * torusNormal;
		subCenter = m_center + mainVec;

		width += widthFactor;
		height += heightFactor;
		halfWidthVec = torusNormal * width / 2.0;
		halfHeightVec = m_normal * height / 2.0;

		vertexArr->push_back(subCenter - halfWidthVec - halfHeightVec);
		size_t size = vertexArr->size();
		pBottomDrawEle->push_back(size - 1);
		osg::Vec3 bottomNormal = ((*vertexArr)[size - 9] - vertexArr->back()) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 2]);
		bottomNormal.normalize();
		normalArr->push_back(bottomNormal);

		vertexArr->push_back(vertexArr->back());
		size = vertexArr->size();
		pInSideDrawEle->push_back(size - 1);
		osg::Vec3 inSideNormal = (vertexArr->back() - (*vertexArr)[size - 9]) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 6]);
		inSideNormal.normalize();
		normalArr->push_back(inSideNormal);

		vertexArr->push_back(subCenter + halfWidthVec - halfHeightVec);
		size = vertexArr->size();
		pBottomDrawEle->push_back(size - 1);
		normalArr->push_back(bottomNormal);

		vertexArr->push_back(vertexArr->back());
		size = vertexArr->size();
		pOutSideDrawEle->push_back(size - 1);
		osg::Vec3 outSideNormal = ((*vertexArr)[size - 9] - vertexArr->back()) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 1]);
		outSideNormal.normalize();
		normalArr->push_back(outSideNormal);

		vertexArr->push_back(subCenter + halfWidthVec + halfHeightVec);
		size = vertexArr->size();
		pOutSideDrawEle->push_back(size - 1);
		normalArr->push_back(outSideNormal);

		vertexArr->push_back(vertexArr->back());
		size = vertexArr->size();
		pTopDrawEle->push_back(size - 1);
		osg::Vec3 topNormal = ((*vertexArr)[size - 9] - vertexArr->back()) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 1]);
		topNormal.normalize();
		normalArr->push_back(topNormal);

		vertexArr->push_back(subCenter - halfWidthVec + halfHeightVec);
		size = vertexArr->size();
		pTopDrawEle->push_back(size - 1);
		normalArr->push_back(topNormal);

		vertexArr->push_back(vertexArr->back());
		size = vertexArr->size();
		pInSideDrawEle->push_back(size - 1);
		normalArr->push_back(inSideNormal);
	}

	// 最后一圈
	osg::Quat fullQuat(m_angle, m_normal);
	mainVec = fullQuat * (m_startPnt - m_center);
	torusNormal = mainVec;
	torusNormal.normalize();
	subCenter = m_center + mainVec;

	halfWidthVec = torusNormal * m_endWidth / 2.0;
	halfHeightVec = m_normal * m_endHeight / 2.0;

	vertexArr->push_back(subCenter - halfWidthVec - halfHeightVec);
	size_t size = vertexArr->size();
	pBottomDrawEle->push_back(size - 1);
	osg::Vec3 bottomNormal = ((*vertexArr)[size - 9] - vertexArr->back()) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 2]);
	bottomNormal.normalize();
	normalArr->push_back(bottomNormal);

	vertexArr->push_back(vertexArr->back());
	size = vertexArr->size();
	pInSideDrawEle->push_back(size - 1);
	osg::Vec3 inSideNormal = (vertexArr->back() - (*vertexArr)[size - 9]) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 6]);
	inSideNormal.normalize();
	normalArr->push_back(inSideNormal);

	vertexArr->push_back(subCenter + halfWidthVec - halfHeightVec);
	size = vertexArr->size();
	pBottomDrawEle->push_back(size - 1);
	normalArr->push_back(bottomNormal);

	vertexArr->push_back(vertexArr->back());
	size = vertexArr->size();
	pOutSideDrawEle->push_back(size - 1);
	osg::Vec3 outSideNormal = ((*vertexArr)[size - 9] - vertexArr->back()) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 1]);
	outSideNormal.normalize();
	normalArr->push_back(outSideNormal);

	vertexArr->push_back(subCenter + halfWidthVec + halfHeightVec);
	size = vertexArr->size();
	pOutSideDrawEle->push_back(size - 1);
	normalArr->push_back(outSideNormal);

	vertexArr->push_back(vertexArr->back());
	size = vertexArr->size();
	pTopDrawEle->push_back(size - 1);
	osg::Vec3 topNormal = ((*vertexArr)[size - 9] - vertexArr->back()) ^ ((*vertexArr)[size - 9] - (*vertexArr)[size - 9 + 1]);
	topNormal.normalize();
	normalArr->push_back(topNormal);

	vertexArr->push_back(subCenter - halfWidthVec + halfHeightVec);
	size = vertexArr->size();
	pTopDrawEle->push_back(size - 1);
	normalArr->push_back(topNormal);

	vertexArr->push_back(vertexArr->back());
	size = vertexArr->size();
	pInSideDrawEle->push_back(size - 1);
	normalArr->push_back(inSideNormal);

	// 最后一圈法向
	normalArr->push_back(bottomNormal);
	normalArr->push_back(inSideNormal);
	normalArr->push_back(bottomNormal);
	normalArr->push_back(outSideNormal);
	normalArr->push_back(outSideNormal);
	normalArr->push_back(topNormal);
	normalArr->push_back(topNormal);
	normalArr->push_back(inSideNormal);

	addPrimitiveSet(pOutSideDrawEle);
	addPrimitiveSet(pTopDrawEle);
	addPrimitiveSet(pInSideDrawEle);
	addPrimitiveSet(pBottomDrawEle);

	if (m_topVis)
	{
		size_t first = vertexArr->size();
		size_t base = first - 8;
		osg::Vec3 topNormal = m_normal ^ torusNormal;
		topNormal.normalize();
		for (int i = 0; i < 8; i += 2)
		{
			vertexArr->push_back((*vertexArr)[base + i]);
			normalArr->push_back(topNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));
	}

	if (m_bottomVis)
	{
		size_t first = vertexArr->size();
		osg::Vec3 bottomNormal = (m_startPnt - m_center) ^ m_normal;
		bottomNormal.normalize();
		for (int i = 0; i < 8; i += 2)
		{
			vertexArr->push_back((*vertexArr)[i]);
			normalArr->push_back(bottomNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));
	}
}

} // namespace Geometry