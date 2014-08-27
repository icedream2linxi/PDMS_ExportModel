#include "stdafx.h"
#include "inc\Prism.h"


namespace Geometry
{

Prism::Prism()
	: m_radius(0.0)
{
}


Prism::~Prism()
{
}

void Prism::subDraw()
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

	double incAng = 2.0 * M_PI / m_edgeNum;

	// bottom
	GLint first = vertexArr->size();
	osg::Vec3 vec = m_bottomStartPnt - m_org;
	osg::Vec3 normal = -m_height;
	normal.normalize();
	osg::Quat quat(incAng, normal);
	vertexArr->push_back(m_org);
	for (int i = 0; i < m_edgeNum; ++i)
	{
		vertexArr->push_back(m_org + vec);
		vec = quat * vec;
	}
	vertexArr->push_back((*vertexArr)[first + 1]);
	for (int i = 0; i < m_edgeNum + 2; ++i)
		normalArr->push_back(normal);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));

	// top
	first = vertexArr->size();
	normal = -normal;
	vertexArr->push_back((*vertexArr)[0] + m_height);
	normalArr->push_back(normal);
	for (int i = m_edgeNum + 1; i >= 1; --i)
	{
		vertexArr->push_back((*vertexArr)[i] + m_height);
		normalArr->push_back(normal);
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	size_t topEnd = vertexArr->size();

	first = vertexArr->size();
	for (int i = 0; i < m_edgeNum; ++i)
	{
		vertexArr->push_back((*vertexArr)[topEnd - i - 1]);
		vertexArr->push_back((*vertexArr)[topEnd - i - 2]);
		vertexArr->push_back((*vertexArr)[i + 2]);
		vertexArr->push_back((*vertexArr)[i + 1]);

		normal = m_height ^ ((*vertexArr)[i + 2] - (*vertexArr)[i + 1]);
		normal.normalize();
		for (int j = 0; j < 4; ++j)
			normalArr->push_back(normal);
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));
}

bool Prism::doCullAndUpdate(const osg::CullStack &cullStack)
{
	float psb = cullStack.clampedPixelSize(m_org, m_radius * 2.0);
	float pst = cullStack.clampedPixelSize(m_org + m_height, m_radius * 2.0);
	float ps = osg::maximum(psb, pst);
	if (ps <= cullStack.getSmallFeatureCullingPixelSize())
		return true;
	return false;
}

void Prism::computeAssistVar()
{
	m_radius = (m_bottomStartPnt - m_org).length();
}
} // namespace Geometry