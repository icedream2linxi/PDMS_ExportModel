#include "stdafx.h"
#include "RvmLoad.h"
#include <string>
#include <cassert>
using namespace std;

std::istream &operator>>(std::istream &is, osg::Vec3 &vec)
{
	return is >> vec[0] >> vec[1] >> vec[2];
}

std::istream &operator>>(std::istream &is, osg::Matrixd &mat)
{
	is >> mat(0, 0) >> mat(0, 1) >> mat(0, 2) >> mat(3, 0)
		>> mat(1, 0) >> mat(1, 1) >> mat(1, 2) >> mat(3, 1)
		>> mat(2, 0) >> mat(2, 1) >> mat(2, 2) >> mat(3, 2);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 3; ++j)
			mat(i, j) *= 1000.0;
	}
	return is;
}

RvmLoad::RvmLoad(osg::ref_ptr<osg::Group> &root, const std::string &filePath, osg::ref_ptr<ViewCenterManipulator> &mani)
	: m_root(root)
	, m_filePath(filePath)
	, m_mani(mani)
	, m_fin(filePath.c_str())
{
}


RvmLoad::~RvmLoad()
{
}

void RvmLoad::load()
{
	string line;
	if (!getline(m_fin, line))
		return;
	if (line != "HEAD")
		return;
	if (!skipLine(5))
		return;

	if (!getline(m_fin, line))
		return;
	if (line != "MODL")
	{
		if (!getline(m_fin, line))
			return;
	}

	if (line != "MODL")
		return;
	if (!skipLine(2))
		return;

	string siteName;
	m_fin >> siteName;
	loadProject(siteName);
}

bool RvmLoad::skipLine(size_t num)
{
	string line;
	for (size_t i = 0; i < num; ++i)
	{
		if (!getline(m_fin, line))
			return false;
	}
	return true;
}

bool RvmLoad::loadProject(const std::string &name)
{
	string flag;
	while (true)
	{
		m_fin >> flag;
		if (flag != "CNTB")
			return false;
		loadItem(1);
	}
	return true;
}

bool RvmLoad::loadItem(int parentColor)
{
	string flag;
	int v1, v2;
	m_fin >> v1 >> v2;

	string name;
	m_fin >> name;

	osg::Vec3 pos;
	int color;
	m_fin >> pos;
	m_fin >> color;

	color = (color == 1 ? parentColor : color);

	while (true)
	{
		m_fin >> flag;

		if (flag == "PRIM")
		{
			if (!loadPrim(color))
				return false;
		}
		else if (flag == "CNTB")
		{
			if (!loadItem(color))
				return false;
		}
		else if (flag == "CNTE")
		{
			m_fin >> v1 >> v2;
			break;
		}
		else
			assert(false);
	}
	return true;
}

bool RvmLoad::loadPrim(int color)
{
	int v1, v2, type = 0;
	m_fin >> v1, v2 >> type;

	osg::Matrixd mat;
	m_fin >> mat;

	osg::Vec3 boundMin, boundMax;
	m_fin >> boundMin >> boundMax;

	switch (type)
	{
	case 1: // Pyramid
	{
		double bottomXLen, bottomYLen, topXLen, topYLen, offsetX, offsetY, height;
		m_fin >> bottomXLen >> bottomYLen >> topXLen >> topYLen >> offsetX >> offsetY >> height;
		break;
	}
	case 2: // Box
	{
		double xLen, yLen, zLen;
		m_fin >> xLen >> yLen >> zLen;
		break;
	}
	case 3: // Rectangular Torus
	{
		double bottomDist, topDist, height, angle;
		m_fin >> bottomDist >> topDist >> height >> angle;
		break;
	}
	case 4: // Circular Torus
	{
		double majorRadius, minorRadius, angle;
		m_fin >> majorRadius >> minorRadius >> angle;
		break;
	}
	case 5: // Ellipsoid
	{
		double bRadius, aLen;
		m_fin >> bRadius >> aLen;
		break;
	}
	case 6: // Sphere
	{
		double radius, height;
		m_fin >> radius >> height;
		break;
	}
	case 7: // Slope Cylinder
	{
		double bottomRadius, topRadius, height, offsetX, offsetY;
		double bottomXAngle, bottomYAngle, topXAngle, topYAngle;
		m_fin >> bottomRadius >> topRadius >> height >> offsetX >> offsetY
			>> bottomXAngle >> bottomYAngle >> topXAngle >> topYAngle;
		break;
	}
	case 8: // Cylinder
	{
		double radius, height;
		m_fin >> radius >> height;
		break;
	}
	case 9: // Unknown
		assert(false);
		break;
	case 10: // Line
	{
		double param1, len;
		m_fin >> param1 >> len;
		break;
	}
	case 11: // Primitive Set
	{
		int count;
		m_fin >> count;
		for (int i = 0; i < count; ++i)
		{
			int type;
			m_fin >> type;
			if (type == 1) // Triangle Fan
			{
				int vertexCount;
				m_fin >> vertexCount;
				osg::Vec3 vertex;
				for (int j = 0; j < vertexCount; ++j)
					m_fin >> vertex;
			}
			else if (type == 2) // Polygon Sub
			{
				int vertexCount;
				m_fin >> vertexCount;
				osg::Vec3 vertex;
				for (int j = 0; j < vertexCount; ++j)
					m_fin >> vertex;

				m_fin >> vertexCount;
				for (int j = 0; j < vertexCount; ++j)
					m_fin >> vertex;
			}
			else
				assert(false);
		}
	}
	default:
		assert(false);
		break;
	}
	return true;
}