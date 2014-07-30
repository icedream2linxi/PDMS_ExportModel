#pragma once
#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Geode>

namespace Geometry
{
	osg::ref_ptr<osg::Geometry> BuildCircularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double radius, double angle, const osg::Vec4 &color, bool topVis = true, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildRectangularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double width, double height, double angle, const osg::Vec4 &color, bool topVis = true, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildCone(const osg::Vec3 &center, const osg::Vec3 &height, double radius, const osg::Vec4 &color, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildCone(const osg::Vec3 &center, const osg::Vec3 &height, const osg::Vec3 &offset, double radius, const osg::Vec4 &color, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildSnout(const osg::Vec3 &center, const osg::Vec3 &height, const osg::Vec3 &offset,
		double bottomRadius, const double topRadius, const osg::Vec4 &color, bool bottomVis = true, bool topVis = true);
	osg::ref_ptr<osg::Geometry> BuildPyramid(const osg::Vec3 &org, const osg::Vec3 &height, const osg::Vec3 &xAxis, const osg::Vec3 &offset,
		double bottomXLen, double bottomYLen, double topXLen, double topYLen, const osg::Vec4 &color);
	osg::ref_ptr<osg::Geometry> BuildSphere(const osg::Vec3 &center, const osg::Vec3 &height,
		double bottomRadius, const osg::Vec4 &color, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildSphere(const osg::Vec3 &center, const osg::Vec3 &bottomNormal,
		double sphereRadius, double angle, const osg::Vec4 &color, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildEllipsoid(const osg::Vec3 &center, const osg::Vec3 &height,
		double radius, const osg::Vec4 &color, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildWedge(const osg::Vec3 &org, const osg::Vec3 &edge1,
		const osg::Vec3 &edge2, const osg::Vec3 &height, const osg::Vec4 &color);
	osg::ref_ptr<osg::Geometry> BuildPrism(const osg::Vec3 &org, const osg::Vec3 &height,
		const osg::Vec3 &bottomStartPnt, int edgeNum, const osg::Vec4 &color);
}