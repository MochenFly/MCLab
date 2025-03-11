#pragma once

#include <vtkInteractorStyleTrackballCamera.h>
class ParametricSplineInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static ParametricSplineInteractorStyle* New();
    vtkTypeMacro(ParametricSplineInteractorStyle, vtkInteractorStyleTrackballCamera);

protected:
    ParametricSplineInteractorStyle();
    ~ParametricSplineInteractorStyle() override;

private:
    ParametricSplineInteractorStyle(const ParametricSplineInteractorStyle&) = delete;
    void operator=(const ParametricSplineInteractorStyle&) = delete;
};

