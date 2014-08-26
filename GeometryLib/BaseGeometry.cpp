#include "stdafx.h"
#include "BaseGeometry.h"

namespace Geometry
{

const int g_defaultDivision = 16;

BaseGeometry::BaseGeometry()
	: m_division(g_defaultDivision)
	, m_needRedraw(true)
	, m_isCulled(false)
{
}


BaseGeometry::~BaseGeometry()
{
}

void BaseGeometry::draw()
{
	subDraw();
	m_needRedraw = false;
	setUpdateCallback(NULL);
}

unsigned int BaseGeometry::getDivision()
{
	return m_division;
}

void BaseGeometry::subDraw()
{

}

bool BaseGeometry::doCullAndUpdate(const osg::CullStack &cullStack)
{
	return false;
}

void BaseGeometry::updateDivision(float pixelSize)
{
	int div = computeDivision(pixelSize);

	if (m_division == div)
		return;

	m_division = div;
	m_needRedraw = true;
}

int BaseGeometry::computeDivision(float pixelSize)
{
	int div = 8;
	if (pixelSize < 40.0f)
		div = 8;
	else if (pixelSize < 120.0f)
		div = 12;
	else if (pixelSize < 300.0f)
		div = 16;
	else if (pixelSize < 600.0f)
		div = 20;
	else if (pixelSize < 1000.0f)
		div = 24;
	else
		div = 32;
	return div;
}

bool BaseGeometry::cullAndUpdate(const osg::CullStack &cullStack)
{
	return m_isCulled = doCullAndUpdate(cullStack);
}

double GetEpsilon()
{
	return 0.00001;
}

} // namespace Geometry