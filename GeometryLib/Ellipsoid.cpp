#include "stdafx.h"
#include "inc\Ellipsoid.h"


namespace Geometry
{

Ellipsoid::Ellipsoid()
	: m_bottomVis(true)
{
}


Ellipsoid::~Ellipsoid()
{
}

void Ellipsoid::subDraw()
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

	osg::Vec3 bottomNormal = -m_aLen;
	bottomNormal.normalize();
	osg::Quat localToWold;
	localToWold.makeRotate(osg::Z_AXIS, -bottomNormal);
	osg::Vec3 xVec = localToWold * osg::X_AXIS;
	osg::Vec3 yVec = xVec ^ bottomNormal;
	int hCount = (int)getDivision();
	double hIncAng = 2 * M_PI / hCount;
	osg::Quat hQuat(hIncAng, -bottomNormal);

	int vCount = (int)ceil(m_angle / hIncAng);
	if (vCount & 1) // 如果是奇数，则变成偶数
		++vCount;
	double vIncAng = m_angle / vCount;

	double currAngle = m_angle / 2.0;
	double b = m_bRadius;
	double a = m_aLen.length();
	osg::Vec3 vec1(b * sin(currAngle), 0, a * cos(currAngle));
	vec1 = localToWold * vec1;
	osg::Vec3 normal1(sin(currAngle) / b, 0, cos(currAngle) / a);
	normal1 = localToWold * normal1;
	normal1.normalize();

	currAngle -= vIncAng;
	osg::Vec3 vec2(b * sin(currAngle), 0, a * cos(currAngle));
	vec2 = localToWold * vec2;
	osg::Vec3 normal2(sin(currAngle) / b, 0, cos(currAngle) / a);
	normal2 = localToWold * normal2;
	normal2.normalize();

	const GLint first = vertexArr->size();
	for (int i = 0; i < vCount / 2; ++i)
	{
		osg::Vec3 hVec1 = vec1;
		osg::Vec3 hVec2 = vec2;
		osg::Vec3 hNormal1 = normal1;
		osg::Vec3 hNormal2 = normal2;
		const size_t hFirst = vertexArr->size();
		for (int j = 0; j < hCount; ++j)
		{
			vertexArr->push_back(m_center + hVec1);
			vertexArr->push_back(m_center + hVec2);
			normalArr->push_back(hNormal1);
			normalArr->push_back(hNormal2);

			hVec1 = hQuat * hVec1;
			hVec2 = hQuat * hVec2;

			hNormal1 = hQuat * hNormal1;
			hNormal2 = hQuat * hNormal2;
		}
		vertexArr->push_back((*vertexArr)[hFirst]);
		vertexArr->push_back((*vertexArr)[hFirst + 1]);
		normalArr->push_back((*normalArr)[hFirst]);
		normalArr->push_back((*normalArr)[hFirst + 1]);

		vec1 = vec2;
		currAngle -= vIncAng;
		vec2.set(b * sin(currAngle), 0, a * cos(currAngle));
		vec2 = localToWold * vec2;

		normal1 = normal2;
		normal2.set(sin(currAngle) / b, 0, cos(currAngle) / a);
		normal2 = localToWold * normal2;
		normal2.normalize();
	}
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

	if (!isFull && m_bottomVis)
	{
		const GLint first = vertexArr->size();
		currAngle = m_angle / 2.0;
		osg::Vec3 vec1(b * sin(currAngle), 0, a * cos(currAngle));
		vec1 = localToWold * vec1;
		osg::Vec3 vec2(0, 0, a * cos(currAngle));
		vec2 = localToWold * vec2;

		vertexArr->push_back(m_center + vec2);
		normalArr->push_back(bottomNormal);
		for (int i = 0; i < hCount; ++i)
		{
			vertexArr->push_back(m_center + vec1);
			normalArr->push_back(bottomNormal);
			vec1 = hQuat * vec1;
		}
		vertexArr->push_back((*vertexArr)[first + 1]);
		normalArr->push_back(bottomNormal);
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
	}
}

} // namespace Geometry