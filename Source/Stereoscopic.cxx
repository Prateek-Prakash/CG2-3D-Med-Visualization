#define ERROR_FILE "Errors.txt"

#define RAW_FILE "Data.raw"

#define CAMERA_OFFSET 35.0
#define CAMERA_DISTANCE 750.0

#define DATA_X 512
#define DATA_Y 512
#define DATA_Z 160

#define SPACING_X 0.5
#define SPACING_Y 0.5
#define SPACING_Z 1.00002

#define SCROLL_S 10.0

#include "Stereoscopic.h"
#include "ui_Stereoscopic.h"

#include <vtkNew.h.>
#include <vtkSmartPointer.h>
#include <vtkOutputWindow.h>
#include <vtkFileOutputWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkInteractorStyleJoystickActor.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkImageReader.h>
#include <vtkImageData.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkTransform.h>
#include <vtkVolumePicker.h>
#include <vtkExtractVOI.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>
#include <vtkPlot.h>
#include <vtkPlotPoints.h>
#include <vtkAxis.h>
#include <vtkChartMatrix.h>
#include <vtkChart.h>

vtkSmartPointer<vtkRenderWindow> renWin;
vtkSmartPointer<vtkRenderWindowInteractor> interactor;
vtkNew<vtkRenderer> leftRen, rightRen;
vtkNew<vtkCamera> leftCam, rightCam;
vtkNew<vtkImageReader> imageReader;
vtkNew<vtkImageData> imageData;
vtkNew<vtkSmartVolumeMapper> volumeMapper, eMapper;
vtkNew<vtkVolumeProperty> volumeProperty;
vtkNew<vtkPiecewiseFunction> opacityTransferFunction;
vtkNew<vtkColorTransferFunction> colorTransferFunction;
vtkNew<vtkVolume> volume, eVolume;
vtkNew<vtkTransform> transform, eTransform;
vtkNew<vtkExtractVOI> eVOI;
vtkNew<vtkVolumePicker> picker;
vtkNew<vtkTable> opacityTable;

double centerX = 0.0;
double centerY = 0.0;
double centerZ = 0.0;

bool rightButtonDown = false;
bool validSelection = false;
bool usingSelection = false;

double pickCoords[3];
int xMin, xMax, yMin, yMax;

void resetModel(vtkTransform *T);
void showSelection(int xMin, int xMax, int yMin, int yMax, int zMin, int zMax);

class trackballStyle : public vtkInteractorStyleTrackballActor
{
public:
	static trackballStyle* New();
	vtkTypeMacro(trackballStyle, vtkInteractorStyleTrackballActor);

	virtual void OnRightButtonDown()
	{
		rightButtonDown = true;

		int *winDim = renWin->GetSize();

		picker->SetTolerance(0.0005);
		picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, leftRen.GetPointer());
		picker->GetPickPosition(pickCoords);
		xMin = (pickCoords[0] + (DATA_X / 4)) * 2;
		yMin = (pickCoords[1] + (DATA_Y / 4)) * 2;
		
		if (this->Interactor->GetEventPosition()[0] < (winDim[0] / 2))
		{
			validSelection = true;
		}
		else
		{
			validSelection = false;
		}
	}

	virtual void OnRightButtonUp()
	{
		rightButtonDown = false;

		if (validSelection)
		{
			int tempNum = 0;
			if (xMin > xMax)
			{
				tempNum = xMin;
				xMin = xMax;
				xMax = tempNum;
			}
			if (yMin > yMax)
			{
				tempNum = yMin;
				yMin = yMax;
				yMax = tempNum;
			}
			showSelection(xMin, xMax, yMin, yMax, 0, DATA_Z);
		}
	}

	virtual void OnMouseMove()
	{
		if (rightButtonDown)
		{
			int *winDim = renWin->GetSize();

			picker->SetTolerance(0.0005);
			picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, leftRen.GetPointer());
			picker->GetPickPosition(pickCoords);
			xMax = (pickCoords[0] + (DATA_X / 4)) * 2;
			yMax = (pickCoords[1] + (DATA_Y / 4)) * 2;

			if (this->Interactor->GetEventPosition()[0] < (winDim[0] / 2))
			{
				validSelection = true;
			}
			else
			{
				validSelection = false;
			}
		}
		vtkInteractorStyleTrackballActor::OnMouseMove();
	}

	virtual void OnMouseWheelForward()
	{
		if (!usingSelection)
		{
			transform->Translate(0.0, 0.0, SCROLL_S);
		}
		else
		{
			eTransform->Translate(0.0, 0.0, SCROLL_S);
		}

		renWin->Render();
	}

	virtual void OnMouseWheelBackward()
	{
		if (!usingSelection)
		{
			transform->Translate(0.0, 0.0, -SCROLL_S);
		}
		else
		{
			eTransform->Translate(0.0, 0.0, -SCROLL_S);
		}

		renWin->Render();
	}

	virtual void OnKeyPress()
	{
		QString keySym = this->Interactor->GetKeySym();

		if (keySym == "Up")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "Down")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
				transform->RotateY(180.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
				eTransform->RotateY(180.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "Left")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
				transform->RotateY(-90.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
				eTransform->RotateY(-90.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "Right")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
				transform->RotateY(90.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
				eTransform->RotateY(90.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "r" || keySym == "R")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
			}
			else
			{
				resetModel(eTransform.GetPointer());
			}
			renWin->Render();
		}
	}

	virtual void OnChar()
	{
		NULL;
	}
};
vtkStandardNewMacro(trackballStyle);
vtkNew<trackballStyle> tStyle;

