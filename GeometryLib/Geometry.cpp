#include "stdafx.h"
#include "Geometry.hpp"

using namespace osg;

namespace Geometry
{
	const double g_deflection = 0.5;

	osg::ref_ptr<osg::Geometry> BuildTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double radius, double angle, const osg::Vec4 &color, bool topVis /*= true*/, bool bottomVis /*= true*/)
	{
		ref_ptr<osg::Geometry> geometry = new osg::Geometry();
		ref_ptr<Vec3Array> vertexArr = new Vec3Array;
		ref_ptr<Vec3Array> normalArr = new Vec3Array;
		geometry->setVertexArray(vertexArr);
		geometry->setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
		colArr->push_back(color);
		geometry->setColorArray(colArr, osg::Array::BIND_OVERALL);

		Vec3 mainVec = startPnt - center;
		double mainRadius = mainVec.length();
		double mainIncAng = 2 * acos((mainRadius - g_deflection) / mainRadius);
		int mainCount = (int)ceil(angle / mainIncAng);
		mainIncAng = angle / mainCount;
		Quat mainQuat(mainIncAng, normal);

		double subIncAng = 2 * acos((radius - g_deflection) / radius);
		int subCount = (int)ceil(2 * M_PI / subIncAng);
		subIncAng = 2 * M_PI / subCount;
		Vec3 subNormal = normal ^ mainVec;
		subNormal.normalize();
		Quat subQuat(subIncAng, subNormal);

		vector<Vec3> prevCircArr;
		Vec3 subVec = mainVec;
		subVec.normalize();
		subVec *= radius;
		for (int i = 0; i < subCount; ++i)
		{
			prevCircArr.push_back(startPnt + subVec);
			subVec = subQuat * subVec;
		}
		prevCircArr.push_back(prevCircArr.front());

		if (bottomVis)
		{
			Vec3 bottomNormal = -subNormal;
			const GLint first = vertexArr->size();
			for (int i = 0; i < subCount; ++i)
			{
				vertexArr->push_back(prevCircArr[i]);
				normalArr->push_back(bottomNormal);
			}
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
		}

		vector<Vec3> vecArr;
		const size_t prevCircArrCount = prevCircArr.size();
		for (size_t i = 0; i < prevCircArrCount; ++i)
		{
			vecArr.push_back(prevCircArr[i] - center);
		}
		Vec3 prevSubCenterVec = startPnt - center;

		for (int i = 0; i < mainCount; ++i)
		{
			vector<Vec3> circArr;
			const size_t vecArrCount = vecArr.size();
			for (size_t j = 0; j < vecArrCount; ++j)
			{
				vecArr[j] = mainQuat * vecArr[j];
				circArr.push_back(center + vecArr[j]);
			}
			Vec3 subCenterVec = mainQuat * prevSubCenterVec;

			GLint first = vertexArr->size();
			for (size_t j = 0; j < vecArrCount - 1; ++j)
			{
				vertexArr->push_back(prevCircArr[j]);
				normalArr->push_back(prevCircArr[j] - prevSubCenterVec);
				normalArr->back().normalize();

				vertexArr->push_back(circArr[j]);
				normalArr->push_back(circArr[j] - subCenterVec);
				normalArr->back().normalize();

				vertexArr->push_back(prevCircArr[j + 1]);
				normalArr->push_back(prevCircArr[j + 1] - prevSubCenterVec);
				normalArr->back().normalize();

				vertexArr->push_back(circArr[j + 1]);
				normalArr->push_back(circArr[j + 1] - subCenterVec);
				normalArr->back().normalize();
			}
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

			prevCircArr.swap(circArr);
			prevSubCenterVec = subCenterVec;
		}

		if (topVis)
		{
			Vec3 topNormal = normal ^ prevSubCenterVec;
			topNormal.normalize();
			const GLint first = vertexArr->size();
			for (int i = 0; i < subCount; ++i)
			{
				vertexArr->push_back(prevCircArr[i]);
				normalArr->push_back(topNormal);
			}
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
		}

		return geometry;
	}

}