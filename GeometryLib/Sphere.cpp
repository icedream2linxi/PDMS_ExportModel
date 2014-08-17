#include "stdafx.h"
#include "inc\Sphere.h"


namespace Geometry
{

Sphere::Sphere()
	: m_bottomVis(true)
{
}


Sphere::~Sphere()
{
}

void Sphere::subDraw()
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

	osg::Quat quat;
	quat.makeRotate(osg::Z_AXIS, m_bottomNormal);
	osg::Vec3 xVec = quat * osg::X_AXIS;
	osg::Vec3 yVec = xVec ^ m_bottomNormal;
	int hCount = (int)getDivision();
	double hIncAng = 2 * M_PI / hCount;
	osg::Quat hQuat(hIncAng, -m_bottomNormal);

	int vCount = (int)ceil(m_angle / hIncAng);
	if (vCount & 1) // 如果是奇数，则变成偶数
		++vCount;
	double vIncAng = m_angle / vCount;
	osg::Quat vQuat(vIncAng, yVec);

	quat.makeRotate(-m_angle / 2.0, yVec);
	osg::Vec3 vec1 = quat * (-m_bottomNormal) * m_radius;
	osg::Vec3 vec2 = vQuat * vec1;
	if (!isFull && m_bottomVis)
	{
		const GLint first = vertexArr->size();
		osg::Vec3 bVec = vec1;
		osg::Vec3 bottomCenter;
		if (m_angle > M_PI)
		{
			double len = m_radius * sin((m_angle - M_PI) / 2.0);
			bottomCenter = m_center + m_bottomNormal * len;
		}
		else
		{
			double len = m_radius * sin((M_PI - m_angle) / 2.0);
			bottomCenter = m_center - m_bottomNormal * len;
		}
		vertexArr->push_back(bottomCenter);
		normalArr->push_back(m_bottomNormal);
		for (int i = 0; i < hCount; ++i)
		{
			vertexArr->push_back(m_center + bVec);
			normalArr->push_back(m_bottomNormal);
			bVec = hQuat * bVec;
		}
		vertexArr->push_back((*vertexArr)[first + 1]);
		normalArr->push_back(m_bottomNormal);
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}

	const GLint first = vertexArr->size();
	for (int i = 0; i < vCount / 2; ++i)
	{
		osg::Vec3 hVec1 = vec1;
		osg::Vec3 hVec2 = vec2;
		const size_t hFirst = vertexArr->size();
		for (int j = 0; j < hCount; ++j)
		{
			vertexArr->push_back(m_center + hVec1);
			vertexArr->push_back(m_center + hVec2);
			normalArr->push_back(hVec1);
			normalArr->back().normalize();
			normalArr->push_back(hVec2);
			normalArr->back().normalize();

			hVec1 = hQuat * hVec1;
			hVec2 = hQuat * hVec2;
		}
		vertexArr->push_back((*vertexArr)[hFirst]);
		vertexArr->push_back((*vertexArr)[hFirst + 1]);
		normalArr->push_back((*normalArr)[hFirst]);
		normalArr->push_back((*normalArr)[hFirst + 1]);

		vec1 = vec2;
		vec2 = vQuat * vec2;
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));
}

} // namespace Geometry