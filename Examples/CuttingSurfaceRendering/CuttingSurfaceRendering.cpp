#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolume16Reader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkOutlineFilter.h>
#include <vtkCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkStripper.h>
#include <vtkPolyDataNormals.h>
#include <vtkContourFilter.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkSphereSource.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkCommand.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkImplicitPlaneRepresentation.h>

class vtkIPWCallback : public vtkCommand
{
public:
	static vtkIPWCallback *New() 
	{ 
		return new vtkIPWCallback; 
	}
	virtual void Execute(vtkObject *caller, unsigned long, void*)
	{
		vtkImplicitPlaneWidget2 *planeWidget = 
			reinterpret_cast<vtkImplicitPlaneWidget2*>(caller);
		vtkImplicitPlaneRepresentation *rep = 
			reinterpret_cast<vtkImplicitPlaneRepresentation*>(planeWidget->GetRepresentation());
		rep->GetPlane(this->Plane);
	}
	vtkIPWCallback():Plane(0),Actor(0) {}
	vtkPlane *Plane;
	vtkActor *Actor;

};

int main(int argc, char *argv[])
{
	//===================读入切片数据================
	vtkVolume16Reader *v16 = vtkVolume16Reader::New();
	v16->SetDataDimensions (64,64);
	v16->SetImageRange (1,93);
	v16->SetDataByteOrderToLittleEndian();
	v16->SetFilePrefix ("..\\headsq\\quarter");
	v16->SetDataSpacing (3.2, 3.2, 1.5);

	//======================skinExtractor=======================
	vtkContourFilter *skinExtractor = vtkContourFilter::New();
    	skinExtractor->SetInputConnection(v16->GetOutputPort());
    	skinExtractor->SetValue(0, 500);

	//========================skinNormals=======================
	vtkPolyDataNormals *skinNormals = vtkPolyDataNormals::New();
    	skinNormals->SetInputConnection(skinExtractor->GetOutputPort());
    	skinNormals->SetFeatureAngle(60.0);

	//=======================skinStripper=======================
	vtkSmartPointer<vtkStripper> skinStripper = vtkSmartPointer<vtkStripper>::New();
	skinStripper->SetInputConnection(skinNormals->GetOutputPort());

	//=======================skinMapper=========================
	vtkSmartPointer<vtkPolyDataMapper> skinMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	skinMapper->SetInputConnection(skinStripper->GetOutputPort());
	skinMapper->ScalarVisibilityOff();

	//==========================skin============================
	vtkSmartPointer<vtkActor> skin = vtkSmartPointer<vtkActor>::New();
	skin->SetMapper(skinMapper);
	skin->GetProperty()->SetDiffuseColor(1, .49, .25);
	skin->GetProperty()->SetSpecular(.3);
	skin->GetProperty()->SetSpecularPower(20);
	skin->GetProperty()->SetOpacity(.5);

	//======================boneExtractor=======================
	vtkSmartPointer<vtkContourFilter> boneExtractor = vtkSmartPointer<vtkContourFilter>::New();
	boneExtractor->SetInputConnection(v16->GetOutputPort());
	boneExtractor->SetValue(0, 1150);

	//========================boneNormals=======================
	vtkSmartPointer<vtkPolyDataNormals> boneNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
	boneNormals->SetInputConnection(boneExtractor->GetOutputPort());
	boneNormals->SetFeatureAngle(60.0);

	//=======================boneStripper=======================
	vtkSmartPointer<vtkStripper> boneStripper = vtkSmartPointer<vtkStripper>::New();
	boneStripper->SetInputConnection(boneNormals->GetOutputPort());

	//=======================boneMapper=========================
	vtkSmartPointer<vtkPolyDataMapper> boneMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	boneMapper->SetInputConnection(boneStripper->GetOutputPort());
	boneMapper->ScalarVisibilityOff();

	//==========================bone============================
	vtkSmartPointer<vtkActor> bone = vtkSmartPointer<vtkActor>::New();
	bone->SetMapper(boneMapper);
	bone->GetProperty()->SetDiffuseColor(1, 1, .9412);

	//=============================================


	//=========================绘制边框=========================
	vtkSmartPointer<vtkOutlineFilter> outlineData = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineData->SetInputConnection(v16->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapOutline = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapOutline->SetInputConnection(outlineData->GetOutputPort());

	vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
	outline->SetMapper(mapOutline);
	outline->GetProperty()->SetColor(0,255,0);


	//=========================================
	vtkSmartPointer<vtkPlane> plane =vtkSmartPointer<vtkPlane>::New();
	vtkSmartPointer<vtkClipPolyData> clipper =  vtkSmartPointer<vtkClipPolyData>::New();
	clipper->SetClipFunction(plane);
	clipper->InsideOutOn();

	clipper->SetInputConnection(skinNormals->GetOutputPort());
	//clipper->SetInputConnection(boneNormals->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> datMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	datMapper->ScalarVisibilityOff();
	datMapper->SetInputConnection(clipper->GetOutputPort());
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(datMapper);

	vtkSmartPointer<vtkProperty> backFaces =vtkSmartPointer<vtkProperty>::New();
	backFaces->SetDiffuseColor(.8, .8, .4);
	actor->SetBackfaceProperty(backFaces);
 

	//=========================设置相机=========================
	vtkCamera *aCamera = vtkCamera::New();
	aCamera->SetViewUp (0, 0, -1);
	aCamera->SetPosition (0, 1, 0);
	aCamera->SetFocalPoint (0, 0, 0);
	aCamera->ComputeViewPlaneNormal();


	//===============创建render、renderWindow、renderWindowInteractor===============
	vtkSmartPointer<vtkRenderWindow> renderWin= vtkSmartPointer<vtkRenderWindow>::New();
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

	renderer->AddActor(outline);
	renderer->AddActor(actor);

	renderWin->AddRenderer(renderer);

	renderer->SetActiveCamera(aCamera);
	renderer->ResetCamera();

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWin);

	//=========================绘制切割平面=========================
	vtkSmartPointer<vtkImplicitPlaneRepresentation> rep = vtkSmartPointer<vtkImplicitPlaneRepresentation>::New();
	rep->SetPlaceFactor(1.25); 
	rep->PlaceWidget(actor->GetBounds());
	rep->SetNormal(plane->GetNormal());
	rep->SetOrigin(100,0,500); 

	vtkSmartPointer<vtkIPWCallback> myCallback = vtkSmartPointer<vtkIPWCallback>::New();
	myCallback->Plane = plane;
	myCallback->Actor = actor;

	vtkSmartPointer<vtkImplicitPlaneWidget2> planeWidget = vtkSmartPointer<vtkImplicitPlaneWidget2>::New();
	planeWidget->SetInteractor(renderWindowInteractor);
	planeWidget->SetRepresentation(rep);
	planeWidget->AddObserver(vtkCommand::InteractionEvent,myCallback);
	planeWidget->On();

	//=========================开始绘制=========================
	renderWindowInteractor->Initialize();
	renderWin->Render();
	renderWin->SetSize(640, 480);
	planeWidget->On();
	renderWindowInteractor->Start();
	return EXIT_SUCCESS;
}