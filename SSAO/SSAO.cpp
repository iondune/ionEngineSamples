
#include <ionWindow.h>
#include <ionGraphics.h>
#include <ionGraphicsGL.h>
#include <ionScene.h>
#include <ionApplication.h>

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


int main()
{
	////////////////////
	// ionEngine Init //
	////////////////////

	Log::AddDefaultOutputs();

	SingletonPointer<CGraphicsAPI> GraphicsAPI;
	SingletonPointer<CWindowManager> WindowManager;
	SingletonPointer<CTimeManager> TimeManager;
	SingletonPointer<CSceneManager> SceneManager;
	SingletonPointer<CAssetManager> AssetManager;

	GraphicsAPI->Init(new Graphics::COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);
	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);

	CWindow * Window = WindowManager->CreateWindow(vec2i(1600, 900), "DemoApplication", EWindowType::Windowed);

	AssetManager->SetAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Images/");

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> BackBuffer = Context->GetBackBuffer();
	BackBuffer->SetClearColor(color3f(0.3f));

	SharedPointer<IFrameBuffer> FrameBuffer = Context->CreateFrameBuffer();

	SharedPointer<ITexture2D> SceneTexture = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGBA, ITexture::EInternalFormatType::Fix8);
	SharedPointer<ITexture2D> SceneDepth = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::R, ITexture::EInternalFormatType::Depth);
	FrameBuffer->AttachColorTexture(SceneTexture, 0);
	FrameBuffer->AttachDepthTexture(SceneDepth);
	if (! FrameBuffer->CheckCorrectness())
	{
		Log::Error("Frame buffer not valid!");
	}


	/////////////////
	// Load Assets //
	/////////////////

	CSimpleMesh * SphereMesh = CGeometryCreator::CreateSphere();
	CSimpleMesh * PlaneMesh = CGeometryCreator::CreatePlane(vec2f(100.f));

	SharedPointer<IShaderProgram> DiffuseShader = AssetManager->LoadShader("Diffuse");
	SharedPointer<IShaderProgram> QuadCopyShader = AssetManager->LoadShader("QuadCopy");


	////////////////////
	// ionScene Setup //
	////////////////////

	CRenderPass * RenderPass = new CRenderPass(Context);
	RenderPass->SetRenderTarget(FrameBuffer);
	SceneManager->AddRenderPass(RenderPass);

	CRenderPass * PostProcess = new CRenderPass(Context);
	PostProcess->SetRenderTarget(BackBuffer);
	SceneManager->AddRenderPass(PostProcess);

	CPerspectiveCamera * Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(0, 3, -5));
	Camera->SetFocalLength(0.4f);
	RenderPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(15.f * Constants32::Pi / 48.f);
	Controller->SetPhi(-Constants32::Pi / 16.f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);


	/////////////////
	// Add Objects //
	/////////////////

	CSimpleMeshSceneObject * LightSphere1 = new CSimpleMeshSceneObject();
	LightSphere1->SetMesh(SphereMesh);
	LightSphere1->SetShader(DiffuseShader);
	LightSphere1->SetPosition(vec3f(0, 0, 0));
	LightSphere1->SetScale(2.f);
	RenderPass->AddSceneObject(LightSphere1);

	CSimpleMeshSceneObject * LightSphere2 = new CSimpleMeshSceneObject();
	LightSphere2->SetMesh(SphereMesh);
	LightSphere2->SetShader(DiffuseShader);
	LightSphere2->SetPosition(vec3f(4, 0, 0));
	LightSphere2->SetScale(3.f);
	RenderPass->AddSceneObject(LightSphere2);

	CSimpleMeshSceneObject * LightSphere3 = new CSimpleMeshSceneObject();
	LightSphere3->SetMesh(SphereMesh);
	LightSphere3->SetShader(DiffuseShader);
	LightSphere3->SetPosition(vec3f(12, 0, 0));
	LightSphere3->SetScale(4.f);
	RenderPass->AddSceneObject(LightSphere3);

	CSimpleMeshSceneObject * SpecularSphere = new CSimpleMeshSceneObject();
	SpecularSphere->SetMesh(SphereMesh);
	SpecularSphere->SetShader(DiffuseShader);
	SpecularSphere->SetPosition(vec3f(3, 0, 6));
	RenderPass->AddSceneObject(SpecularSphere);

	CSimpleMeshSceneObject * PlaneObject = new CSimpleMeshSceneObject();
	PlaneObject->SetMesh(PlaneMesh);
	PlaneObject->SetShader(DiffuseShader);
	RenderPass->AddSceneObject(PlaneObject);

	CSimpleMeshSceneObject * PostProcessObject = new CSimpleMeshSceneObject();
	PostProcessObject->SetMesh(CGeometryCreator::CreateScreenTriangle());
	PostProcessObject->SetShader(QuadCopyShader);
	PostProcessObject->SetTexture("uTexture", SceneTexture);
	PostProcess->AddSceneObject(PostProcessObject);

	CPointLight * Light1 = new CPointLight();
	Light1->SetPosition(vec3f(0, 6, 0));
	RenderPass->AddLight(Light1);


	///////////////
	// Main Loop //
	///////////////

	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();

		FrameBuffer->ClearColorAndDepth();
		BackBuffer->ClearColorAndDepth();
		SceneManager->DrawAll();
		Window->SwapBuffers();
	}

	return 0;
}
