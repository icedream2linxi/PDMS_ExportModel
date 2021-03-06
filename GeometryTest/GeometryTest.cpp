// GeometryTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Geometry.hpp>

osg::ref_ptr<osg::Geode> TestCircularTorus()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 startPnt(300, 0, 0);
	osg::Vec3 normal(0, 0, 1);
	double startRadius = 50.0, endRadius = 25.0, angle = M_PI / 2;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildCircularTorus(center, startPnt, normal,
		startRadius, endRadius, angle, osg::Vec4(1, 1, 1, 0)));
	return geode;
}

osg::ref_ptr<osg::Geode> TestRectangularTorus()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 startPnt(300, 0, 0);
	osg::Vec3 normal(0, 0, 1);
	double startWidth = 100.0, startHeight = 50.0, angle = M_PI * 2;
	double endWidth = 50.0, endHeight = 25.0;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildRectangularTorus(center, startPnt, normal,
		startWidth, startHeight, endWidth, endHeight, angle, osg::Vec4(1, 1, 1, 0)));
	return geode;
}

osg::ref_ptr<osg::Geode> TestCone()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 height(0, 0, 500);
	osg::Vec3 offset(200, 0, 0);
	double radius = 200;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildCone(center, height, offset, radius, osg::Vec4(1, 0, 0, 0)));
	return geode;
}

osg::ref_ptr<osg::Geode> TestSnout()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 height(0, 0, 500);
	osg::Vec3 offset(200, 0, 0);
	double bottomRadius = 200, topRadius = 100;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildSnout(center, height, offset, bottomRadius, topRadius, osg::Vec4(1, 1, 1, 0)));
	return geode;
}

osg::ref_ptr<osg::Geode> TestPyramid()
{
	osg::Vec3 org(0, 0, 0);
	osg::Vec3 height(0, 0, 500);
	osg::Vec3 xAxis(1, 0, 0);
	osg::Vec3 offset(100, 0, 0);
	double bottomXLen = 300, bottomYLen = 100;
	double topXLen = 200, topYlen = 50;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildPyramid(org, height, xAxis, offset, bottomXLen, bottomYLen, topXLen, topYlen, osg::Vec4(1, 1, 1, 0)));
	return geode;
}

osg::ref_ptr<osg::Geode> TestSphere()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 height(0, 0, 300);
	double bottomRadius = 150;
	osg::Vec4 color(1, 1, 1, 0);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildSphere(center, height, bottomRadius, color));
	return geode;
}

osg::ref_ptr<osg::Geode> TestEllipsoid()
{
	osg::Vec3 center(0, 0, 0);
	osg::Vec3 height(100, 100, 100);
	double bottomRadius = 500;
	double angle = 270.0 * M_PI / 180.0;
	osg::Vec4 color(1, 1, 1, 0);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildEllipsoid(center, height, bottomRadius, angle, color));
	return geode;
}

osg::ref_ptr<osg::Geode> TestSCylinder()
{
	osg::Vec3 org(0, 0, 0);
	osg::Vec3 height(0, 0, 200);
	osg::Vec3 bottomNormal(0, -1, -1);
	bottomNormal.normalize();
	double radius = 50;
	osg::Vec4 color(1, 1, 1, 0);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildSCylinder(org, height, bottomNormal, radius, color));
	return geode;
}

osg::ref_ptr<osg::Geode> TestSaddle()
{
	osg::Vec3 org(0, 0, 0);
	osg::Vec3 xLen(200, 0, 0), zLen(0, 0, 150);
	double yLen = 100, radius = 30;
	osg::Vec4 color(1, 1, 1, 0);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildSaddle(org, xLen, yLen, zLen, radius, color));
	return geode;
}

osg::ref_ptr<osg::Geode> TestRectCirc()
{
	osg::Vec3 rectCenter(0, 0, 0);
	osg::Vec3 xLen(200, 0, 0), height(0, 0, 150), offset(0, 30, 0);
	double yLen = 100, radius = 50;
	osg::Vec4 color(1, 1, 1, 0);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(Geometry::BuildRectCirc(rectCenter, xLen, yLen, height, offset, radius, color));
	return geode;
}

osg::ref_ptr<osg::Fog> TestFog()
{
	bool bLinear = true;
	osg::ref_ptr<osg::Fog> fog = new osg::Fog();
	fog->setColor(osg::Vec4(1, 1, 1, 1));
	fog->setDensity(0.01f);
	if (bLinear)
		fog->setMode(osg::Fog::LINEAR);
	else
		fog->setMode(osg::Fog::EXP);
	fog->setStart(5.0f);
	fog->setEnd(2000.0f);
	return fog;
}

osg::ref_ptr<osg::Node> TestExplode()
{
	osg::ref_ptr<osg::Group> explode = new osg::Group();
	osg::Vec3 wind(1, 0, 0);
	osg::Vec3 position(0, 0, -10);

	osg::ref_ptr<osgParticle::ExplosionEffect> explosion = new osgParticle::ExplosionEffect(position, 1.0f);
	osg::ref_ptr<osgParticle::ExplosionDebrisEffect> explosionDebri = new osgParticle::ExplosionDebrisEffect(position, 1.0f);
	osg::ref_ptr<osgParticle::SmokeEffect> smoke = new osgParticle::SmokeEffect(position, 1.0f);
	osg::ref_ptr<osgParticle::FireEffect> fire = new osgParticle::FireEffect(position, 1.0f);

	explosion->setWind(wind);
	explosionDebri->setWind(wind);
	smoke->setWind(wind);
	fire->setWind(wind);

	explode->addChild(explosion);
	explode->addChild(explosionDebri);
	explode->addChild(smoke);
	explode->addChild(fire);

	return explode;
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
	//root->addChild(TestCone());
	//root->addChild(TestSnout());
	//root->addChild(TestPyramid());
	//root->addChild(TestSphere());
	//root->addChild(TestEllipsoid());
	//root->addChild(TestSCylinder());
	//root->addChild(TestSaddle());
	root->addChild(TestRectCirc());
	//root->getOrCreateStateSet()->setAttributeAndModes(TestFog(), osg::StateAttribute::ON);

	//osg::ref_ptr<osgParticle::PrecipitationEffect> pe = new osgParticle::PrecipitationEffect();
	//pe->snow(0.5f);
	//pe->rain(0.5f);
	//myViewer.getCamera()->setClearColor(pe->getFog()->getColor());

	//osg::ref_ptr<osg::Node> node = TestCone();
	//node->getOrCreateStateSet()->setAttributeAndModes(pe->getFog());
	//root->addChild(node);
	//root->addChild(pe);
	//root->addChild(TestExplode());

	osgUtil::Optimizer optimizer;
	optimizer.optimize(root);

	myViewer.setSceneData(root);
	myViewer.realize();

	myViewer.addEventHandler(new osgGA::StateSetManipulator(myViewer.getCamera()->getOrCreateStateSet()));
	myViewer.addEventHandler(new osgViewer::StatsHandler);
	myViewer.addEventHandler(new osgViewer::WindowSizeHandler);

	return myViewer.run();

}