#include "stdafx.h"
#include "inc\Snout.h"


namespace Geometry
{

Snout::Snout()
{
}


Snout::~Snout()
{
}

void Snout::subDraw()
{
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

	osg::Vec3 topCenter = m_org + m_height + m_offset;
	std::vector<osg::Vec3> bottomPntArr, topPntArr;
	for (int i = 0; i < count; ++i)
	{
		bottomPntArr.push_back(m_org + xVec * m_bottomRadius);
		topPntArr.push_back(topCenter + xVec * m_topRadius);

		xVec = quat * xVec;
	}
	bottomPntArr.push_back(bottomPntArr[0]);
	topPntArr.push_back(topPntArr[0]);
	size_t pntCount = bottomPntArr.size();

	if (m_bottomVis)
	{
		const GLint first = vertexArr->size();
		vertexArr->push_back(m_org);
		normalArr->push_back(bottomNormal);
		for (size_t i = 0; i < pntCount; ++i)
		{
			vertexArr->push_back(bottomPntArr[i]);
			normalArr->push_back(bottomNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}

	if (m_topVis)
	{
		const GLint first = vertexArr->size();
		osg::Vec3 topNormal = -bottomNormal;
		vertexArr->push_back(topCenter);
		normalArr->push_back(topNormal);
		for (size_t i = 0; i < pntCount; ++i)
		{
			vertexArr->push_back(topPntArr[i]);
			normalArr->push_back(topNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}

	osg::Vec3 yVec = m_height ^ xVec;
	yVec.normalize();
	const GLint first = vertexArr->size();
	for (size_t i = 0; i < pntCount; ++i)
	{
		vertexArr->push_back(topPntArr[i]);
		vertexArr->push_back(bottomPntArr[i]);

		osg::Vec3 normal = yVec ^ (topPntArr[i] - bottomPntArr[i]);
		normal.normalize();
		normalArr->push_back(normal);
		normalArr->push_back(normal);

		yVec = quat * yVec;
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));
}

} // namespace Geometry