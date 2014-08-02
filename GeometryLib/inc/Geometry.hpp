#pragma once
#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/Geode>

namespace Geometry
{
	osg::ref_ptr<osg::Geometry> BuildCircularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double radius, double angle, const osg::Vec4 &color, bool topVis = true, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildCircularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double startRadius,  double endRadius, double angle, const osg::Vec4 &color, bool topVis = true, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildRectangularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double width, double height, double angle, const osg::Vec4 &color, bool topVis = true, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildRectangularTorus(const osg::Vec3 &center, const osg::Vec3 &startPnt, const osg::Vec3 &normal,
		double startWidth, double startHeight, double endWidth, double endHeight, double angle, const osg::Vec4 &color,
		bool topVis = true, bool bottomVis = true);
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
	osg::ref_ptr<osg::Geometry> BuildEllipsoid(const osg::Vec3 &center, const osg::Vec3 &aLen,
		double bRadius, double angle, const osg::Vec4 &color, bool bottomVis = true);
	osg::ref_ptr<osg::Geometry> BuildWedge(const osg::Vec3 &org, const osg::Vec3 &edge1,
		const osg::Vec3 &edge2, const osg::Vec3 &height, const osg::Vec4 &color);
	osg::ref_ptr<osg::Geometry> BuildPrism(const osg::Vec3 &org, const osg::Vec3 &height,
		const osg::Vec3 &bottomStartPnt, int edgeNum, const osg::Vec4 &color);
	osg::ref_ptr<osg::Geometry> BuildSCylinder(const osg::Vec3 &org, const osg::Vec3 &height, const osg::Vec3 &bottomNormal,
		double radius, const osg::Vec4 &color, bool bottomVis = true, bool topVis = true);
	osg::ref_ptr<osg::Geometry> BuildSaddle(const osg::Vec3 &org, const osg::Vec3 &xLen, double yLen, const osg::Vec3 &zLen,
		double radius, const osg::Vec4 &color);
	osg::ref_ptr<osg::Geometry> BuildRectCirc(const osg::Vec3 &rectCenter, const osg::Vec3 &xLen, double yLen, const osg::Vec3 &height,
		const osg::Vec3 &offset, double radius, const osg::Vec4 &color);
}