#pragma once
#include <osg/Geode>

#include <TopoDS_Face.hxx>

osg::Geode* BuildMesh(const TopoDS_Face &face, double deflection = 0.5);
void BuildMesh(osg::Geode *geode, const TopoDS_Face &face, double deflection = 0.5);
void BuildShapeMesh(osg::Geode *geode, const TopoDS_Shape &shape, double deflection = 0.5);
osg::Node* BuildCylinder(DbModel::Cylinder^ cyl);
osg::Geode* BuildCircularTorus(DbModel::CircularTorus^ ct);
osg::Node* BuildSnout(DbModel::Snout^ snout);
osg::Node* BuildDish(DbModel::Dish^ dish);
osg::Node* BuildCone(DbModel::Cone^ cone);