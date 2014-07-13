#include "stdafx.h"
#include "Geometry.hpp"

using namespace osg;

namespace Geometry
{
	const double g_epsilon = 0.00001;
	const double g_deflection = 0.5;

	osg::ref_ptr<osg::Geometry> BuildCircularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
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

		bool isFull = equivalent(angle, 2 * M_PI, g_epsilon);
		if (isFull)
		{
			angle = 2 * M_PI;
		}

		Vec3 mainVec = startPnt - center;
		double mainRadius = mainVec.length() + radius;
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

		if (!isFull && bottomVis)
		{
			Vec3 bottomNormal = -subNormal;
			const GLint first = vertexArr->size();
			vertexArr->push_back(startPnt);
			normalArr->push_back(bottomNormal);
			for (int i = 0; i < subCount; ++i)
			{
				vertexArr->push_back(prevCircArr[i]);
				normalArr->push_back(bottomNormal);
			}
			vertexArr->push_back(prevCircArr[0]);
			normalArr->push_back(bottomNormal);
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
		}

		vector<Vec3> vecArr;
		const size_t prevCircArrCount = prevCircArr.size();
		for (size_t i = 0; i < prevCircArrCount; ++i)
		{
			vecArr.push_back(prevCircArr[i] - center);
		}
		Vec3 prevSubCenterVec = startPnt - center;

		vector<Vec3> postCircArr;
		Vec3 postSubCenterPnt;
		Vec3 topNormal;
		if (isFull)
		{
			postCircArr = prevCircArr;
			postSubCenterPnt = startPnt;
		}
		else
		{
			Quat fullQuat(angle, normal);
			for (size_t i = 0; i < prevCircArrCount; ++i)
			{
				postCircArr.push_back(center + fullQuat * vecArr[i]);
			}
			postSubCenterPnt = center + fullQuat * prevSubCenterVec;
			topNormal = fullQuat * subNormal;
		}

		for (int i = 0; i < mainCount - 1; ++i)
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
				normalArr->push_back(prevCircArr[j] - (center + prevSubCenterVec));
				normalArr->back().normalize();

				vertexArr->push_back(circArr[j]);
				normalArr->push_back(circArr[j] - (center + subCenterVec));
				normalArr->back().normalize();

				vertexArr->push_back(prevCircArr[j + 1]);
				normalArr->push_back(prevCircArr[j + 1] - (center + prevSubCenterVec));
				normalArr->back().normalize();

