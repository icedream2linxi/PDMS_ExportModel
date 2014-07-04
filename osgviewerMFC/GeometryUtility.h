#pragma once
#include <osg/Geode>

#include <TopoDS_Face.hxx>

osg::Geode* BuildMesh(const TopoDS_Face &face, double deflection = 0.1);
osg::Geode* BuildCylinder(DbModel::Cylinder^ cyl);
osg::Geode* BuildCircularTorus(DbModel::CircularTorus^ ct);