class joystickStyle : public vtkInteractorStyleJoystickActor
{
public:
	static joystickStyle* New();
	vtkTypeMacro(joystickStyle, vtkInteractorStyleJoystickActor);

	virtual void OnRightButtonDown()
	{
		rightButtonDown = true;

		int *winDim = renWin->GetSize();

		picker->SetTolerance(0.0005);
		picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, leftRen.GetPointer());
		picker->GetPickPosition(pickCoords);
		xMin = (pickCoords[0] + (DATA_X / 4)) * 2;
		yMin = (pickCoords[1] + (DATA_Y / 4)) * 2;

		if (this->Interactor->GetEventPosition()[0] < (winDim[0] / 2))
		{
			validSelection = true;
		}
		else
		{
			validSelection = false;
		}
	}

	virtual void OnRightButtonUp()
	{
		rightButtonDown = false;

		if (validSelection)
		{
			int tempNum = 0;
			if (xMin > xMax)
			{
				tempNum = xMin;
				xMin = xMax;
				xMax = tempNum;
			}
			if (yMin > yMax)
			{
				tempNum = yMin;
				yMin = yMax;
				yMax = tempNum;
			}
			showSelection(xMin, xMax, yMin, yMax, 0, DATA_Z);
		}
	}

	virtual void OnMouseMove()
	{
		if (rightButtonDown)
		{
			int *winDim = renWin->GetSize();

			picker->SetTolerance(0.0005);
			picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, leftRen.GetPointer());
			picker->GetPickPosition(pickCoords);
			xMax = (pickCoords[0] + (DATA_X / 4)) * 2;
			yMax = (pickCoords[1] + (DATA_Y / 4)) * 2;

			if (this->Interactor->GetEventPosition()[0] < (winDim[0] / 2))
			{
				validSelection = true;
			}
			else
			{
				validSelection = false;
			}
		}
		vtkInteractorStyleJoystickActor::OnMouseMove();
	}

	virtual void OnMouseWheelForward()
	{
		if (!usingSelection)
		{
			transform->Translate(0.0, 0.0, SCROLL_S);
		}
		else
		{
			eTransform->Translate(0.0, 0.0, SCROLL_S);
		}

		renWin->Render();
	}

	virtual void OnMouseWheelBackward()
	{
		if (!usingSelection)
		{
			transform->Translate(0.0, 0.0, -SCROLL_S);
		}
		else
		{
			eTransform->Translate(0.0, 0.0, -SCROLL_S);
		}

		renWin->Render();
	}

	virtual void OnKeyPress()
	{
		QString keySym = this->Interactor->GetKeySym();

		if (keySym == "Up")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "Down")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
				transform->RotateY(180.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
				eTransform->RotateY(180.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "Left")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
				transform->RotateY(-90.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
				eTransform->RotateY(-90.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "Right")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
				transform->RotateX(90.0);
				transform->RotateY(90.0);
			}
			else
			{
				resetModel(eTransform.GetPointer());
				eTransform->RotateX(90.0);
				eTransform->RotateY(90.0);
			}
			
			renWin->Render();
		}
		else if (keySym == "r" || keySym == "R")
		{
			if (!usingSelection)
			{
				resetModel(transform.GetPointer());
			}
			else
			{
				resetModel(eTransform.GetPointer());
			}
			renWin->Render();
		}
	}

	virtual void OnChar()
	{
		NULL;
	}
};
vtkStandardNewMacro(joystickStyle);
vtkNew<joystickStyle> jStyle;

