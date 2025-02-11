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

void ImageInteractorStyle::OnMouseMove()
{
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    switch (this->State)
    {
    case VTKIS_ENV_ROTATE:
    {
        this->FindPokedRenderer(x, y);
        this->EnvironmentRotate();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    }break;
    case VTKIS_ROTATE:
    {
        this->FindPokedRenderer(x, y);
        this->Rotate();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    }break;
    case VTKIS_PAN:
    {
        this->FindPokedRenderer(x, y);
        this->Pan();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    }break;
    case VTKIS_DOLLY:
    {
        this->FindPokedRenderer(x, y);
        this->Dolly();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    }break;
    case VTKIS_SPIN:
    {
        this->FindPokedRenderer(x, y);
        this->Spin();
        this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    }break;
    }
}

void ImageInteractorStyle::OnLeftButtonUp()
{
    switch (this->State)
    {
    case VTKIS_DOLLY:
    {
        this->EndDolly();
    }break;


    case VTKIS_PAN:
    {
        this->EndPan();
    }break;

    case VTKIS_SPIN:
    {
        this->EndSpin();
    }break;

    case VTKIS_ROTATE:
    {
        this->EndRotate();
    }break;
    }

    if (this->Interactor)
    {
        this->ReleaseFocus();
    }
}

void ImageInteractorStyle::OnMiddleButtonDown()
{
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    this->FindPokedRenderer(x, y);

    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);

    this->StartPan();
}

void ImageInteractorStyle::OnMiddleButtonUp()
{
    switch (this->State)
    {
    case VTKIS_PAN:
    {
        this->EndPan();
        if (this->Interactor)
        {
            this->ReleaseFocus();
        }
    }break;
    }
}

void ImageInteractorStyle::OnRightButtonDown()
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
        this->StartEnvRotate();
    }
    else
    {
        this->StartDolly();
    }
}

void ImageInteractorStyle::OnRightButtonUp()
{
    switch (this->State)
    {
    case VTKIS_ENV_ROTATE:
    {
        this->EndEnvRotate();
    }break;
    case VTKIS_DOLLY:
    {
        this->EndDolly();
    }break;
    }

    if (this->Interactor)
    {
        this->ReleaseFocus();
    }
}

void ImageInteractorStyle::OnMouseWheelForward()
{
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    this->FindPokedRenderer(x, y);

    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    this->StartDolly();
    double factor = this->MotionFactor * 0.2 * this->MouseWheelMotionFactor;
    this->Dolly(pow(1.1, factor));
    this->EndDolly();
    this->ReleaseFocus();
}

void ImageInteractorStyle::OnMouseWheelBackward()
{
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    this->FindPokedRenderer(x, y);

    if (this->CurrentRenderer == nullptr)
    {
        return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    this->StartDolly();
    double factor = this->MotionFactor * -0.2 * this->MouseWheelMotionFactor;
    this->Dolly(pow(1.1, factor));
    this->EndDolly();
    this->ReleaseFocus();
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
