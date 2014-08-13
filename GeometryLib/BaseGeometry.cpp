#include "stdafx.h"
#include "BaseGeometry.h"


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