Stereoscopic::Stereoscopic()
{
	this->ui = new Ui_Stereoscopic;
	this->ui->setupUi(this);

	setFileOutputWindow();
	getRenderWindow();
	getInteractor();
	setTrackballInteractor();
	setRenderers();
	setCameras();
	readData();
	createVolume();
	setTransform();
	createOTFChart();

	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));

	connect(this->ui->actionTrackball, SIGNAL(triggered()), this, SLOT(slotTrackball()));
	connect(this->ui->actionJoystick, SIGNAL(triggered()), this, SLOT(slotJoystick()));

	connect(this->ui->actionFull, SIGNAL(triggered()), this, SLOT(slotFull()));
	connect(this->ui->actionSelected, SIGNAL(triggered()), this, SLOT(slotSelected()));

	connect(this->ui->actionOTF, SIGNAL(triggered()), this, SLOT(slotOTF()));
}

void Stereoscopic::setFileOutputWindow()
{
	vtkNew<vtkOutputWindow> outWin;
	vtkNew<vtkFileOutputWindow> fileOut;
	fileOut->SetFileName(ERROR_FILE);
	outWin->SetInstance(fileOut.GetPointer());
}

void Stereoscopic::getRenderWindow()
{
	renWin = this->ui->vtkWidget->GetRenderWindow();
}

void Stereoscopic::getInteractor()
{
	interactor = renWin->GetInteractor();
}

void Stereoscopic::setTrackballInteractor()
{
	interactor->SetInteractorStyle(tStyle.GetPointer());
}

void Stereoscopic::setJoystickInteractor()
{
	interactor->SetInteractorStyle(jStyle.GetPointer());
}

void Stereoscopic::setRenderers()
{
	double leftView[4] = { 0.0, 0.0, 0.5, 1.0 };
	double rightView[4] = { 0.5, 0.0, 1.0, 1.0 };

	renWin->AddRenderer(leftRen.GetPointer());
	leftRen->SetViewport(leftView);

	renWin->AddRenderer(rightRen.GetPointer());
	rightRen->SetViewport(rightView);
}

void Stereoscopic::setCameras()
{
	leftCam->SetPosition(-CAMERA_OFFSET, 0.0, CAMERA_DISTANCE);
	leftCam->SetFocalPoint(0.0, 0.0, 0.0);
	leftCam->SetClippingRange(0.1, 10000);

	rightCam->SetPosition(CAMERA_OFFSET, 0.0, CAMERA_DISTANCE);
	rightCam->SetFocalPoint(0.0, 0.0, 0.0);
	rightCam->SetClippingRange(0.1, 10000);

	leftRen->SetActiveCamera(leftCam.GetPointer());
	rightRen->SetActiveCamera(rightCam.GetPointer());
}

void Stereoscopic::readData()
{
	imageReader->SetFileName(RAW_FILE);
	imageReader->SetFileDimensionality(3);
	imageReader->SetDataExtent(0, (DATA_X - 1), 0, (DATA_Y - 1), 0, (DATA_Z - 1));
	imageReader->SetDataSpacing(SPACING_X, SPACING_Y, SPACING_Z);
	imageReader->SetDataScalarTypeToUnsignedChar();
	imageReader->Update();

	imageData->DeepCopy(imageReader->GetOutput());

	centerX = imageData->GetCenter()[0];
	centerY = imageData->GetCenter()[1];
	centerZ = imageData->GetCenter()[2];
}

void Stereoscopic::createVolume()
{
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetInputData(imageData.GetPointer());

	volumeProperty->ShadeOn();
	volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

	volumeProperty->SetScalarOpacity(opacityTransferFunction.GetPointer());

	colorTransferFunction->AddRGBSegment(0.0, 0.0, 0.0, 0.0, 68.0, 0.0, 0.0, 0.0);
	colorTransferFunction->AddRGBSegment(69.0, 1.0, 0.8, 0.75, 200.0, 0.8, 0.6, 0.2);
	colorTransferFunction->AddRGBSegment(220.0, 1.0, 0.0, 0.0, 255.0, 1.0, 0.0, 0.0);
	volumeProperty->SetColor(colorTransferFunction.GetPointer());

	volume->SetMapper(volumeMapper.GetPointer());
	volume->SetProperty(volumeProperty.GetPointer());

	leftRen->AddActor(volume.GetPointer());
	rightRen->AddActor(volume.GetPointer());
}

void Stereoscopic::setTransform()
{
	transform->PostMultiply();
	resetModel(transform.GetPointer());
	volume->SetUserTransform(transform.GetPointer());
}

