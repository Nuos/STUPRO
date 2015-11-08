#include "vtkPVStuproView.h"

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkInteractorStyle.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindow.h>
#include <cmath>

#include "MakeUnique.hpp"
#include "StuproInteractor.hpp"

vtkStandardNewMacro(vtkPVStuproView);
//----------------------------------------------------------------------------
vtkPVStuproView::vtkPVStuproView()
{
}

//----------------------------------------------------------------------------
vtkPVStuproView::~vtkPVStuproView()
{
}

//----------------------------------------------------------------------------
void vtkPVStuproView::Initialize(unsigned int id)
{
	this->Superclass::Initialize(id);

	initParameters();
	initRenderer();
	initCallbacks();
	initGlobe();
}

//----------------------------------------------------------------------------
void vtkPVStuproView::initParameters()
{
	// Initialize parameters.
	myDisplayMode = DisplayGlobe;
	myGlobeRadius = 0.5f;
	myPlaneSize = 1.f;
	myHeightFactor = 0.05f;
}

//----------------------------------------------------------------------------
void vtkPVStuproView::initRenderer()
{
	// Set fitting clipping range.
	float r1 = myGlobeRadius * 100.f;
	float r2 = 0.001f;
	GetRenderer()->ResetCameraClippingRange(r1, r2, r1, r2, r1, r2);

	// Create interactor for render window.
	vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(GetRenderWindow());

	// Create interactor style for render window.
	vtkSmartPointer<StuproInteractor> interactorStyle = StuproInteractor::New(this);
	interactorStyle->SetAutoAdjustCameraClippingRange(false);
	interactor->SetInteractorStyle(interactorStyle);

	// Zoom out.
	GetRenderWindow()->Render();
	interactorStyle->zoomWithFactor(-1800.f);
}

//----------------------------------------------------------------------------
void vtkPVStuproView::initCallbacks()
{
	// Create callback function that updates the display mode interpolation value.
	auto timerFunc = [](vtkObject* caller, unsigned long eventId, void* clientData, void* callData)
	{
		vtkPVStuproView& client = *((vtkPVStuproView*) clientData);

		// Determine target interpolation based on display mode.
		float interpolationTarget = client.myDisplayMode == DisplayGlobe ? 0.f : 1.f;

		// Check if change is significant enough to be re-rendered.
		if(std::abs(interpolationTarget - client.myDisplayModeInterpolation) > 0.000001f)
		{
			// Controls the speed of the globe-map transition.
			float effectSpeed = 2.f;

			// Smoothly transition interpolation value based on previous and target value.
			client.myGlobe->setDisplayModeInterpolation((interpolationTarget * effectSpeed +
				client.myGlobe->getDisplayModeInterpolation()) / (effectSpeed + 1.f));

			// Update renderer.
			client.GetRenderWindow()->Render();
		}
	};

	// Create and assign callback for the interpolation function.
	vtkSmartPointer<vtkCallbackCommand> timerCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	timerCallback->SetCallback(timerFunc);
	timerCallback->SetClientData(this);

	// Workaround for a VTK bug involving a missing Xt App Context causing a segfault in
	// CreateRepeatingTimer.
	GetRenderWindow()->Render();

	// Enable timer on the render window.
	GetRenderWindow()->GetInteractor()->CreateRepeatingTimer(17);
	GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::TimerEvent, timerCallback);
}

//----------------------------------------------------------------------------
void vtkPVStuproView::initGlobe()
{
	myGlobe = makeUnique<Globe>(*GetRenderer());
}

//----------------------------------------------------------------------------
void vtkPVStuproView::switchCurrentDisplayMode()
{
	GetActiveCamera()->SetPosition(0, 0, 2.8);
	GetActiveCamera()->SetFocalPoint(0, 0, 0);
	myDisplayMode = myDisplayMode == DisplayGlobe ? DisplayMap : DisplayGlobe;
	myGlobe->setDisplayModeInterpolation(myDisplayMode == DisplayGlobe ? 0.f : 1.f);

	GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
void vtkPVStuproView::PrintSelf(ostream& os, vtkIndent indent)
{
	this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
float vtkPVStuproView::getGlobeRadius()
{
	return this->myGlobeRadius;
}

//----------------------------------------------------------------------------
vtkPVStuproView::DisplayMode vtkPVStuproView::getDisplayMode()
{
	return this->myDisplayMode;
}