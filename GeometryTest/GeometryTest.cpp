// GeometryTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Geometry.hpp>

osg::ref_ptr<osg::Geode> TestCircularTorus()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 startPnt(300, 0, 0);
	osg::Vec3 normal(0, 0, 1);
	double radius = 50.0, angle = M_PI / 2;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildCircularTorus(center, startPnt, normal, radius, angle, osg::Vec4(1, 1, 1, 0)));
	return geode;
}

osg::ref_ptr<osg::Geode> TestRectangularTorus()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 startPnt(300, 0, 0);
	osg::Vec3 normal(0, 0, 1);
	double width = 100.0, height = 50.0, angle = M_PI * 2;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildRectangularTorus(center, startPnt, normal, width, height, angle, osg::Vec4(1, 1, 1, 0)));
	return geode;
}

osg::ref_ptr<osg::Geode> TestCone()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 height(0, 0, 500);
	osg::Vec3 top(200, 0, 0);
	double radius = 200;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildCone(center, height, top - center, radius, osg::Vec4(1, 1, 1, 0)));
	return geode;
}

void InitWnd(osgViewer::Viewer &viewer)
{
	int xoffset = 40;
	int yoffset = 40;

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->x = xoffset + 0;
	traits->y = yoffset + 0;
	traits->width = 600;
	traits->height = 480;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;

	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setGraphicsContext(gc.get());
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
	camera->setDrawBuffer(buffer);
	camera->setReadBuffer(buffer);
	//camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));
	//camera->setProjectionMatrixAsPerspective(
	//	30.0f, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0, 1000.0);

	// add this slave camera to the viewer, with a shift left of the projection matrix
	viewer.addSlave(camera.get(), osg::Matrixd::translate(1.0, 0.0, 0.0), osg::Matrixd());
}

int main(int argc, char* argv[])
{
	osgViewer::Viewer myViewer;
	InitWnd(myViewer);
	osg::ref_ptr<osg::Group> root = new osg::Group();

	//root->addChild(TestCasCadeByMakeRevol());
	//root->addChild(TestCasCadeByMakeTorus());
	//root->addChild(TestCircularTorus());
	//root->addChild(TestRectangularTorus());
	root->addChild(TestCone());

	myViewer.setSceneData(root);

	myViewer.addEventHandler(new osgGA::StateSetManipulator(myViewer.getCamera()->getOrCreateStateSet()));
	myViewer.addEventHandler(new osgViewer::StatsHandler);
	myViewer.addEventHandler(new osgViewer::WindowSizeHandler);

	return myViewer.run();

}