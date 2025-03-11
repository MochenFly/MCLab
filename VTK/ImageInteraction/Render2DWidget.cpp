#include "Render2DWidget.h"

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkProperty.h>

#include <QDebug>

Render2DWidget::Render2DWidget(QWidget* parent, Qt::WindowFlags f)
    : QVTKOpenGLNativeWidget(parent, f)
{
    initialise();

    setInteractorStyle(InteractorStyle_None);
}

bool Render2DWidget::loadPNGFile()
{
    double spacing[3] = { 0, 0, 0 };
    double origin[3] = { 0, 0, 0 };
    double bound[6] = { 0, 0, 0, 0, 0, 0 };
    int dimensions[3] = { 0, 0, 0 };

    const char* filePath = "../Data/WuKong.png";
    vtkSmartPointer<vtkPNGReader> pPNGReader = vtkSmartPointer<vtkPNGReader>::New();
    bool canRead = pPNGReader->CanReadFile(filePath);
    if (canRead)
    {
        pPNGReader->SetFileName(filePath);
        pPNGReader->Update();

        vtkImageData* pImageData = pPNGReader->GetOutput();
        
        vtkSmartPointer<vtkImageActor> pImageActor = vtkSmartPointer<vtkImageActor>::New();
        pImageActor->GetMapper()->SetInputData(pImageData);

        m_pRenderer->AddActor(pImageActor);

        m_pRenderer->ResetCamera();
        m_pRenderWindow->Render();

        pImageData->GetSpacing(spacing);
        pImageData->GetOrigin(origin);
        pImageData->GetBounds(bound);
        pImageData->GetDimensions(dimensions);
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
    case InteractorStyleType::InteractorStyle_ParametricSpline:
    {
        m_pRenderWindow->GetInteractor()->SetInteractorStyle(m_pParametricSplineInteractorStyle);
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

void Render2DWidget::drawSplineLine()
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0, 0, 0);
    points->InsertNextPoint(100, 100, 0);
    points->InsertNextPoint(200, 0, 0);
    points->InsertNextPoint(300, 100, 0);
    points->InsertNextPoint(400, 0, 0);

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    vtkIdType ids[5] = {0, 1, 2, 3, 4};
    lines->InsertNextCell(5, ids);

    vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
    data->SetPoints(points);
    data->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(data);
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1, 0, 0);
    m_pRenderer->AddActor(actor);

    vtkSmartPointer<vtkParametricSpline> spline = vtkSmartPointer<vtkParametricSpline>::New();
    spline->SetPoints(points);
    vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
    functionSource->SetParametricFunction(spline);
    functionSource->Update();

    vtkSmartPointer<vtkPolyData> splineData = functionSource->GetOutput();
    vtkSmartPointer<vtkPolyDataMapper> splineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    splineMapper->SetInputData(splineData);
    vtkSmartPointer<vtkActor> splineActor = vtkSmartPointer<vtkActor>::New();
    splineActor->SetMapper(splineMapper);
    splineActor->GetProperty()->SetColor(0, 1, 0);
    m_pRenderer->AddActor(splineActor);

    m_pRenderer->ResetCamera();
    m_pRenderWindow->Render();

    vtkPoints* splinePoints = splineData->GetPoints();
    for (vtkIdType i = 0; i < splinePoints->GetNumberOfPoints(); ++i)
    {
        double p[3];
        splinePoints->GetPoint(i, p);
        qDebug() << "Point ID: " << i << "  X: " << p[0] << "  Y: " << p[1] << "  Z: " << p[2];
    }
}

void Render2DWidget::clearData()
{
    m_pRenderer->RemoveAllViewProps();
    m_pSecondRenderer->RemoveAllViewProps();

    m_pRenderer->ResetCamera();
    m_pSecondRenderer->ResetCamera();

    m_pRenderWindow->Render();
}

void Render2DWidget::initialise()
{
    m_pCamera = vtkSmartPointer<vtkCamera>::New();
    m_pCamera->ParallelProjectionOn();

    m_pRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_pRenderer->SetBackground(0, 0, 0);
    m_pRenderer->SetActiveCamera(m_pCamera);
    m_pRenderer->ResetCamera();

    m_pSecondRenderer = vtkSmartPointer<vtkRenderer>::New();
    m_pSecondRenderer->SetBackground(1, 1, 1);
    m_pSecondRenderer->SetActiveCamera(m_pCamera);
    m_pSecondRenderer->ResetCamera();

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

    m_pParametricSplineInteractorStyle = vtkSmartPointer<ParametricSplineInteractorStyle>::New();
    m_pParametricSplineInteractorStyle->SetDefaultRenderer(m_pRenderer);

    m_pRectangleImageInteractorStyle = vtkSmartPointer<RectangleImageInteractorStyle>::New();
    m_pRectangleImageInteractorStyle->SetDefaultRenderer(m_pRenderer);
    m_pRectangleImageInteractorStyle->setRenderWindow(m_pRenderWindow);
    m_pRectangleImageInteractorStyle->setSecondRenderer(m_pSecondRenderer);

    m_pEllipseImageInteractorStyle = vtkSmartPointer<EllipseImageInteractorStyle>::New();
    m_pEllipseImageInteractorStyle->SetDefaultRenderer(m_pRenderer);
    m_pEllipseImageInteractorStyle->setRenderWindow(m_pRenderWindow);
    m_pEllipseImageInteractorStyle->setSecondRenderer(m_pSecondRenderer);

}
