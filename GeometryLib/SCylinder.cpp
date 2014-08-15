#include "stdafx.h"
#include "inc\SCylinder.h"


SCylinder::SCylinder()
{
}


SCylinder::~SCylinder()
{
}

void SCylinder::subDraw()
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
	osg::Vec3 topNormal = m_height;
	topNormal.normalize();
	double angleCos = m_bottomNormal * (-topNormal) / m_bottomNormal.length() / topNormal.length();
	double angle = acos(angleCos);
	double a = m_radius;
	double b = m_radius / sin(M_PI_2 - angle);
	osg::Vec3 vec = m_bottomNormal ^ topNormal;
	vec.normalize();
	osg::Quat bottomQuat(incAng, m_bottomNormal);
	double currAngle = 0;
	osg::Quat localToWorldQuat;
	localToWorldQuat.makeRotate(osg::Z_AXIS, m_bottomNormal);
	osg::Vec3 yAxis = localToWorldQuat * osg::Y_AXIS;
	osg::Quat localToWorldQuat2;
	localToWorldQuat2.makeRotate(yAxis, vec);
	localToWorldQuat *= localToWorldQuat2;
	osg::Vec3 bottomVec(b * sin(currAngle), a * cos(currAngle), 0);
	bottomVec = localToWorldQuat * bottomVec;
	//bottomVec = localToWorldQuat2 * bottomVec;

	osg::Quat topQuat(incAng, topNormal);
	osg::Vec3 topVec = vec * m_radius;
	osg::Vec3 topCenter = m_org + m_height;
	size_t first = vertexArr->size();
	for (int i = 0; i < count; ++i)
	{
		vertexArr->push_back(topCenter + topVec);
		vertexArr->push_back(m_org + bottomVec);
		normalArr->push_back(topVec);
		normalArr->back().normalize();
		normalArr->push_back(normalArr->back());

		currAngle += incAng;
		bottomVec.set(b * sin(currAngle), a * cos(currAngle), 0);
		bottomVec = localToWorldQuat * bottomVec;
		//bottomVec = localToWorldQuat2 * bottomVec;
		topVec = topQuat * topVec;
	}
	vertexArr->push_back((*vertexArr)[first]);
	vertexArr->push_back((*vertexArr)[first + 1]);
	normalArr->push_back((*normalArr)[first]);
	normalArr->push_back((*normalArr)[first + 1]);
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, first, vertexArr->size() - first));

	if (m_bottomVis)
	{
		first = vertexArr->size();
		vertexArr->push_back(m_org);
		normalArr->push_back(m_bottomNormal);
		for (int i = 0; i < count + 1; ++i)
		{
			vertexArr->push_back((*vertexArr)[i * 2 + 1]);
			normalArr->push_back(m_bottomNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}

	if (m_topVis)
	{
		first = vertexArr->size();
		vertexArr->push_back(topCenter);
		normalArr->push_back(topNormal);
		for (int i = 0; i < count + 1; ++i)
		{
			vertexArr->push_back((*vertexArr)[i * 2]);
			normalArr->push_back(topNormal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}
}