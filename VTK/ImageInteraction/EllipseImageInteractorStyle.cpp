#include "EllipseImageInteractorStyle.h"

#include <vtkCellArray.h>
#include <vtkInteractorObserver.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(EllipseImageInteractorStyle);

EllipseImageInteractorStyle::EllipseImageInteractorStyle()
{
    m_pEllipseActor = vtkSmartPointer<vtkActor>::New();
    m_pEllipseActor->GetProperty()->SetColor(1, 0, 0);
}

EllipseImageInteractorStyle::~EllipseImageInteractorStyle()
{
}

void EllipseImageInteractorStyle::OnLeftButtonDown()
{
    if (this->Interactor->GetShiftKey())
    {
        this->Superclass::OnLeftButtonDown();
    }
    else
    {
        this->m_drawing = true;

        this->Interactor->GetEventPosition(this->m_startPos);
    }
}

void EllipseImageInteractorStyle::OnMouseMove()
{
    if (this->m_drawing)
    {
        int currentPos[2];
        this->Interactor->GetEventPosition(currentPos);

        double startWorldPos[4];
        vtkInteractorObserver::ComputeDisplayToWorld(this->GetDefaultRenderer(), this->m_startPos[0], this->m_startPos[1], 0, startWorldPos);

        double currentWorldPos[4];
        vtkInteractorObserver::ComputeDisplayToWorld(this->GetDefaultRenderer(), currentPos[0], currentPos[1], 0, currentWorldPos);

        double centerX = (startWorldPos[0] + currentWorldPos[0]) / 2.0;
        double centerY = (startWorldPos[1] + currentWorldPos[1]) / 2.0;

        double radiusX = std::abs(startWorldPos[0] - currentWorldPos[0]) / 2.0;
        double radiusY = std::abs(startWorldPos[1] - currentWorldPos[1]) / 2.0;

        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        int resolution = 100;
        for (int i = 0; i <= resolution; ++i)
        {
            double theta = 2.0 * vtkMath::Pi() * i / resolution;
            double x = centerX + radiusX * cos(theta);
            double y = centerY + radiusY * sin(theta);
            points->InsertNextPoint(x, y, 0);
        }

        vtkIdType vertexIndices[100 + 1];
        for (int i = 0; i <= resolution; i++)
        {
            vertexIndices[i] = static_cast<vtkIdType>(i);
        }

        vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
        lines->InsertNextCell(resolution + 1, vertexIndices);

        vtkNew<vtkPolyData> polyData;
        polyData->SetPoints(points);
        polyData->SetLines(lines);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(polyData);

        m_pEllipseActor->SetMapper(mapper);

        if (!this->getSecondRenderer()->HasViewProp(m_pEllipseActor))
        {
            this->getSecondRenderer()->AddActor(m_pEllipseActor);
        }

        this->getRenderWindow()->Render();
    }
    else
    {
        this->Superclass::OnMouseMove();
    }
}

void EllipseImageInteractorStyle::OnLeftButtonUp()
{
    this->m_drawing = false;

    this->m_startPos[0] = 0;
    this->m_startPos[1] = 0;

    this->Superclass::OnLeftButtonUp();
}
