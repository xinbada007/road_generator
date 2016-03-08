#include "stdafx.h"
#include "camera.h"
#include <osg/Geometry>
#include <fstream>
#include <string>
#include <stdio.h>
#include <iostream>
#include "sisl.h"

camera::camera() :_tellScale(200.0f)
{
	osg::Matrix lMat;
	lMat.makeRotate(3.14159265 / 2, osg::Vec3d(1.0f,0.0f,0.0f));
//	_camRotation = lMat.getRotate();

	_offset.set(0.0f, 0.0f, 0.0f);
	_eyePoint.set(_offset);

	_interactiveCtrl = 0;

	_ctrlUpdated = false;

	_filename = 0;

	_incr = 0;

	_curvature = new osg::DoubleArray;
	_originalRadius = new osg::DoubleArray;

	_scaleV = 1.0f;
	_length = 0.0f;

	_CR = 0.0025f;
}

camera::~camera()
{
}

void camera::setByMatrix(const osg::Matrixd& matrix)
{
}

void camera::setByInverseMatrix(const osg::Matrixd& matrix)
{
}

osg::Matrixd camera::getMatrix() const
{
	return osg::Matrix::rotate(_camRotation) *
		osg::Matrix::translate(_eyePoint);
}

osg::Matrixd camera::getInverseMatrix() const
{
	return osg::Matrix::inverse(_proj);
	return osg::Matrix::translate(-_eyePoint) *
		osg::Matrix::rotate(_camRotation.inverse());
}

void camera::save()
{
	if (_ctrlPoints->empty() || _knot->empty())
	{
		return;
	}

	char temp[10];
	_itoa_s(_filename, temp,10);
	std::string fn = "..\\CtrlPoints\\";
	fn += temp;
	fn += ".txt";

	std::fstream fileout(fn.c_str(), std::ios::out);

	fileout << "EndlessNurbs" << std::endl;
	fileout << "ctrlpoints" << std::endl;
	osg::Vec3dArray::const_iterator i = _ctrlPoints->begin();
	while (i != _ctrlPoints->end())
	{
		fileout << (*i).x() << "\t" << (*i).y() << "\t" << (*i).z() << std::endl;
		i++;
	}
	fileout << "knots" << std::endl;

	osg::DoubleArray::const_iterator j = _knot->begin();
	while (j != _knot->end())
	{
		fileout << *j << std::endl;
		j++;
	}
	fileout << "#Curvature" << std::endl;
	j = _curvature->begin();
	int k = 0;
	while (j != _curvature->end() && k!= 10)
	{
		fileout << "#" << *j << std::endl;;
		j++;
		k++;
	}
	fileout << "#Length" << std::endl;
	fileout << "#" << _length << std::endl;
// 	fileout << "#Scale" << std::endl;
// 	fileout << "#" << _tellScale << std::endl;

	fileout.close();

	fn.clear();
	fn = "..\\CtrlPoints\\";
	fn += temp;
	fn += ".radius";
	std::fstream fout(fn.c_str(), std::ios::out);
	j = _originalRadius->begin();
	while (j!=_originalRadius->end())
	{
		fout << *j << std::endl;
		j++;
	}
	
	{
		char tempd[20];
		const unsigned size_tempd(sizeof(tempd));
		const unsigned numDigit(6);
		std::string roads;

		std::string ctrlfn = "..\\CtrlPoints\\";
		ctrlfn += "roads.txt";
		std::fstream wout(ctrlfn.c_str(), std::ios::out);
		osg::Vec3dArray::const_iterator j = _path->begin();
		while (j != _path->end())
		{
			_gcvt_s(tempd, size_tempd, (*j).x(), numDigit);
			roads += tempd;
			roads += "\t";
			_gcvt_s(tempd, size_tempd, (*j).y(), numDigit);
			roads += tempd;
			roads += "\t";
			_gcvt_s(tempd, size_tempd, (*j).z(), numDigit);
			roads += tempd;
			roads += "\n";
			j++;
		}

		wout << roads << std::endl;
	}

	_filename++;
}

