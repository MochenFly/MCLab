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

enum InteractorStyleType
{
    InteractorStyle_None,
    InteractorStyle_Image,
    Selection_2D_Rectangle,
    Selection_2D_Ellipse,
};

class Render2DWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    Render2DWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    bool loadPNGFile(const char* filePath);

    void setInteractorStyle(const InteractorStyleType& type);

private:
    void initialise();

private:
    vtkSmartPointer<vtkCamera>                          m_pCamera                               { nullptr };
    vtkSmartPointer<vtkRenderer>                        m_pRenderer                             { nullptr };
    vtkSmartPointer<vtkRenderer>                        m_pSecondRenderer                       { nullptr };
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>       m_pRenderWindow                         { nullptr };

    vtkSmartPointer<ImageInteractorStyle>               m_pImageInteractorStyle                 { nullptr };
    vtkSmartPointer<RectangleImageInteractorStyle>      m_pRectangleImageInteractorStyle        { nullptr };
    vtkSmartPointer<EllipseImageInteractorStyle>        m_pEllipseImageInteractorStyle          { nullptr };

    vtkSmartPointer<vtkPNGReader>                       m_pPNGReader                            { nullptr };

    vtkSmartPointer<vtkImageActor>                      m_pImageActor                           { nullptr };
};
