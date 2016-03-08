#pragma once
#include <osgGA/CameraManipulator>
#include "Nurbs.h"

class camera :
	public osgGA::CameraManipulator
{
public:
	camera();
	~camera();
	inline void setArray(osg::ref_ptr<osg::Vec3dArray> ref)
	{
		_ctrlPoints = ref;
		_originalCtrl = new osg::Vec3dArray(_ctrlPoints->begin(), _ctrlPoints->end());
	}
	inline void setPath(osg::ref_ptr<osg::Vec3dArray> ref)
	{
		_path = ref;
	}
	inline void setPathGeometry(osg::Geometry *ref)
	{
		_gmtry = ref;
	}
	inline void setCtrlGeometry(osg::Geometry *ref)
	{
		_ctrlGmtry = ref;
	}
	inline void setPointGeometry(osg::Geometry *ref)
	{
		_pointGmtry = ref;
	}
	inline void setKnotVector(osg::ref_ptr<osg::DoubleArray> ref)
	{
		_knot = ref;
	}
	inline void setIncr(double ref)
	{
		_incr = ref;
	}
protected:
	virtual osg::Matrixd getInverseMatrix() const;
	virtual osg::Matrixd getMatrix() const;
	virtual void setByInverseMatrix(const osg::Matrixd& matrix);
	virtual void setByMatrix(const osg::Matrixd& matrix);
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
	void save();
	void generateCurve();

private:
	osg::Quat _camRotation;
	osg::Vec3d _offset;
	osg::Vec3d _eyePoint;

	osg::ref_ptr<osg::Vec3dArray> _originalCtrl;
	osg::ref_ptr<osg::Vec3dArray> _ctrlPoints;
	unsigned _interactiveCtrl;
	bool _ctrlUpdated;
	osg::ref_ptr<osg::Vec3dArray> _path;
	osg::ref_ptr<osg::DoubleArray> _knot;
	osg::ref_ptr<osg::DoubleArray> _curvature;
	osg::ref_ptr<osg::DoubleArray> _originalRadius;

	osg::Geometry *_gmtry;
	osg::Geometry *_ctrlGmtry;
	osg::Geometry *_pointGmtry;

	unsigned _filename;

	double _incr;
	double _CR;

	osg::Matrixd _proj;
	double _scaleV;
	double _length;
	const double _tellScale;
};

