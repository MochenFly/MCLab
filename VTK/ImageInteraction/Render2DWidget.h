#pragma once

#include <QVTKOpenGLNativeWidget.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPNGReader.h>
#include <vtkImageActor.h>

#include "ImageInteractorStyle.h"
#include "RectangleImageInteractorStyle.h"
#include "EllipseImageInteractorStyle.h"
#include "ParametricSplineInteractorStyle.h"

enum InteractorStyleType
{
    InteractorStyle_None,
    InteractorStyle_Image,
    InteractorStyle_ParametricSpline,
    Selection_2D_Rectangle,
    Selection_2D_Ellipse,
};

class Render2DWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    Render2DWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    bool loadPNGFile();

    void setInteractorStyle(const InteractorStyleType& type);

    void drawSplineLine();

    void clearData();

private:
    void initialise();

private:
    vtkSmartPointer<vtkCamera>                          m_pCamera                               { nullptr };
    vtkSmartPointer<vtkRenderer>                        m_pRenderer                             { nullptr };
    vtkSmartPointer<vtkRenderer>                        m_pSecondRenderer                       { nullptr };
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>       m_pRenderWindow                         { nullptr };

    vtkSmartPointer<ImageInteractorStyle>               m_pImageInteractorStyle                 { nullptr };
    vtkSmartPointer<ParametricSplineInteractorStyle>    m_pParametricSplineInteractorStyle      { nullptr };
    vtkSmartPointer<RectangleImageInteractorStyle>      m_pRectangleImageInteractorStyle        { nullptr };
    vtkSmartPointer<EllipseImageInteractorStyle>        m_pEllipseImageInteractorStyle          { nullptr };
};
