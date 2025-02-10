#include "EllipseImageInteractorStyle.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(EllipseImageInteractorStyle);

EllipseImageInteractorStyle::EllipseImageInteractorStyle()
{
    m_pEllipseActor = vtkSmartPointer<vtkActor>::New();
}

EllipseImageInteractorStyle::~EllipseImageInteractorStyle()
{
}
