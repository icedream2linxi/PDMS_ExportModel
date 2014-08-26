#include "stdafx.h"
#include "inc\ViewCenterManipulator.h"


ViewCenterManipulator::ViewCenterManipulator()
	: m_isMouseRelease(true)
{
}


ViewCenterManipulator::~ViewCenterManipulator()
{
}

bool ViewCenterManipulator::handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	m_isMouseRelease = false;
	return __super::handleMousePush(ea, us);
}

bool ViewCenterManipulator::handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	m_isMouseRelease = true;
	return __super::handleMouseRelease(ea, us);
}