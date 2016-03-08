// roadDesign.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgViewer/Viewer>
#include <osg/Point>

#include <sisl.h>

#include "camera.h"

#include "Nurbs.h"

int main()
{
	const int numPoints(17);
	const int dim(3);
	const int order(4);
	const double width(4.0f);
	double ctrlpts[numPoints*dim];
	osg::ref_ptr<osg::Vec3dArray> cpoints = new osg::Vec3dArray;
	for (int i = 0, j = 0; i < numPoints; i++)
	{
		const double x = 0.0f;
		const double y = -2 + (width / (numPoints - 1)) * i;
		const double z = 0.0f;

//		const double x = cos(((2 * 3.1415926535897932384626433832795) / numPoints)*i);
//		const double y = sin(((2 * 3.1415926535897932384626433832795) / numPoints)*i);

		ctrlpts[j++] = x;
		ctrlpts[j++] = y;
		ctrlpts[j++] = z;
		cpoints->push_back(osg::Vec3d(x,y,z));
	}
	double knots[numPoints + order] = { 0, 0, 0, 0,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
		14, 14, 14, 14};
	osg::ref_ptr<osg::DoubleArray> kts = new osg::DoubleArray;
	for (int i = 0; i < numPoints + order;i++)
	{
		kts->push_back(knots[i]);
	}

	const int kind = 1;
	SISLCurve* sc = newCurve(numPoints, order, knots, ctrlpts, kind, dim, 0);

	const int der = 0;
	double derive[(der + 1)*dim];
	memset(derive, 0, sizeof(derive));
	double curvature[dim];
	memset(curvature, 0, sizeof(curvature));
	double radius;
	int jstat;
	osg::ref_ptr<osg::Vec3dArray> path = new osg::Vec3dArray;
	osg::ref_ptr<osg::DoubleArray> curv = new osg::DoubleArray;
	int i;
	float k;
	int leftknot;
	const int degree = order - 1;
	const double step = 0.005f;
	for (i = 0; i < numPoints + order - 1; i++)
	{
		leftknot = i;
		for (k = knots[i]; k <= (knots[i + 1]); k += step)
		{
			s1225(sc, der, k, &leftknot, derive, curvature, &radius, &jstat);
			if (jstat == 0)
			{
 				if (path->empty() || path->back() != osg::Vec3d(derive[0],derive[1],derive[2]))
				{
					path->push_back(osg::Vec3d(derive[0], derive[1], derive[2]));
					curv->push_back(radius);
				}
			}
			else
			{
				printf("Wrong\n");
				getchar();
			}
			if (k == knots[i + 1]) break;
		}
	}
	freeCurve(sc);

	osg::ref_ptr<NurbsCurve> refNB = new NurbsCurve;
	refNB->setCtrlPoints(cpoints);
	refNB->setKnotVector(kts);
	refNB->setNumPath(3000);
	refNB->setDegree(degree);
	refNB->update();

	osg::ref_ptr<osg::Geometry> gmtry = new osg::Geometry;
	gmtry->setVertexArray(path);
	osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
	color->push_back(osg::Vec4d(1.0f,0.0f,0.0f,1.0f));
	gmtry->setColorArray(color);
	gmtry->setColorBinding(osg::Geometry::BIND_OVERALL);
	gmtry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, path->size()));
	gmtry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	gmtry->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::Geometry> ctrlGmtry = new osg::Geometry;
	ctrlGmtry->setVertexArray(cpoints);
	osg::ref_ptr<osg::Vec4Array> color1 = new osg::Vec4Array;
	color1->push_back(osg::Vec4d(1.0f, 1.0f, 1.0f, 1.0f));
	ctrlGmtry->setColorArray(color1);
	ctrlGmtry->setColorBinding(osg::Geometry::BIND_OVERALL);
	ctrlGmtry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, cpoints->size()));
	ctrlGmtry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, cpoints->size()));
	ctrlGmtry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	osg::ref_ptr<osg::Point> pointsize = new osg::Point(2.0f);
	ctrlGmtry->getOrCreateStateSet()->setAttribute(pointsize);
	ctrlGmtry->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::Vec3dArray> nowP = new osg::Vec3dArray(cpoints->begin(), cpoints->begin() + 1);
	osg::ref_ptr<osg::Geometry> pointGmtry = new osg::Geometry;
	pointGmtry->setVertexArray(nowP);
	pointGmtry->setColorArray(color1);
	pointGmtry->setColorBinding(osg::Geometry::BIND_OVERALL);
	pointGmtry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, nowP->size()));
	pointGmtry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	pointsize = new osg::Point(5.0f);
	pointGmtry->getOrCreateStateSet()->setAttribute(pointsize);
	pointGmtry->setDataVariance(osg::Object::DYNAMIC);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(gmtry);
	geode->addDrawable(ctrlGmtry);
	geode->addDrawable(pointGmtry);

	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(geode);
	
	osg::ref_ptr<camera> camevent = new camera;
	camevent->setArray(cpoints);
	camevent->setKnotVector(kts);
	camevent->setPath(path);
	camevent->setPathGeometry(gmtry);
	camevent->setCtrlGeometry(ctrlGmtry);
	camevent->setPointGeometry(pointGmtry);
	camevent->setIncr(width / (numPoints - 1));

	osgViewer::Viewer *view = new osgViewer::Viewer;
	view->setSceneData(root);
	view->setUpViewOnSingleScreen(0);
	osg::Camera *cam = view->getCamera();
