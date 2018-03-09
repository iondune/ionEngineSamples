
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

	AssetManager->AddAssetPath("Assets");
	AssetManager->SetShaderPath("Shaders");
	AssetManager->SetTexturePath("Images");

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> RenderTarget = Context->GetBackBuffer();
	RenderTarget->SetClearColor(color3f(0.3f));


	/////////////////
	// Load Assets //
	/////////////////

	CSimpleMesh * SphereMesh = CGeometryCreator::CreateSphere();
	CSimpleMesh * SkyBoxMesh = CGeometryCreator::CreateCube();
	CSimpleMesh * PlaneMesh = CGeometryCreator::CreatePlane(vec2f(100.f));

	SharedPointer<IShader> SpecularShader = AssetManager->LoadShader("Specular");
	SharedPointer<IShader> SkyBoxShader = AssetManager->LoadShader("SkyBox");

	SharedPointer<ITexture2D> EnvironmentMap = AssetManager->LoadTexture("Ice_Lake_HiRes_TMap.jpg");
	EnvironmentMap->SetWrapMode(ITexture::EWrapMode::Clamp);


	////////////////////
	// ionScene Setup //
	////////////////////

	CRenderPass * RenderPass = new CRenderPass(Context);
	RenderPass->SetRenderTarget(RenderTarget);
	SceneManager->AddRenderPass(RenderPass);

	CPerspectiveCamera * Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(0, 3, -5));
	Camera->SetLookAtTarget(vec3f(0, 0, 0));
	Camera->SetFocalLength(0.4f);
	RenderPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);


	/////////////////
	// Add Objects //
	/////////////////

	CSimpleMeshSceneObject * SpecularSphere = new CSimpleMeshSceneObject();
	SpecularSphere->SetMesh(SphereMesh);
	SpecularSphere->SetShader(SpecularShader);
	SpecularSphere->SetPosition(vec3f(0));
	SpecularSphere->SetScale(vec3f(2.f));
	SpecularSphere->GetMaterial().Ambient = vec3f(0.05f);
	SpecularSphere->SetTexture("uEnvironmentMap", EnvironmentMap);
	RenderPass->AddSceneObject(SpecularSphere);

	CSimpleMeshSceneObject * SkyBoxObject = new CSimpleMeshSceneObject();
	SkyBoxObject->SetMesh(SkyBoxMesh);
	SkyBoxObject->SetShader(SkyBoxShader);
	SkyBoxObject->SetTexture("uTexture", EnvironmentMap);
	RenderPass->AddSceneObject(SkyBoxObject);

	CPointLight * Light1 = new CPointLight();
	Light1->SetPosition(vec3f(3, 6, 3));
	RenderPass->AddLight(Light1);


	///////////////
	// Main Loop //
	///////////////

	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();

		SkyBoxObject->SetPosition(Camera->GetPosition());

		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		Window->SwapBuffers();
	}

	return 0;
}