				vertexArr->push_back(circArr[j + 1]);
				normalArr->push_back(circArr[j + 1] - (center + subCenterVec));
				normalArr->back().normalize();
			}
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

			prevCircArr.swap(circArr);
			prevSubCenterVec = subCenterVec;
		}

		// 最后一环
		const size_t vecArrCount = vecArr.size();
		GLint first = vertexArr->size();
		for (size_t j = 0; j < vecArrCount - 1; ++j)
		{
			vertexArr->push_back(prevCircArr[j]);
			normalArr->push_back(prevCircArr[j] - (center + prevSubCenterVec));
			normalArr->back().normalize();

			vertexArr->push_back(postCircArr[j]);
			normalArr->push_back(postCircArr[j] - postSubCenterPnt);
			normalArr->back().normalize();

			vertexArr->push_back(prevCircArr[j + 1]);
			normalArr->push_back(prevCircArr[j + 1] - (center + prevSubCenterVec));
			normalArr->back().normalize();

			vertexArr->push_back(postCircArr[j + 1]);
			normalArr->push_back(postCircArr[j + 1] - postSubCenterPnt);
			normalArr->back().normalize();
		}
		geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

		if (!isFull && topVis)
		{
			topNormal.normalize();
			const GLint first = vertexArr->size();
			vertexArr->push_back(postSubCenterPnt);
			normalArr->push_back(topNormal);
			for (int i = 0; i < subCount; ++i)
			{
				vertexArr->push_back(postCircArr[i]);
				normalArr->push_back(topNormal);
			}
			vertexArr->push_back(postCircArr[0]);
			normalArr->push_back(topNormal);
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
		}

		return geometry;
	}

	osg::ref_ptr<osg::Geometry> BuildRectangularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double width, double height, double angle, const osg::Vec4 &color, bool topVis /*= true*/, bool bottomVis /*= true*/)
	{
		ref_ptr<osg::Geometry> geometry = new osg::Geometry();
		ref_ptr<Vec3Array> vertexArr = new Vec3Array;
		ref_ptr<Vec3Array> normalArr = new Vec3Array;
		geometry->setVertexArray(vertexArr);
		geometry->setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
		colArr->push_back(color);
		geometry->setColorArray(colArr, osg::Array::BIND_OVERALL);

		bool isFull = equivalent(angle, 2 * M_PI, g_epsilon);
		if (isFull)
		{
			angle = 2 * M_PI;
		}

		Vec3 mainVec = startPnt - center;
		double mainRadius = mainVec.length() + width / 2.0;
		double mainIncAng = 2 * acos((mainRadius - g_deflection) / mainRadius);
		int mainCount = (int)ceil(angle / mainIncAng);
		mainIncAng = angle / mainCount;
		Quat mainQuat(mainIncAng, normal);

		Vec3 subVec = mainVec;
		subVec.normalize();
		Vec3 prevPntArr[4], postPntArr[4];
		prevPntArr[0] = startPnt - subVec * width / 2.0 - normal * height / 2.0;
		prevPntArr[1] = startPnt + subVec * width / 2.0 - normal * height / 2.0;
		prevPntArr[2] = startPnt + subVec * width / 2.0 + normal * height / 2.0;
		prevPntArr[3] = startPnt - subVec * width / 2.0 + normal * height / 2.0;

		Vec3 postOutNormal = subVec, postInNormal = -subVec;
		Vec3 topNormal;
		if (isFull)
		{
			for (int i = 0; i < 4; ++i)
				postPntArr[i] = prevPntArr[i];
		}
		else
		{
			Quat fullQuat(angle, normal);
			for (int i = 0; i < 4; ++i)
				postPntArr[i] = center + fullQuat * (prevPntArr[i] - center);
			postOutNormal = fullQuat * subVec;
			postInNormal = fullQuat * (-subVec);
			topNormal = normal ^ (fullQuat * subVec);
		}

		if (!isFull && bottomVis)
		{
			const GLint first = vertexArr->size();
			Vec3 bottomNormal = subVec ^ normal;
			for (int i = 0; i < 4; ++i)
			{
				vertexArr->push_back(prevPntArr[i]);
				normalArr->push_back(bottomNormal);
			}
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));
		}

		// 底
		Vec3 vec1 = prevPntArr[0] - center;
		Vec3 vec2 = prevPntArr[1] - center;
		Vec3 subNormal = -normal;
		GLint first = vertexArr->size();
		for (int i = 0; i < mainCount; ++i)
		{
			vertexArr->push_back(center + vec1);
			normalArr->push_back(subNormal);
			vec1 = mainQuat * vec1;

			vertexArr->push_back(center + vec2);
			normalArr->push_back(subNormal);
			vec2 = mainQuat * vec2;
		}
		vertexArr->push_back(postPntArr[0]);
		normalArr->push_back(subNormal);

		vertexArr->push_back(postPntArr[1]);
		normalArr->push_back(subNormal);
		geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

		// 外侧
		vec1 = prevPntArr[1] - center;
		vec2 = prevPntArr[2] - center;
		subNormal = subVec;
		first = vertexArr->size();
		for (int i = 0; i < mainCount; ++i)
		{
			vertexArr->push_back(center + vec1);
			normalArr->push_back(subNormal);
			vec1 = mainQuat * vec1;

			vertexArr->push_back(center + vec2);
			normalArr->push_back(subNormal);
			vec2 = mainQuat * vec2;

			subNormal = mainQuat * subNormal;
		}
		vertexArr->push_back(postPntArr[1]);
		normalArr->push_back(postOutNormal);

		vertexArr->push_back(postPntArr[2]);
		normalArr->push_back(postOutNormal);
		geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

		// 订
		vec1 = prevPntArr[2] - center;
		vec2 = prevPntArr[3] - center;
		subNormal = normal;
		first = vertexArr->size();
		for (int i = 0; i < mainCount; ++i)
		{
			vertexArr->push_back(center + vec1);
			normalArr->push_back(subNormal);
			vec1 = mainQuat * vec1;

			vertexArr->push_back(center + vec2);
			normalArr->push_back(subNormal);
			vec2 = mainQuat * vec2;
		}
		vertexArr->push_back(postPntArr[2]);
		normalArr->push_back(subNormal);

		postPntArr[3] = center + vec2;
		vertexArr->push_back(postPntArr[3]);
		normalArr->push_back(subNormal);
		geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

		// 内侧
		vec1 = prevPntArr[3] - center;
		vec2 = prevPntArr[0] - center;
		subNormal = -subVec;
		first = vertexArr->size();
		for (int i = 0; i < mainCount; ++i)
		{
			vertexArr->push_back(center + vec1);
			normalArr->push_back(subNormal);
			vec1 = mainQuat * vec1;

			vertexArr->push_back(center + vec2);
			normalArr->push_back(subNormal);
			vec2 = mainQuat * vec2;

			subNormal = mainQuat * subNormal;
		}
		vertexArr->push_back(postPntArr[3]);
		normalArr->push_back(postInNormal);

		vertexArr->push_back(postPntArr[0]);
		normalArr->push_back(postInNormal);
		geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUAD_STRIP, first, vertexArr->size() - first));

		if (!isFull && topVis)
		{
			const GLint first = vertexArr->size();
			for (int i = 0; i < 4; ++i)
			{
				vertexArr->push_back(postPntArr[i]);
				normalArr->push_back(topNormal);
			}
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::QUADS, first, vertexArr->size() - first));
		}

		return geometry;
	}

	osg::ref_ptr<osg::Geometry> BuildCone(const osg::Vec3 &center, const osg::Vec3 &height, double radius, const Vec4 &color, bool bottomVis /*= true*/)
	{
		osg::Vec3 bottomNormal = -height;
		bottomNormal.normalize();
		return BuildCone(center, bottomNormal, height, radius, color, bottomVis);
	}

	osg::ref_ptr<osg::Geometry> BuildCone(const osg::Vec3 &center, const osg::Vec3 &height, const osg::Vec3 &offset, double radius, const Vec4 &color, bool bottomVis /*= true*/)
	{
		ref_ptr<osg::Geometry> geometry = new osg::Geometry();
		ref_ptr<Vec3Array> vertexArr = new Vec3Array;
		ref_ptr<Vec3Array> normalArr = new Vec3Array;
		geometry->setVertexArray(vertexArr);
		geometry->setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
		colArr->push_back(color);
		geometry->setColorArray(colArr, osg::Array::BIND_OVERALL);

		Vec3 bottomNormal = -height;
		bottomNormal.normalize();
		Vec3 xVec = bottomNormal ^ osg::Z_AXIS;
		if (xVec.length2() < g_epsilon)
			xVec = osg::X_AXIS;
		double incAng = 2 * acos((radius - g_deflection) / radius);
		int count = (int)ceil(2 * M_PI / incAng);
		incAng = 2 * M_PI / count;
		Quat quat(incAng, bottomNormal);

		vector<Vec3> pntArr;
		xVec *= radius;
		for (int i = 0; i < count; ++i)
		{
			pntArr.push_back(center + xVec);
			xVec = quat * xVec;
		}
		size_t pntCount = pntArr.size();

		if (bottomVis)
		{
			const GLint first = vertexArr->size();
			vertexArr->push_back(center);
			normalArr->push_back(bottomNormal);
			for (size_t i = 0; i < pntCount; ++i)
			{
				vertexArr->push_back(pntArr[i]);
				normalArr->push_back(bottomNormal);
			}
			vertexArr->push_back(pntArr[0]);
			normalArr->push_back(bottomNormal);
			geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, first, vertexArr->size() - first));
		}

		const GLint first = vertexArr->size();
		Vec3 topPnt = center + height + offset;
		Vec3 prevPnt = pntArr[pntCount - 1];
		Vec3 prevVec = topPnt - prevPnt;
		prevVec.normalize();
		Vec3 topNormal;
		vector<Vec3> normals;
		size_t testCount = pntCount;
		for (size_t i = 0; i < pntCount; ++i)
		{
			if (i < testCount)
			{
				vertexArr->push_back(pntArr[i]);
				vertexArr->push_back(prevPnt);
				vertexArr->push_back(topPnt);
			}

			Vec3 vec = topPnt - pntArr[i];
			vec.normalize();

			normals.push_back(vec ^ prevVec);
			normals.back().normalize();
			topNormal += normals.back();

			prevPnt = pntArr[i];
			prevVec = vec;
		}

		topNormal /= pntCount / 2.0;
		//topNormal.normalize();
		for (size_t i = 0; i < testCount; ++i)
		{
			if (i == 0)
			{
				normalArr->push_back(normals[i] + normals[i + 1]);
				normalArr->back().normalize();
				normalArr->push_back(normals[i] + normals[normals.size() - 1]);
				normalArr->back().normalize();
				//normalArr->push_back((*normalArr)[normalArr->size() - 1] + (*normalArr)[normalArr->size() - 2]);
				//normalArr->back().normalize();
				normalArr->push_back(topNormal);
			}
			else if (i == normals.size() - 1)
			{
				normalArr->push_back(normals[i] + normals[0]);
				normalArr->back().normalize();
				normalArr->push_back(normals[i] + normals[i - 1]);
				normalArr->back().normalize();
				//normalArr->push_back((*normalArr)[normalArr->size() - 1] + (*normalArr)[normalArr->size() - 2]);
				//normalArr->back().normalize();
				normalArr->push_back(topNormal);
			}
			else
			{
				normalArr->push_back(normals[i] + normals[i + 1]);
				normalArr->back().normalize();
				normalArr->push_back(normals[i] + normals[i - 1]);
				normalArr->back().normalize();
				//normalArr->push_back((*normalArr)[normalArr->size() - 1] + (*normalArr)[normalArr->size() - 2]);
				//normalArr->back().normalize();
				normalArr->push_back(topNormal);
			}
		}
		geometry->addPrimitiveSet(new DrawArrays(osg::PrimitiveSet::TRIANGLES, first, vertexArr->size() - first));

		return geometry;
	}

	osg::ref_ptr<osg::Geometry> BuildCone(const osg::Vec3 &center, const osg::Vec3 &height, const osg::Vec3 &offset,
		double bottomRadius, const double topRadius, const osg::Vec4 &color, bool bottomVis /*= true*/, bool topVis /*= true*/)
	{
		if (equivalent(bottomRadius, 0.0, g_epsilon))
		{
			return BuildCone(center + height + offset, -height, -offset, topRadius, color, topVis);
		}
		else if (equivalent(topRadius, 0.0, g_epsilon))
		{
			return BuildCone(center, height, offset, bottomRadius, color, bottomVis);
		}

		ref_ptr<osg::Geometry> geometry = new osg::Geometry();
		ref_ptr<Vec3Array> vertexArr = new Vec3Array;
		ref_ptr<Vec3Array> normalArr = new Vec3Array;
		geometry->setVertexArray(vertexArr);
		geometry->setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
		osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
		colArr->push_back(color);
		geometry->setColorArray(colArr, osg::Array::BIND_OVERALL);

		Vec3 bottomNormal = -height;
		bottomNormal.normalize();
		Vec3 xVec = bottomNormal ^ osg::Z_AXIS;
		if (xVec.length2() < g_epsilon)
			xVec = osg::X_AXIS;
		double mainRadius = 0.0;
		if (bottomRadius > topRadius)
			mainRadius = bottomRadius;
		else
			mainRadius = topRadius;
		double incAng = 2 * acos((mainRadius - g_deflection) / mainRadius);
		int count = (int)ceil(2 * M_PI / incAng);
		incAng = 2 * M_PI / count;
		Quat quat(incAng, bottomNormal);

		return geometry;
	}

}