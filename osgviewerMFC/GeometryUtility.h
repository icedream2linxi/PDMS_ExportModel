#pragma once
#include <osg/Geode>

#include <TopoDS_Face.hxx>

osg::Geode* BuildMesh(const TopoDS_Face &face, double deflection = 0.5);
osg::Geode* BuildCylinder(DbModel::Cylinder^ cyl);
osg::Geode* BuildCircularTorus(DbModel::CircularTorus^ ct);
osg::Group* BuildSnout(DbModel::Snout^ snout);
osg::Node* BuildDish(DbModel::Dish^ dish);