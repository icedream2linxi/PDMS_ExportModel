#pragma once
#include <osgGA/TrackballManipulator>

class ViewCenterManipulator :
	public osgGA::TrackballManipulator
{
public:
	ViewCenterManipulator();
	~ViewCenterManipulator();
	
	bool isMouseRelease() const;

protected:
	virtual bool handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	virtual bool handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

protected:
	bool m_isMouseRelease;
};

inline bool ViewCenterManipulator::isMouseRelease() const
{
	return m_isMouseRelease;
}
