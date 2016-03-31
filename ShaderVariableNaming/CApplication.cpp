
#include "CApplication.h"

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


void CApplication::Run()
{
	InitializeEngine();
	LoadAssets();
	SetupScene();
	AddSceneObjects();

	MainLoop();
}

void CApplication::OnEvent(IEvent & Event)
{
	if (InstanceOf<SKeyboardEvent>(Event))
	{
		SKeyboardEvent KeyboardEvent = As<SKeyboardEvent>(Event);

		if (! KeyboardEvent.Pressed)
		{
			switch (KeyboardEvent.Key)
			{
			case EKey::F1:
				break;
			case EKey::F:
				RenderPass->SetActiveCamera(FreeCamera);
				break;
			case EKey::G:
				break;
			case EKey::LeftBracket:
				break;
			case EKey::RightBracket:
				break;
			case EKey::P:
				break;
			case EKey::Space:
				break;
			}
		}
	}
}

void CApplication::InitializeEngine()
{
	WindowManager->Init();
	TimeManager->Init();

	Window = WindowManager->CreateWindow(vec2i(1600, 900), "ShaderVariableNaming", EWindowType::Windowed);
	Window->AddChild(this);

	GraphicsAPI = new COpenGLAPI();
	GraphicsContext = GraphicsAPI->GetWindowContext(Window);

	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);
	AssetManager->SetAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Textures/");

	RenderTarget = GraphicsContext->GetBackBuffer();
	RenderTarget->SetClearColor(color3f(0.9f));
}

void CApplication::LoadAssets()
{
	CubeMesh = CGeometryCreator::CreateCube();

	SimpleTextureShader = AssetManager->LoadShader("SimpleTexture");
	SpecularShader = AssetManager->LoadShader("Specular");

	GroundTexture = AssetManager->LoadTexture("Ground.png");
	if (GroundTexture)
	{
		GroundTexture->SetMagFilter(ITexture::EFilter::Nearest);
		GroundTexture->SetWrapMode(ITexture::EWrapMode::Clamp);
	}
}

void CApplication::SetupScene()
{
	RenderPass = new CRenderPass(GraphicsAPI, GraphicsContext);
	RenderPass->SetRenderTarget(RenderTarget);
	SceneManager->AddRenderPass(RenderPass);

	FreeCamera = new CPerspectiveCamera(Window->GetAspectRatio());
	FreeCamera->SetPosition(vec3f(0, 3, -5));
	FreeCamera->SetFocalLength(0.4f);
	FreeCamera->SetFarPlane(10000.f);

	CCameraController * Controller = new CCameraController(FreeCamera);
	Controller->SetTheta(15.f * Constants32::Pi / 48.f);
	Controller->SetPhi(-Constants32::Pi / 16.f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);

	RenderPass->SetActiveCamera(FreeCamera);
}

void CApplication::AddSceneObjects()
{
	CSimpleMeshSceneObject * GroundObject = new CSimpleMeshSceneObject();
	GroundObject->SetMesh(CubeMesh);
	GroundObject->SetShader(SimpleTextureShader);
	GroundObject->SetScale(vec3f(16, 1, 16));
	GroundObject->SetPosition(vec3f(0, -0.5f, 0));
	GroundObject->SetTexture("uTexture", GroundTexture);
	RenderPass->AddSceneObject(GroundObject);

	vector<CSimpleMesh *> Meshes = CGeometryCreator::LoadOBJFile("Assets/Meshes/dragon43k.obj", "Assets/Meshes/");
	CSimpleMeshSceneObject * Object = new CSimpleMeshSceneObject();
	Object->SetMesh(Meshes[0]);
	Object->SetShader(SpecularShader);
	Object->SetPosition(vec3f(0, 1, 0));
	RenderPass->AddSceneObject(Object);

	DirectionalLight = new CDirectionalLight();
	DirectionalLight->SetDirection(vec3f(1, -2, -2));
	RenderPass->AddLight(DirectionalLight);

	PointLight = new CPointLight();
	RenderPass->AddLight(PointLight);
}


void CApplication::MainLoop()
{
	TimeManager->Init();
	while (WindowManager->Run())
	{
		TimeManager->Update();
		PointLight->SetPosition(FreeCamera->GetPosition());

		// Draw
		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		Window->SwapBuffers();
	}
}
