#include "stdafx.h"
#include "Cylinder.h"


namespace Geometry
{

Cylinder::Cylinder()
	: m_bottomVis(true)
	, m_topVis(true)
{
}


Cylinder::~Cylinder()
{
}

void Cylinder::subDraw()
{
	getPrimitiveSetList().clear();

	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);
	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);

	unsigned int count = getDivision();
	double incAng = 2 * M_PI / count;
	osg::Vec3 topNormal = m_height;
	topNormal.normalize();
	osg::Quat quat(incAng, topNormal);

	osg::Quat localToWorld;
	localToWorld.makeRotate(osg::Z_AXIS, topNormal);
	osg::Vec3 normal = localToWorld * osg::X_AXIS;
	normal.normalize();

	osg::Vec3 vertex;
	for (unsigned int i = 0; i < count; ++i)
	{
		vertex = m_org + normal * m_radius;
		vertexArr->push_back(vertex + m_height);
		vertexArr->push_back(vertex);
		normalArr->push_back(normal);
		normalArr->push_back(normal);

		normal = quat * normal;
	}
	vertexArr->push_back((*vertexArr)[0]);
	vertexArr->push_back((*vertexArr)[1]);
	normalArr->push_back((*normalArr)[0]);
	normalArr->push_back((*normalArr)[1]);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, vertexArr->size()));

	if (m_topVis)
	{
		size_t first = vertexArr->size();
		vertexArr->push_back(m_org + m_height);
		normalArr->push_back(topNormal);
		for (unsigned int i = 0; i < count + 1; ++i)
		{
			vertexArr->push_back((*vertexArr)[i * 2]);
			normalArr->push_back(topNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}

	if (m_bottomVis)
	{
		size_t first = vertexArr->size();
		osg::Vec3 bottomNormal = -topNormal;
		vertexArr->push_back(m_org);
		normalArr->push_back(bottomNormal);
		for (int i = count; i >= 0; --i)
		{
			vertexArr->push_back((*vertexArr)[i * 2 + 1]);
			normalArr->push_back(bottomNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}
}

bool Cylinder::cullAndUpdate(const osg::CullStack &cullStack)
{
	double dia = m_radius * 2.0;
	float psb = cullStack.clampedPixelSize(m_org, dia);
	float pst = cullStack.clampedPixelSize(m_org + m_height, dia);
	float ps = osg::maximum(psb, pst);
	if (ps <= cullStack.getSmallFeatureCullingPixelSize())
		return true;

	updateDivision(ps);
	return false;
}
} // namespace Geometry