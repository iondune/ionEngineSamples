
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
			case EKey::LeftBracket:
				GeometryClipmapsObject->SetWireframeEnabled(true);
				break;

			case EKey::RightBracket:
				GeometryClipmapsObject->SetWireframeEnabled(false);
				break;

			case EKey::P:
				GeometryClipmapsObject->UseCameraPosition = ! GeometryClipmapsObject->UseCameraPosition;
				break;
			}
		}
	}
}


void CApplication::InitializeEngine()
{
	WindowManager->Init();
	TimeManager->Init();

	Window = WindowManager->CreateWindow(vec2i(1600, 900), "Geometry Clipmaps", EWindowType::Windowed);
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

	GUIManager->Init(Window);
	GUIManager->AddFontFromFile("Assets/GUI/OpenSans.ttf", 18.f);
	Window->AddListener(GUIManager);
}

void CApplication::LoadAssets()
{
	GeometryClipmapsShader = AssetManager->LoadShader("GeometryClipmaps");
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
	GeometryClipmapsObject = new CGeometryClipmapsSceneObject();
	GeometryClipmapsObject->SetWireframeEnabled(true);
	GeometryClipmapsObject->Shader = GeometryClipmapsShader;
	GeometryClipmapsObject->uSamplingMode = 3;
	GeometryClipmapsObject->UseCameraPosition = true;
	RenderPass->AddSceneObject(GeometryClipmapsObject);

	CDirectionalLight * Light = new CDirectionalLight();
	Light->SetDirection(vec3f(1, -2, 1));
	RenderPass->AddLight(Light);

	CDirectionalLight * Shadow = new CDirectionalLight();
	Shadow->SetDirection(vec3f(-1, 2, -1));
	Shadow->SetColor(color3f(0.5f));
	RenderPass->AddLight(Shadow);
}


void CApplication::MainLoop()
{
	TimeManager->Init();
	while (WindowManager->Run())
	{
		TimeManager->Update();
		
		// GUI
		GUIManager->NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		static bool OverlayOpen = true;
		if (ImGui::Begin("Example: Fixed Overlay", &OverlayOpen, ImVec2(0, 0), -1.f,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// Draw
		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		ImGui::Render();
		Window->SwapBuffers();
	}
}