void Stereoscopic::createOTFChart()
{
	// Set Window Title
	opacityWidget->setWindowTitle("OTF");

	// Set Table Data
	vtkNew<vtkDoubleArray> arrX;
	arrX->SetName("Intensity");
	opacityTable->AddColumn(arrX.GetPointer());
	vtkNew<vtkDoubleArray> arrY;
	arrY->SetName("Transparency");
	opacityTable->AddColumn(arrY.GetPointer());
	opacityTable->SetNumberOfRows(10);
	setOTFValues();

	// Connect View & VTK Widget
	vtkNew<vtkContextView> opacityView;
	opacityView->SetInteractor(opacityWidget->GetInteractor());
	opacityWidget->SetRenderWindow(opacityView->GetRenderWindow());

	// Experimental
	vtkNew<vtkChartMatrix> chartMatrix;
	opacityView->GetScene()->AddItem(chartMatrix.GetPointer());
	chartMatrix->SetSize(vtkVector2i(2, 1));
	vtkChart *opacityChart = chartMatrix->GetChart(vtkVector2i(0, 0));

	// Create Plot From Table Data
	vtkPlot *opacityPlot = opacityChart->AddPlot(vtkChart::LINE);
	opacityPlot->SetInputData(opacityTable.GetPointer(), 0, 1);
	opacityPlot->SetColor(0, 0, 0, 255);
	opacityPlot->SetWidth(1.0);

	// Set Points To Circles
	vtkPlotPoints::SafeDownCast(opacityPlot)->SetMarkerStyle(vtkPlotPoints::CIRCLE);
	opacityPlot->SetColor(0, 0, 0, 255);

	// Set Axis Titles
	opacityChart->GetAxis(vtkAxis::LEFT)->SetTitle("Transparency");
	opacityChart->GetAxis(vtkAxis::BOTTOM)->SetTitle("Intensity");
}

void Stereoscopic::setOTFValues()
{
	addOTFPoint(0.0, 0.0, 0);
	addOTFPoint(69.0, 0.0, 1);
	addOTFPoint(80.0, 0.2, 2);
	addOTFPoint(100.0, 0.0, 3);
	addOTFPoint(120.0, 0.8, 4);
	addOTFPoint(200.0, 0.0, 5);
	addOTFPoint(220.0, 0.0, 6);
	addOTFPoint(245.0, 0.2, 7);
	addOTFPoint(250.0, 0.5, 8);
	addOTFPoint(255.0, 1.0, 9);
}

void Stereoscopic::addOTFPoint(double intensity, double transparency, int index)
{
	opacityTransferFunction->AddPoint(intensity, transparency);
	opacityTable->SetValue(index, 0, intensity);
	opacityTable->SetValue(index, 1, transparency);
}

void Stereoscopic::slotExit()
{
	qApp->exit();
}

void Stereoscopic::slotTrackball()
{
	this->setTrackballInteractor();
}

void Stereoscopic::slotJoystick()
{
	this->setJoystickInteractor();
}

void Stereoscopic::slotFull()
{
	usingSelection = false;

	leftRen->RemoveActor(eVolume.GetPointer());
	rightRen->RemoveActor(eVolume.GetPointer());

	leftRen->AddActor(volume.GetPointer());
	rightRen->AddActor(volume.GetPointer());

	renWin->Render();
}

void Stereoscopic::slotSelected()
{
	usingSelection = true;

	leftRen->RemoveActor(volume.GetPointer());
	rightRen->RemoveActor(volume.GetPointer());

	leftRen->AddActor(eVolume.GetPointer());
	rightRen->AddActor(eVolume.GetPointer());

	renWin->Render();
}

void Stereoscopic::slotOTF()
{	
	opacityWidget->setWindowState(Qt::WindowNoState);
	opacityWidget->move(0, 0);
	opacityWidget->show();
}

void resetModel(vtkTransform *T)
{
	T->Identity();
	T->Translate(-centerX, -centerY, -centerZ);
}

void showSelection(int xMin, int xMax, int yMin, int yMax, int zMin, int zMax)
{
	if (!usingSelection)
	{
		usingSelection = true;

		eVOI->SetInputData(imageData.GetPointer());
		eVOI->SetVOI(xMin, xMax, yMin, yMax, zMin, zMax);
		eVOI->Update();

		eMapper->SetBlendModeToComposite();
		eMapper->SetInputData(eVOI->GetOutput());

		eVolume->SetMapper(eMapper.GetPointer());
		eVolume->SetProperty(volumeProperty.GetPointer());

		eTransform->PostMultiply();
		eTransform->Identity();
		eTransform->Translate(-centerX, -centerY, -centerZ);

		eVolume->SetUserTransform(eTransform.GetPointer());

		leftRen->RemoveActor(volume.GetPointer());
		rightRen->RemoveActor(volume.GetPointer());

		leftRen->AddActor(eVolume.GetPointer());
		rightRen->AddActor(eVolume.GetPointer());

		renWin->Render();
	}
}