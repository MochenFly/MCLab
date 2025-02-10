#include "ImageInteractorStyle.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>

vtkStandardNewMacro(ImageInteractorStyle);

ImageInteractorStyle::ImageInteractorStyle()
{

}

ImageInteractorStyle::~ImageInteractorStyle()
{
}

void ImageInteractorStyle::OnMouseMove()
{
    this->Superclass::OnMouseMove();
}

void ImageInteractorStyle::OnLeftButtonDown()
{
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    this->FindPokedRenderer(x, y);

    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    if (this->Interactor->GetShiftKey())
    {
        if (this->Interactor->GetControlKey())
        {
            this->StartDolly();
        }
        else
        {
            this->StartPan();
        }
    }
    else
    {
        if (this->Interactor->GetControlKey())
        {
            // 禁用默认旋转交互
            //this->StartSpin();
        }
        else
        {
            // 禁用默认旋转交互
            //this->StartRotate();
        }
    }
}

void ImageInteractorStyle::OnLeftButtonUp()
{
    this->Superclass::OnLeftButtonUp();
}

void ImageInteractorStyle::OnMiddleButtonDown()
{
    this->Superclass::OnMiddleButtonDown();
}

void ImageInteractorStyle::OnMiddleButtonUp()
{
    this->Superclass::OnMiddleButtonUp();
}

void ImageInteractorStyle::OnRightButtonDown()
{
    this->Superclass::OnRightButtonDown();
}

void ImageInteractorStyle::OnRightButtonUp()
{
    this->Superclass::OnRightButtonUp();
}

void ImageInteractorStyle::OnMouseWheelForward()
{
    this->Superclass::OnMouseWheelForward();
}

void ImageInteractorStyle::OnMouseWheelBackward()
{
    this->Superclass::OnMouseWheelBackward();
}

void ImageInteractorStyle::setRenderWindow(vtkGenericOpenGLRenderWindow* renderWindow)
{
    m_pRenderWindow = renderWindow;
}

vtkGenericOpenGLRenderWindow* ImageInteractorStyle::getRenderWindow()
{
    return m_pRenderWindow;
}

void ImageInteractorStyle::setSecondRenderer(vtkRenderer* renderer)
{
    m_pSecondRenderer = renderer;
}

vtkRenderer* ImageInteractorStyle::getSecondRenderer()
{
    return m_pSecondRenderer;
}
