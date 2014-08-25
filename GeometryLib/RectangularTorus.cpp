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

	// 计算顶点
	vertexArr->push_back(subCenter - halfWidthVec - halfHeightVec);
	size_t bottomInIdx = vertexArr->size() - 1;
	vertexArr->push_back(vertexArr->back());
	size_t inBottomIdx = vertexArr->size() - 1;

	vertexArr->push_back(subCenter + halfWidthVec - halfHeightVec);
	size_t bottomOutIdx = vertexArr->size() - 1;
	vertexArr->push_back(vertexArr->back());
	size_t outBottomIdx = vertexArr->size() - 1;

	vertexArr->push_back(subCenter + halfWidthVec + halfHeightVec);
	size_t outTopIdx = vertexArr->size() - 1;
	vertexArr->push_back(vertexArr->back());
	size_t topOutIdx = vertexArr->size() - 1;

	vertexArr->push_back(subCenter - halfWidthVec + halfHeightVec);
	size_t topInIdx = vertexArr->size() - 1;
	vertexArr->push_back(vertexArr->back());
	size_t inTopIdx = vertexArr->size() - 1;

	// 构造面片
	pBottomDrawEle->push_back(bottomOutIdx);
	pBottomDrawEle->push_back(bottomInIdx);
	pTopDrawEle->push_back(topInIdx);
	pTopDrawEle->push_back(topOutIdx);
	pInSideDrawEle->push_back(inBottomIdx);
	pInSideDrawEle->push_back(inTopIdx);
	pOutSideDrawEle->push_back(outTopIdx);
	pOutSideDrawEle->push_back(outBottomIdx);

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

		// 计算顶点
		vertexArr->push_back(subCenter - halfWidthVec - halfHeightVec);
		bottomInIdx = vertexArr->size() - 1;
		osg::Vec3 bottomNormal = ((*vertexArr)[bottomInIdx - 8] - vertexArr->back()) ^ ((*vertexArr)[bottomInIdx - 8] - (*vertexArr)[bottomInIdx - 8 + 2]);
		bottomNormal.normalize();
		normalArr->push_back(bottomNormal);

		vertexArr->push_back(vertexArr->back());
		inBottomIdx = vertexArr->size() - 1;
		osg::Vec3 inSideNormal = (vertexArr->back() - (*vertexArr)[inBottomIdx - 8]) ^ ((*vertexArr)[inBottomIdx - 8] - (*vertexArr)[inBottomIdx - 8 + 6]);
		inSideNormal.normalize();
		normalArr->push_back(inSideNormal);

		vertexArr->push_back(subCenter + halfWidthVec - halfHeightVec);
		bottomOutIdx = vertexArr->size() - 1;
		normalArr->push_back(bottomNormal);

		vertexArr->push_back(vertexArr->back());
		outBottomIdx = vertexArr->size() - 1;
		osg::Vec3 outSideNormal = ((*vertexArr)[outBottomIdx - 8] - vertexArr->back()) ^ ((*vertexArr)[outBottomIdx - 8] - (*vertexArr)[outBottomIdx - 8 + 1]);
		outSideNormal.normalize();
		normalArr->push_back(outSideNormal);

		vertexArr->push_back(subCenter + halfWidthVec + halfHeightVec);
		outTopIdx = vertexArr->size() - 1;
		normalArr->push_back(outSideNormal);

		vertexArr->push_back(vertexArr->back());
		topOutIdx = vertexArr->size() - 1;
		osg::Vec3 topNormal = ((*vertexArr)[topOutIdx - 8] - vertexArr->back()) ^ ((*vertexArr)[topOutIdx - 8] - (*vertexArr)[topOutIdx - 8 + 1]);
		topNormal.normalize();
		normalArr->push_back(topNormal);

		vertexArr->push_back(subCenter - halfWidthVec + halfHeightVec);
		topInIdx = vertexArr->size() - 1;
		normalArr->push_back(topNormal);

		vertexArr->push_back(vertexArr->back());
		inTopIdx = vertexArr->size() - 1;
		normalArr->push_back(inSideNormal);

		// 构造面片
		pBottomDrawEle->push_back(bottomOutIdx);
		pBottomDrawEle->push_back(bottomInIdx);
		pTopDrawEle->push_back(topInIdx);
		pTopDrawEle->push_back(topOutIdx);
		pInSideDrawEle->push_back(inBottomIdx);
		pInSideDrawEle->push_back(inTopIdx);
		pOutSideDrawEle->push_back(outTopIdx);
		pOutSideDrawEle->push_back(outBottomIdx);
	}

	// 最后一圈
	osg::Quat fullQuat(m_angle, m_normal);
	mainVec = fullQuat * (m_startPnt - m_center);
	torusNormal = mainVec;
	torusNormal.normalize();
	subCenter = m_center + mainVec;

	halfWidthVec = torusNormal * m_endWidth / 2.0;
	halfHeightVec = m_normal * m_endHeight / 2.0;

	// 计算顶点
	vertexArr->push_back(subCenter - halfWidthVec - halfHeightVec);
	bottomInIdx = vertexArr->size() - 1;
	osg::Vec3 bottomNormal = ((*vertexArr)[bottomInIdx - 8] - vertexArr->back()) ^ ((*vertexArr)[bottomInIdx - 8] - (*vertexArr)[bottomInIdx - 8 + 2]);
	bottomNormal.normalize();
	normalArr->push_back(bottomNormal);

	vertexArr->push_back(vertexArr->back());
	inBottomIdx = vertexArr->size() - 1;
	osg::Vec3 inSideNormal = (vertexArr->back() - (*vertexArr)[inBottomIdx - 8]) ^ ((*vertexArr)[inBottomIdx - 8] - (*vertexArr)[inBottomIdx - 8 + 6]);
	inSideNormal.normalize();
	normalArr->push_back(inSideNormal);

	vertexArr->push_back(subCenter + halfWidthVec - halfHeightVec);
	bottomOutIdx = vertexArr->size() - 1;
	normalArr->push_back(bottomNormal);

	vertexArr->push_back(vertexArr->back());
	outBottomIdx = vertexArr->size() - 1;
	osg::Vec3 outSideNormal = ((*vertexArr)[outBottomIdx - 8] - vertexArr->back()) ^ ((*vertexArr)[outBottomIdx - 8] - (*vertexArr)[outBottomIdx - 8 + 1]);
	outSideNormal.normalize();
	normalArr->push_back(outSideNormal);

	vertexArr->push_back(subCenter + halfWidthVec + halfHeightVec);
	outTopIdx = vertexArr->size() - 1;
	normalArr->push_back(outSideNormal);

	vertexArr->push_back(vertexArr->back());
	topOutIdx = vertexArr->size() - 1;
	osg::Vec3 topNormal = ((*vertexArr)[topOutIdx - 8] - vertexArr->back()) ^ ((*vertexArr)[topOutIdx - 8] - (*vertexArr)[topOutIdx - 8 + 1]);
	topNormal.normalize();
	normalArr->push_back(topNormal);

	vertexArr->push_back(subCenter - halfWidthVec + halfHeightVec);
	topInIdx = vertexArr->size() - 1;
	normalArr->push_back(topNormal);

	vertexArr->push_back(vertexArr->back());
	inTopIdx = vertexArr->size() - 1;
	normalArr->push_back(inSideNormal);

	// 构造面片
	pBottomDrawEle->push_back(bottomOutIdx);
	pBottomDrawEle->push_back(bottomInIdx);
	pTopDrawEle->push_back(topInIdx);
	pTopDrawEle->push_back(topOutIdx);
	pInSideDrawEle->push_back(inBottomIdx);
	pInSideDrawEle->push_back(inTopIdx);
	pOutSideDrawEle->push_back(outTopIdx);
	pOutSideDrawEle->push_back(outBottomIdx);

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
		for (int i = 7; i >= 0; i -= 2)
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