bool camera::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	switch (ea.getEventType())
	{
	case osgGA::GUIEventAdapter::KEYDOWN:
		switch (ea.getKey())
		{
		case osgGA::GUIEventAdapter::KEY_Up:
			if (_interactiveCtrl < _ctrlPoints->size() - 1)
			{
				++_interactiveCtrl;
			}

			break;
		case osgGA::GUIEventAdapter::KEY_Down:
			if (_interactiveCtrl > 0)
			{
				--_interactiveCtrl;
			}

			break;
		case osgGA::GUIEventAdapter::KEY_Left:
			_ctrlPoints->at(_interactiveCtrl).x() -= _CR;
			_ctrlUpdated = true;
			break;
		case osgGA::GUIEventAdapter::KEY_Right:
			_ctrlPoints->at(_interactiveCtrl).x() += _CR;
			_ctrlUpdated = true;
			break;
		case osgGA::GUIEventAdapter::KEY_Page_Down:
			_ctrlPoints->at(_interactiveCtrl).y() -= _CR;
			_ctrlUpdated = true;
			break;
		case osgGA::GUIEventAdapter::KEY_Page_Up:
			_ctrlPoints->at(_interactiveCtrl).y() += _CR;
			_ctrlUpdated = true;
			break;
		case osgGA::GUIEventAdapter::KEY_KP_Enter:
			if (!_originalCtrl->empty())
			{
				_ctrlPoints->clear();
				osg::Vec3dArray::const_iterator i = _originalCtrl->begin();
				while (i != _originalCtrl->end())
				{
					_ctrlPoints->push_back(*i);
					i++;
				}
				_ctrlUpdated = true;
			}
			break;
		case osgGA::GUIEventAdapter::KEY_S:
			save();
			break;
		case osgGA::GUIEventAdapter::KEY_KP_Add:
			if (_incr)
			{
				_incr *= ((double) (_ctrlPoints->size() - 1) / (double)(_ctrlPoints->size()));
				osg::Vec3d start = *_originalCtrl->begin();
				_originalCtrl->clear();
				for (unsigned i = 0; i < _ctrlPoints->size() + 1;i++)
				{
					const double x = start.x();
					const double y = start.y() + i*_incr;
					const double z = start.z();
					_originalCtrl->push_back(osg::Vec3d(x, y, z));
				}

				_ctrlPoints->clear();
				osg::Vec3dArray::const_iterator i = _originalCtrl->begin();
				while (i != _originalCtrl->end())
				{
					_ctrlPoints->push_back(*i);
					i++;
				}

				unsigned size = _ctrlPoints->size() + 4;
				_knot->clear();
				_knot->push_back(0.0f); _knot->push_back(0.0f); _knot->push_back(0.0f); _knot->push_back(0.0f);
				unsigned int ii = 1;
				for (ii = 1; ii <= size - 8;ii++)
				{
					_knot->push_back(ii);
				}
				_knot->push_back(ii); _knot->push_back(ii); _knot->push_back(ii); _knot->push_back(ii);

				_ctrlUpdated = true;
				_proj = osg::Matrix::identity();
			}
			break;
		case osgGA::GUIEventAdapter::KEY_KP_Multiply:
			if (!_ctrlPoints->empty())
			{
				std::cout << "Input scale factor:\t" << std::endl;
				std::cin >> _scaleV;
				osg::Vec3dArray::iterator i = _ctrlPoints->begin();
				while (i != _ctrlPoints->end())
				{
					(*i) = (*i) * osg::Matrix::scale(osg::Vec3d(_scaleV, _scaleV, _scaleV));
					i++;
				}
				_CR *= _scaleV;
				_ctrlUpdated = true;
			}
			break;
		case osgGA::GUIEventAdapter::KEY_F1:
			_proj = /*_proj**/osg::Matrix::scale(osg::Vec3d(_scaleV, _scaleV, _scaleV));
			break;
		default:
			break;
		}

	case osgGA::GUIEventAdapter::FRAME:
		if (_ctrlUpdated)
		{
// 			osg::ref_ptr<NurbsCurve> nurbs = new NurbsCurve;
// 			nurbs->setCtrlPoints(_ctrlPoints);
// 			nurbs->setKnotVector(_knot);
// 			nurbs->setDegree(3);
// 			nurbs->setNumPath(1000);
// 			nurbs->update();
// 			_path = nurbs->getPath();
			generateCurve();
			if (_gmtry)
			{
				_gmtry->setVertexArray(_path);
				_gmtry->setPrimitiveSet(0, new osg::DrawArrays(GL_LINE_STRIP, 0, _path->size()));
				_gmtry->dirtyBound();
				_gmtry->dirtyDisplayList();
			}
			
			if (_ctrlGmtry)
			{
				_ctrlGmtry->setVertexArray(_ctrlPoints);
				_ctrlGmtry->setPrimitiveSet(0, new osg::DrawArrays(GL_LINE_STRIP, 0, _ctrlPoints->size()));
				_ctrlGmtry->setPrimitiveSet(1, new osg::DrawArrays(GL_POINTS, 0, _ctrlPoints->size()));
				_ctrlGmtry->dirtyBound();
				_ctrlGmtry->dirtyDisplayList();
			}

			_ctrlUpdated = false;
		}

		if (_pointGmtry)
		{
			osg::ref_ptr<osg::Vec3dArray> nowP = new osg::Vec3dArray(_ctrlPoints->begin() + _interactiveCtrl, _ctrlPoints->begin() + _interactiveCtrl + 1);
			_pointGmtry->setVertexArray(nowP);
			_pointGmtry->dirtyBound();
			_pointGmtry->dirtyDisplayList();
		}

		break;
	default:
		break;
	}

	return false;
}

