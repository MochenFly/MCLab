#pragma once

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>

class ImageInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static ImageInteractorStyle* New();
    vtkTypeMacro(ImageInteractorStyle, vtkInteractorStyleTrackballCamera);

    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMiddleButtonDown() override;
    void OnMiddleButtonUp() override;
    void OnRightButtonDown() override;
    void OnRightButtonUp() override;
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;

public:
    void setRenderWindow(vtkGenericOpenGLRenderWindow* renderWindow);
    vtkGenericOpenGLRenderWindow* getRenderWindow();

    void setSecondRenderer(vtkRenderer* renderer);
    vtkRenderer* getSecondRenderer();

protected:
    ImageInteractorStyle();
    ~ImageInteractorStyle() override;

private:
    ImageInteractorStyle(const ImageInteractorStyle&) = delete;
    void operator=(const ImageInteractorStyle&) = delete;

private:
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>       m_pRenderWindow                         { nullptr };
    vtkSmartPointer<vtkRenderer>                        m_pSecondRenderer                       { nullptr };
};
