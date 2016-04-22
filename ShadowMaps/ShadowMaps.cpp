
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

	GraphicsAPI->Init(new COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);
	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);

	CWindow * Window = WindowManager->CreateWindow(vec2i(1600, 900), "Shadow Maps", EWindowType::Windowed);

	AssetManager->SetAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Images/");

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> BackBuffer = Context->GetBackBuffer();
	BackBuffer->SetClearColor(color3f(0.3f));

	SharedPointer<IFrameBuffer> ShadowBuffer = Context->CreateFrameBuffer();

	SharedPointer<ITexture2D> ShadowTexture = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGBA, ITexture::EInternalFormatType::Fix8);
	SharedPointer<ITexture2D> ShadowDepth = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::R, ITexture::EInternalFormatType::Depth);
	ShadowBuffer->AttachColorTexture(ShadowTexture, 0);
	ShadowBuffer->AttachDepthTexture(ShadowDepth);
	if (! ShadowBuffer->CheckCorrectness())
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

	CRenderPass * ShadowPass = new CRenderPass(Context);
	ShadowPass->SetRenderTarget(ShadowBuffer);
	SceneManager->AddRenderPass(ShadowPass);

	CRenderPass * ColorPass = new CRenderPass(Context);
	ColorPass->SetRenderTarget(BackBuffer);
	SceneManager->AddRenderPass(ColorPass);

	CRenderPass * PostProcess = new CRenderPass(Context);
	PostProcess->SetRenderTarget(BackBuffer);
	SceneManager->AddRenderPass(PostProcess);

	CPerspectiveCamera * Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(0, 3, -5));
	Camera->SetFocalLength(0.4f);
	ColorPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(15.f * Constants32::Pi / 48.f);
	Controller->SetPhi(-Constants32::Pi / 16.f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);

	vec3f const LightDirection = vec3f(2, -12, 2);
	float const LightViewSize = 100.f;

	COrthographicCamera * LightCamera = new COrthographicCamera(-LightViewSize, LightViewSize, -LightViewSize, LightViewSize);
	LightCamera->SetPosition(-LightDirection);
	LightCamera->SetLookDirection(LightDirection);
	LightCamera->SetNearPlane(1.f);
	LightCamera->SetNearPlane(20.f);
	ShadowPass->SetActiveCamera(LightCamera);


	/////////////////
	// Add Objects //
	/////////////////

	CSimpleMeshSceneObject * Sphere1 = new CSimpleMeshSceneObject();
	Sphere1->SetMesh(SphereMesh);
	Sphere1->SetShader(DiffuseShader);
	Sphere1->SetPosition(vec3f(0, 0, 0));
	Sphere1->SetScale(2.f);
	ColorPass->AddSceneObject(Sphere1);
	ShadowPass->AddSceneObject(Sphere1);

	CSimpleMeshSceneObject * Sphere2 = new CSimpleMeshSceneObject();
	Sphere2->SetMesh(SphereMesh);
	Sphere2->SetShader(DiffuseShader);
	Sphere2->SetPosition(vec3f(4, 0, 0));
	Sphere2->SetScale(3.f);
	ColorPass->AddSceneObject(Sphere2);
	ShadowPass->AddSceneObject(Sphere2);

	CSimpleMeshSceneObject * Sphere3 = new CSimpleMeshSceneObject();
	Sphere3->SetMesh(SphereMesh);
	Sphere3->SetShader(DiffuseShader);
	Sphere3->SetPosition(vec3f(12, 0, 0));
	Sphere3->SetScale(4.f);
	ColorPass->AddSceneObject(Sphere3);
	ShadowPass->AddSceneObject(Sphere3);

	CSimpleMeshSceneObject * Sphere4 = new CSimpleMeshSceneObject();
	Sphere4->SetMesh(SphereMesh);
	Sphere4->SetShader(DiffuseShader);
	Sphere4->SetPosition(vec3f(3, 0, 6));
	ColorPass->AddSceneObject(Sphere4);
	ShadowPass->AddSceneObject(Sphere4);

	CSimpleMeshSceneObject * Plane = new CSimpleMeshSceneObject();
	Plane->SetMesh(PlaneMesh);
	Plane->SetShader(DiffuseShader);
	ColorPass->AddSceneObject(Plane);
	ShadowPass->AddSceneObject(Plane);

	CSimpleMeshSceneObject * PostProcessObject = new CSimpleMeshSceneObject();
	PostProcessObject->SetMesh(CGeometryCreator::CreateScreenTriangle());
	PostProcessObject->SetShader(QuadCopyShader);
	PostProcessObject->SetTexture("uTexture", ShadowTexture);
	//PostProcessObject->SetVisible(false);
	PostProcess->AddSceneObject(PostProcessObject);

	CDirectionalLight * Light1 = new CDirectionalLight();
	Light1->SetDirection(LightDirection);
	ColorPass->AddLight(Light1);
	ShadowPass->AddLight(Light1);


	///////////////
	// Main Loop //
	///////////////

	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();

		PostProcessObject->SetVisible(Window->IsKeyDown(EKey::F1));

		ShadowBuffer->ClearColorAndDepth();
		BackBuffer->ClearColorAndDepth();

		SceneManager->DrawAll();
		Window->SwapBuffers();
	}

	return 0;
}
