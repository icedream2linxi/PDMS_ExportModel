#include "stdafx.h"
#include "inc\CombineGeometry.h"


CombineGeometry::CombineGeometry()
{
}


CombineGeometry::~CombineGeometry()
{
}

void CombineGeometry::subDraw()
{
	osg::ref_ptr<osg::Vec3Array> vertexArr = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArr = new osg::Vec3Array;
	setVertexArray(vertexArr);
	setNormalArray(normalArr, osg::Array::BIND_PER_VERTEX);
	osg::ref_ptr<osg::Vec4Array> colArr = new osg::Vec4Array();
	colArr->push_back(m_color);
	setColorArray(colArr, osg::Array::BIND_OVERALL);

	// shell
	std::vector<osg::Vec3> triVertexArr, triNormalArr;
	for each(const auto &shell in m_shells)
	{
		for (size_t i = 0; i < shell->faces.size();)
		{
			if (shell->faces[i] == 4)
			{
				for (int j = 0; j < shell->faces[i]; ++j)
				{
					vertexArr->push_back(shell->vertexs[shell->faces[i + j + 1]]);
				}

				osg::Vec3 vec1 = shell->vertexs[shell->faces[i + 2]] - shell->vertexs[shell->faces[i + 1]];
				osg::Vec3 vec2 = shell->vertexs[shell->faces[i + 3]] - shell->vertexs[shell->faces[i + 2]];
				osg::Vec3 normal = vec1 ^ vec2;
				normal.normalize();
				for (int j = 0; j < shell->faces[i]; ++j)
					normalArr->push_back(normal);
			}
			else if (shell->faces[i] == 3)
			{
				for (int j = 0; j < shell->faces[i]; ++j)
				{
					triVertexArr.push_back(shell->vertexs[shell->faces[i + j + 1]]);
				}

				osg::Vec3 vec1 = shell->vertexs[shell->faces[i + 2]] - shell->vertexs[shell->faces[i + 1]];
				osg::Vec3 vec2 = shell->vertexs[shell->faces[i + 3]] - shell->vertexs[shell->faces[i + 2]];
				osg::Vec3 normal = vec1 ^ vec2;
				normal.normalize();
				for (int j = 0; j < shell->faces[i]; ++j)
					triNormalArr.push_back(normal);
			}

			i += shell->faces[i] + 1;
		}
	}

	// mesh
	for each (const auto &mesh in m_meshs)
	{
		for (int i = 0; i < mesh->rows - 1; ++i)
		{
			for (int j = 0; j < mesh->colums - 1; ++j)
			{
				osg::Vec3 pnt1 = mesh->vertexs[i * mesh->colums + j];
				osg::Vec3 pnt2 = mesh->vertexs[i * mesh->colums + j + 1];
				osg::Vec3 pnt3 = mesh->vertexs[(i + 1) * mesh->colums + j + 1];
				osg::Vec3 pnt4 = mesh->vertexs[(i + 1) * mesh->colums + j];
				vertexArr->push_back(pnt1);
				vertexArr->push_back(pnt2);
				vertexArr->push_back(pnt3);
				vertexArr->push_back(pnt4);

				osg::Vec3 normal = (pnt2 - pnt1) ^ (pnt1 - pnt4);
				normal.normalize();
				for (int k = 0; k < 4; ++k)
					normalArr->push_back(normal);
			}
		}
	}

	if (!vertexArr->empty())
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertexArr->size()));

	size_t first = vertexArr->size();
	for (size_t i = 0; i < triVertexArr.size(); ++i)
	{
		vertexArr->push_back(triVertexArr[i]);
		normalArr->push_back(triNormalArr[i]);
	}
	if (!triVertexArr.empty())
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, first, vertexArr->size() - first));

	// polygon
	for each (const auto &polygon in m_polygons)
	{
		first = vertexArr->size();
		osg::Vec3 normal = (polygon->vertexs[1] - polygon->vertexs[0]) ^ (polygon->vertexs[2] - polygon->vertexs[1]);
		normal.normalize();
		for (size_t i = 0; i < polygon->vertexs.size(); ++i)
		{
			vertexArr->push_back(polygon->vertexs[i]);
			normalArr->push_back(normal);
		}
		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, first, vertexArr->size() - first));
	}
}