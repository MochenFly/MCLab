#include "Render2DWidget.h"

#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkInteractorStyleImage.h>

Render2DWidget::Render2DWidget(QWidget* parent, Qt::WindowFlags f)
    : QVTKOpenGLNativeWidget(parent, f)
{
    initialise();

    setInteractorStyle(InteractorStyle_None);
}

bool Render2DWidget::loadPNGFile(const char* filePath)
{
    bool canRead = m_pPNGReader->CanReadFile(filePath);
    if (canRead)
    {
        m_pPNGReader->SetFileName(filePath);
        m_pPNGReader->Update();

        m_pImageActor->GetMapper()->SetInputData(m_pPNGReader->GetOutput());

        if (!m_pRenderer->HasViewProp(m_pImageActor))
        {
            m_pRenderer->AddActor(m_pImageActor);
        }

        m_pRenderer->ResetCamera();

        m_pRenderWindow->Render();
    }
    return canRead;
}

void Render2DWidget::setInteractorStyle(const InteractorStyleType& type)
{
    switch (type)
    {
    case InteractorStyleType::InteractorStyle_None:
    {
        m_pRenderWindow->GetInteractor()->SetInteractorStyle(nullptr);
    }break;
    case InteractorStyleType::InteractorStyle_Image:
    {
        m_pRenderWindow->GetInteractor()->SetInteractorStyle(m_pImageInteractorStyle);
    }break;
    case InteractorStyleType::Selection_2D_Rectangle:
    {
        m_pRenderWindow->GetInteractor()->SetInteractorStyle(m_pRectangleImageInteractorStyle);
    }break;
    case InteractorStyleType::Selection_2D_Ellipse:
    {
        m_pRenderWindow->GetInteractor()->SetInteractorStyle(m_pEllipseImageInteractorStyle);
    }break;
    }
}

void Render2DWidget::initialise()
{
    m_pCamera = vtkSmartPointer<vtkCamera>::New();
    m_pCamera->ParallelProjectionOn();

    m_pRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_pRenderer->SetBackground(1, 1, 1);
    m_pRenderer->SetActiveCamera(m_pCamera);

    m_pSecondRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_pSecondRenderer->SetBackground(1, 1, 1);
    m_pSecondRenderer->SetActiveCamera(m_pCamera);

    m_pRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_pRenderWindow->AddRenderer(m_pRenderer);
    m_pRenderWindow->AddRenderer(m_pSecondRenderer);

    m_pRenderWindow->SetNumberOfLayers(2);
    m_pRenderer->SetLayer(0);
    m_pSecondRenderer->SetLayer(1);

    this->setRenderWindow(m_pRenderWindow);

    m_pImageInteractorStyle = vtkSmartPointer<ImageInteractorStyle>::New();
    m_pImageInteractorStyle->SetDefaultRenderer(m_pRenderer);
    m_pImageInteractorStyle->setRenderWindow(m_pRenderWindow);

    m_pRectangleImageInteractorStyle = vtkSmartPointer<RectangleImageInteractorStyle>::New();
    m_pRectangleImageInteractorStyle->SetDefaultRenderer(m_pRenderer);
    m_pRectangleImageInteractorStyle->setRenderWindow(m_pRenderWindow);
    m_pRectangleImageInteractorStyle->setSecondRenderer(m_pSecondRenderer);

    m_pEllipseImageInteractorStyle = vtkSmartPointer<EllipseImageInteractorStyle>::New();
    m_pEllipseImageInteractorStyle->SetDefaultRenderer(m_pRenderer);
    m_pEllipseImageInteractorStyle->setRenderWindow(m_pRenderWindow);
    m_pEllipseImageInteractorStyle->setSecondRenderer(m_pSecondRenderer);

    m_pPNGReader = vtkSmartPointer<vtkPNGReader>::New();

    m_pImageActor = vtkSmartPointer<vtkImageActor>::New();
}
