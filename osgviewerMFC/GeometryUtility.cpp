#include "stdafx.h"
#include "GeometryUtility.h"

#include <osg/ref_ptr>
#include <osg/Geometry>

#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>

#include <Poly_Triangulation.hxx>

#include <BRep_Tool.hxx>
#include <BRepMesh.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>

osg::Geode* BuildMesh(const TopoDS_Face &face, double deflection)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<deprecated_osg::Geometry> triGeom = new deprecated_osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();

	TopLoc_Location location;
	BRepMesh::Mesh(face, deflection);

	Handle_Poly_Triangulation triFace = BRep_Tool::Triangulation(face, location);

	Standard_Integer nTriangles = triFace->NbTriangles();

	gp_Pnt vertex1;
	gp_Pnt vertex2;
	gp_Pnt vertex3;

	Standard_Integer nVertexIndex1 = 0;
	Standard_Integer nVertexIndex2 = 0;
	Standard_Integer nVertexIndex3 = 0;

	const TColgp_Array1OfPnt &nodes = triFace->Nodes();
	const Poly_Array1OfTriangle &triangles = triFace->Triangles();

	for (Standard_Integer i = 1; i <= nTriangles; i++)
	{
		const Poly_Triangle &aTriangle = triangles.Value(i);

		aTriangle.Get(nVertexIndex1, nVertexIndex2, nVertexIndex3);

		vertex1 = nodes.Value(nVertexIndex1).Transformed(location.Transformation());
		vertex2 = nodes.Value(nVertexIndex2).Transformed(location.Transformation());
		vertex3 = nodes.Value(nVertexIndex3).Transformed(location.Transformation());

		gp_XYZ vector12(vertex2.XYZ() - vertex1.XYZ());
		gp_XYZ vector13(vertex3.XYZ() - vertex1.XYZ());
		gp_XYZ normal = vector12.Crossed(vector13);
		Standard_Real rModulus = normal.Modulus();

		if (rModulus > gp::Resolution())
		{
			normal.Normalize();
		}
		else
		{
			normal.SetCoord(0., 0., 0.);
		}

		//if (face.Orientable() != TopAbs_FORWARD)
		//{
		//    normal.Reverse();
		//}

		vertices->push_back(osg::Vec3(vertex1.X(), vertex1.Y(), vertex1.Z()));
		vertices->push_back(osg::Vec3(vertex2.X(), vertex2.Y(), vertex2.Z()));
		vertices->push_back(osg::Vec3(vertex3.X(), vertex3.Y(), vertex3.Z()));

		normals->push_back(osg::Vec3(normal.X(), normal.Y(), normal.Z()));
	}

	triGeom->setVertexArray(vertices.get());
	triGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, vertices->size()));

	triGeom->setNormalArray(normals);
	triGeom->setNormalBinding(deprecated_osg::Geometry::BIND_PER_PRIMITIVE);

	geode->addDrawable(triGeom);

	return geode.release();
}

inline gp_Pnt && ToGpPnt(DbModel::Point^ pnt)
{
	return move(gp_Pnt(pnt->X, pnt->Y, pnt->Z));
}

inline gp_Vec && ToGpVec(DbModel::Point^ pnt)
{
	return move(gp_Vec(pnt->X, pnt->Y, pnt->Z));
}

osg::Geode* BuildCylinder(DbModel::Cylinder^ cyl)
{
	gp_Ax2 axis;
	axis.SetLocation(ToGpPnt(cyl->Org));
	gp_Vec vec = ToGpVec(cyl->Height);
	Standard_Real h = vec.Magnitude();
	vec.Normalize();
	axis.SetDirection(vec);

	TopoDS_Face gpCyl = BRepPrimAPI_MakeCylinder(axis, cyl->Radius, h);
	return BuildMesh(gpCyl);
}

osg::Geode* BuildCircularTorus(DbModel::CircularTorus^ ct)
{
	gp_Pnt center = ToGpPnt(ct->Center);
	gp_Pnt startPnt = ToGpPnt(ct->StartPnt);
	gp_Ax2 axis;
	axis.SetLocation(center);
	axis.SetDirection(ToGpVec(ct->Normal));
	gp_Vec vec(center, startPnt);
	axis.SetXDirection(vec);

	TopoDS_Face gpCt = BRepPrimAPI_MakeTorus(axis, vec.Magnitude(), ct->Radius, ct->Angle);
	return BuildMesh(gpCt);
}

osg::Group* BuildSnout(DbModel::Snout^ snout)
{
	gp_Pnt buttomPnt = ToGpPnt(snout->Org);
	gp_Vec heightVec = ToGpVec(snout->Height);
	gp_Vec offsetVec = ToGpVec(snout->Offset);

	gp_Ax2 axis(buttomPnt, heightVec);
	gp_Circ buttomCirc(axis, snout->ButtomRadius);
	BRepBuilderAPI_MakeEdge buttomEdge(buttomCirc);
	BRepBuilderAPI_MakeWire buttomWire(buttomEdge);

	buttomPnt.Translate(heightVec);
	buttomPnt.Translate(offsetVec);
	axis.SetLocation(buttomPnt);
	gp_Circ topCirc(axis, snout->TopRadius);
	BRepBuilderAPI_MakeEdge topEdge(topCirc);
	BRepBuilderAPI_MakeWire topWire(topEdge);

	BRepOffsetAPI_ThruSections thruSection(Standard_True);
	thruSection.AddWire(buttomWire);
	thruSection.AddWire(topWire);

	osg::Group *group = new osg::Group();
	TopoDS_Shape shape = thruSection;
	for (TopExp_Explorer aFaceExplorer(shape, TopAbs_FACE); aFaceExplorer.More(); aFaceExplorer.Next())
	{
		TopoDS_Face aFace = TopoDS::Face(aFaceExplorer.Current());
		group->addChild(BuildMesh(aFace));
	}
	return group;
}
