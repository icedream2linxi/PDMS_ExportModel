#include "stdafx.h"
#include "BaseGeometry.h"

namespace Geometry
{

BaseGeometry::BaseGeometry()
	: m_division(16)
{
}


BaseGeometry::~BaseGeometry()
{
}

void BaseGeometry::draw()
{
	subDraw();
}

unsigned int BaseGeometry::getDivision()
{
	return m_division;
}

void BaseGeometry::subDraw()
{

}

bool BaseGeometry::cullAndUpdate(const osg::CullStack &cullStack)
{
	return false;
}

double GetEpsilon()
{
	return 0.00001;
}

} // namespace Geometry