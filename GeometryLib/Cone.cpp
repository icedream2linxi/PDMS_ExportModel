#include "stdafx.h"
#include "inc\Cone.h"


namespace Geometry
{

Cone::Cone()
	: m_bottomVis(true)
{
}


Cone::~Cone()
{
}

void Cone::subDraw()
{
	getPrimitiveSetList().clear();

	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	osg::Vec3 bottomNormal = -m_height;
	bottomNormal.normalize();
	osg::Quat localToWorld;
	localToWorld.makeRotate(-osg::Z_AXIS, bottomNormal);
	osg::Vec3 xVec = localToWorld * osg::X_AXIS;
	
	int count = (int)getDivision();
	double incAng = 2 * M_PI / count;
	osg::Quat quat(incAng, bottomNormal);

	osg::Vec3 yVec = m_height ^ xVec;
	yVec.normalize();
	std::vector<osg::Vec3> pntArr;
	xVec *= m_radius;
	osg::Vec3 topPnt = m_org + m_height;
	const GLint first = vertexArr->size();
	for (int i = 0; i < count; ++i)
	{
		osg::Vec3 pnt = m_org + xVec;
		pntArr.push_back(pnt);

		vertexArr->push_back(pnt);
		vertexArr->push_back(topPnt);

		osg::Vec3 normal = yVec ^ (topPnt - pnt);
		normal.normalize();
		normalArr->push_back(normal);
		normalArr->push_back(normal);

		xVec = quat * xVec;
		yVec = quat * yVec;
	}
	size_t pntCount = pntArr.size();

	vertexArr->push_back(topPnt);
	vertexArr->push_back(pntArr[0]);
	normalArr->push_back((*normalArr)[first]);
	normalArr->push_back((*normalArr)[first]);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

	if (m_bottomVis)
	{
		const GLint first = vertexArr->size();
		vertexArr->push_back(m_org);
		normalArr->push_back(bottomNormal);
		for (size_t i = 0; i < pntCount; ++i)
		{
			vertexArr->push_back(pntArr[i]);
			normalArr->push_back(bottomNormal);
		}
		vertexArr->push_back(pntArr[0]);
		normalArr->push_back(bottomNormal);
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}
}

bool Cone::doCullAndUpdate(const osg::CullStack &cullStack)
{
	float ps = cullStack.clampedPixelSize(m_org, m_radius * 2.0);
	if (ps <= cullStack.getSmallFeatureCullingPixelSize())
		return true;

	updateDivision(ps);
	return false;
}

} // namespace Geometry