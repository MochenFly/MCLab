#pragma once

#include "ImageInteractorStyle.h"

#include <vtkRenderer.h>

class EllipseImageInteractorStyle : public ImageInteractorStyle
{
public:
    static EllipseImageInteractorStyle* New();
    vtkTypeMacro(EllipseImageInteractorStyle, ImageInteractorStyle);

    void OnLeftButtonDown() override;
    void OnMouseMove() override;
    void OnLeftButtonUp() override;

protected:
    EllipseImageInteractorStyle();
    ~EllipseImageInteractorStyle() override;

private:
    EllipseImageInteractorStyle(const EllipseImageInteractorStyle&) = delete;
    void operator=(const EllipseImageInteractorStyle&) = delete;

private:
    vtkSmartPointer<vtkActor>               m_pEllipseActor                 { nullptr};

    bool        m_drawing                   { false };
    int         m_startPos[2]               { 0, 0 };
};
