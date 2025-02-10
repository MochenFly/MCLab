#pragma once

#include "ImageInteractorStyle.h"

class RectangleImageInteractorStyle : public ImageInteractorStyle
{
public:
    static RectangleImageInteractorStyle* New();
    vtkTypeMacro(RectangleImageInteractorStyle, ImageInteractorStyle);

protected:
    RectangleImageInteractorStyle();
    ~RectangleImageInteractorStyle() override;

private:
    RectangleImageInteractorStyle(const RectangleImageInteractorStyle&) = delete;
    void operator=(const RectangleImageInteractorStyle&) = delete;
};
