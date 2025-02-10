#pragma once

#include "ImageInteractorStyle.h"

#include <vtkRenderer.h>

class EllipseImageInteractorStyle : public ImageInteractorStyle
{
public:
    static EllipseImageInteractorStyle* New();
    vtkTypeMacro(EllipseImageInteractorStyle, ImageInteractorStyle);

protected:
    EllipseImageInteractorStyle();
    ~EllipseImageInteractorStyle() override;

private:
    EllipseImageInteractorStyle(const EllipseImageInteractorStyle&) = delete;
    void operator=(const EllipseImageInteractorStyle&) = delete;

private:
    vtkSmartPointer<vtkActor>               m_pEllipseActor                 { nullptr};
};