void camera::generateCurve()
{
	osg::Vec3dArray::const_iterator i = _ctrlPoints->begin();
	double pts[999];
	memset(pts, 0, sizeof(pts));
	int j = 0;
	while (i != _ctrlPoints->end())
	{
		pts[j++] = (*i).x();
		pts[j++] = (*i).y();
		pts[j++] = (*i).z();

		i++;
	}
	osg::DoubleArray::const_iterator ii = _knot->begin();
	double kts[999];
	j = 0;
	while (ii != _knot->end())
	{
		kts[j++] = *ii;
		ii++;
	}
	const int kind = 1;
	const int numPoints = _ctrlPoints->size();
	const int order = 4;
	const int dim = 3;
	SISLCurve* sc = newCurve(numPoints, order, kts, pts, kind, dim, 1);
	const int der = 0;
	const double step = 0.005f;
	double derive[(der + 1)*dim];
	memset(derive, 0, sizeof(derive));
	double curvature[dim];
	memset(curvature, 0, sizeof(curvature));
	double radius;
	int jstat;
	_path->clear();
	_curvature->clear();
	_originalRadius->clear();
	for (int i = 0; i < _knot->size() - 1; i++)
	{
		int leftknot = i;
		for (double k = kts[i]; k <= double(kts[i + 1]); k += step)
		{
			s1225(sc, der, k, &leftknot, derive, curvature, &radius, &jstat);
			if (jstat == 0)
			{
				if (_path->empty() || _path->back() != osg::Vec3d(derive[0], derive[1], derive[2]))
				{
					_path->push_back(osg::Vec3d(derive[0], derive[1], derive[2]));
					_originalRadius->push_back(radius);
					if (radius != -1 && radius != 0)
					{
						_curvature->push_back(radius);
					}
					else if (radius == 0)
					{
						printf("Wrong\n");
					}
				}
			}
			else
			{
				printf("Wrong\n");
				getchar();
			}
			if (k == kts[i + 1]) break;
		}
	}

// 	for (int i = dim; i < _knot->size() - order; i++)
// 	{
// 		for (double j = kts[i]; j <= double(kts[i + 1]); j += 0.005)
// 		{
// 			int leftknot = i;
// 			s1225(sc, der, j, &leftknot, derive, curvature, &radius, &jstat);
// 			if (jstat == 0)
// 			{
// 				_path->push_back(osg::Vec3d(derive[0], derive[1], derive[2]));
// 				_originalRadius->push_back(radius);
// 				if (radius != -1 && radius != 0)
// 				{
// 					_curvature->push_back(radius);
// 				}
// 				else if (radius == 0)
// 				{
// 					printf("Wrong\n");
// 				}
// 			}
// 			else
// 			{
// 				printf("Wrong\n");
// 			}
// 		}
// 	}

	if (_curvature->size() > 10)
	{
		std::sort(_curvature->begin(), _curvature->end());
		ii = _curvature->end() - 10;
		while (ii != _curvature->end())
		{
			printf("\n\n");
			ii++;
		}
//		ii = _curvature->end() - 10;
		ii = _curvature->begin();
		while (ii != _curvature->begin() + 10)
		{
			printf("%-.6f\n", *ii);
			ii++;
		}
	}
	
	double epsge = 1e-8;
	double length(0.0f);
	s1240(sc, epsge, &length, &jstat);
	if (!jstat)
	{
		printf("Length = %-.6f\n", length);
		_length = length;
	}
	freeCurve(sc);
}