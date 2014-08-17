#pragma once
#ifdef __cplusplus_cli
#include <osg/Geode>

#include <TopoDS_Face.hxx>


osg::Geode* BuildMesh(const TopoDS_Face &face, const osg::Vec4 &color, double deflection = 0.5);
void BuildMesh(osg::Geode *geode, const TopoDS_Face &face, const osg::Vec4 &color, double deflection = 0.5);
void BuildShapeMesh(osg::Geode *geode, const TopoDS_Shape &shape, const osg::Vec4 &color, double deflection = 0.5);
osg::Node* BuildCylinder(DbModel::Cylinder^ cyl);
osg::Geode* BuildCircularTorus(DbModel::CircularTorus^ ct);
osg::Node* BuildSnout(DbModel::Snout^ snout);
osg::Node* BuildDish(DbModel::Dish^ dish);
osg::Node* BuildCone(DbModel::Cone^ cone);
osg::Node* BuildRectangularTorus(DbModel::RectangularTorus^ rt);

#endif // __cplusplus_cli

osg::Vec4 CvtColor(int color);