//	cam->setProjectionMatrixAsOrtho2D(-4, 4, -4 / (16.0f / 9.0f), 4 / (16.0f / 9.0f));
	cam->setProjectionMatrixAsOrtho2D(-4 / (16.0f / 9.0f), 4 / (16.0f / 9.0f), -4, 4);
	view->setCameraManipulator(camevent);

	return view->run();
}

// int main(int argc, char* argv[])
// {
// 	const double numPoints(16);
// 	const double width(4.0f);
// 	
// 	osg::ref_ptr<osg::Vec3dArray> ctrlPoints = new osg::Vec3dArray;
// 	osg::ref_ptr<osg::DoubleArray> knots = new osg::DoubleArray;
// 	knots->push_back(0.0f); knots->push_back(0.0f); knots->push_back(0.0f); knots->push_back(0.0f);
// 
// 	for (int i = 0; i <= numPoints;i++)
// 	{
// 		const double x = 0.0f;
// 		const double y = -2 + (width / numPoints) * i;
// 
// //		const double x = 2 * cos((2*3.1415926/numPoints) * i);
// //		const double y = 2 * sin((2*3.1415926/numPoints) * i);
// 		const double z = 0.0f;
// 
// 		ctrlPoints->push_back(osg::Vec3d(x, y, z));
// 		if (i >= 1 && i<=13)
// 		{
// 			knots->push_back(i);
// 		}
// 	}
// 
// 	knots->push_back(14.0f); knots->push_back(14.0f); knots->push_back(14.0f); knots->push_back(14.0f);
// 
// 	const int density(1000);
// 	osg::ref_ptr<NurbsCurve> nurbs = new NurbsCurve;
// 	nurbs->setCtrlPoints(ctrlPoints);
// 	nurbs->setKnotVector(knots);
// 	nurbs->setDegree(3);
// 	nurbs->setNumPath(density);
// 	nurbs->update();
// 	osg::ref_ptr<osg::Vec3dArray> path = nurbs->getPath();
// 
// 	osg::ref_ptr<osg::Geometry> gmtry = new osg::Geometry;
// 	gmtry->setVertexArray(path);
// 	osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
// 	color->push_back(osg::Vec4d(1.0f,0.0f,0.0f,1.0f));
// 	gmtry->setColorArray(color);
// 	gmtry->setColorBinding(osg::Geometry::BIND_OVERALL);
// 	gmtry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, path->size()));
// 	gmtry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
// 	gmtry->setDataVariance(osg::Object::DYNAMIC);
// 
// 	osg::ref_ptr<osg::Geometry> ctrlGmtry = new osg::Geometry;
// 	ctrlGmtry->setVertexArray(ctrlPoints);
// 	osg::ref_ptr<osg::Vec4Array> color1 = new osg::Vec4Array;
// 	color1->push_back(osg::Vec4d(1.0f, 1.0f, 1.0f, 1.0f));
// 	ctrlGmtry->setColorArray(color1);
// 	ctrlGmtry->setColorBinding(osg::Geometry::BIND_OVERALL);
// 	ctrlGmtry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,ctrlPoints->size()));
// 	ctrlGmtry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, ctrlPoints->size()));
// 	ctrlGmtry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
// 	osg::ref_ptr<osg::Point> pointsize = new osg::Point(2.0f);
// 	ctrlGmtry->getOrCreateStateSet()->setAttribute(pointsize);
// 	ctrlGmtry->setDataVariance(osg::Object::DYNAMIC);
// 
// 	osg::ref_ptr<osg::Vec3dArray> nowP = new osg::Vec3dArray(ctrlPoints->begin(), ctrlPoints->begin() + 1);
// 	osg::ref_ptr<osg::Geometry> pointGmtry = new osg::Geometry;
// 	pointGmtry->setVertexArray(nowP);
// 	pointGmtry->setColorArray(color1);
// 	pointGmtry->setColorBinding(osg::Geometry::BIND_OVERALL);
// 	pointGmtry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, nowP->size()));
// 	pointGmtry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
// 	pointsize = new osg::Point(5.0f);
// 	pointGmtry->getOrCreateStateSet()->setAttribute(pointsize);
// 	pointGmtry->setDataVariance(osg::Object::DYNAMIC);
// 
// 	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
// 	geode->addDrawable(gmtry);
// 	geode->addDrawable(ctrlGmtry);
// 	geode->addDrawable(pointGmtry);
// 
// 	osg::ref_ptr<osg::Group> root = new osg::Group;
// 	root->addChild(geode);
// 
// 	osg::ref_ptr<camera> camevent = new camera;
// 	camevent->setArray(ctrlPoints);
// 	camevent->setKnotVector(knots);
// 	camevent->setPath(path);
// 
// 	camevent->setPathGeometry(gmtry);
// 	camevent->setCtrlGeometry(ctrlGmtry);
// 	camevent->setPointGeometry(pointGmtry);
// 	camevent->setIncr(width / numPoints);
// 
// 	osgViewer::Viewer *view = new osgViewer::Viewer;
// 	view->setSceneData(root);
// 	view->setUpViewOnSingleScreen(1);
// 
// 	osg::Camera *cam = view->getCamera();
// 	cam->setProjectionMatrixAsOrtho2D(-4, 4, -4/(16.0f/9.0f), 4/(16.0f/9.0f));
// 	view->setCameraManipulator(camevent);
// 	
// 	return view->run();
// }

