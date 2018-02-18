
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
	GraphicsAPI->Init(new COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);

	Window = WindowManager->CreateWindow(vec2i(1600, 900), "Geometry Clipmaps", EWindowType::Windowed);
	Window->AddChild(this);

	GraphicsContext = GraphicsAPI->GetWindowContext(Window);

	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);
	AssetManager->AddAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Textures/");

	RenderTarget = GraphicsContext->GetBackBuffer();
	RenderTarget->SetClearColor(color3f(0.9f));

	GUIManager->Init(Window);
	Window->AddListener(GUIManager);
}

void CApplication::LoadAssets()
{
	AxisShader = AssetManager->LoadShader("Axis");
	GeometryClipmapsShader = AssetManager->LoadShader("GeometryClipmaps");
}

void CApplication::SetupScene()
{
	RenderPass = new CRenderPass(GraphicsContext);
	RenderPass->SetRenderTarget(RenderTarget);
	SceneManager->AddRenderPass(RenderPass);

	FreeCamera = new CPerspectiveCamera(Window->GetAspectRatio());
	FreeCamera->SetPosition(vec3f(0, 3, -5));
	FreeCamera->SetFocalLength(0.4f);
	FreeCamera->SetNearPlane(0.01f);
	FreeCamera->SetFarPlane(100.f);

	CCameraController * Controller = new CCameraController(FreeCamera);
	Controller->SetTheta(15.f * Constants32::Pi / 48.f);
	Controller->SetPhi(-Constants32::Pi / 16.f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);

	RenderPass->SetActiveCamera(FreeCamera);
}

void CApplication::AddSceneObjects()
{
	class SimpleHeight : public CGeometryClipmapsSceneObject::IHeightInput
	{

	public:

		float GetTerrainHeight(vec2i const & Position)
		{
			float const Input = (float) (Length(vec2f(Position)));

			return 0;// -cos(Input * 0.01f) * 15 - cos(Input * 0.5f) * 2;
		}

		color3f GetTerrainColor(vec2i const & Position)
		{
			return color3f(abs(fmodf((float) Position.X * 0.1f, 1.f)), abs(fmodf((float) Position.Y * 0.1f, 1.f)), 0.5f);
		}

	};

	GeometryClipmapsObject = new CGeometryClipmapsSceneObject();
	GeometryClipmapsObject->SetWireframeEnabled(true);
	GeometryClipmapsObject->Shader = GeometryClipmapsShader;
	GeometryClipmapsObject->UseCameraPosition = true;
	GeometryClipmapsObject->HeightInput = new SimpleHeight();
	//GeometryClipmapsObject->SetRotation(vec3f(3.1415f / 2, 0, 0));
	RenderPass->AddSceneObject(GeometryClipmapsObject);

	CDirectionalLight * Light = new CDirectionalLight();
	Light->SetDirection(vec3f(1, -2, 1));
	RenderPass->AddLight(Light);

	CDirectionalLight * Shadow = new CDirectionalLight();
	Shadow->SetDirection(vec3f(-1, 2, -1));
	Shadow->SetColor(color3f(0.5f));
	RenderPass->AddLight(Shadow);

	CCoordinateFrameSceneObject * SceneAxis = new CCoordinateFrameSceneObject();
	SceneAxis->SetShader(AxisShader);
	RenderPass->AddSceneObject(SceneAxis);
}


void CApplication::MainLoop()
{
	TimeManager->Start();
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
			ImGui::Separator();

			vec3f const LongLat = CartToLatLong(FreeCamera->GetPosition());
			ImGui::Text("Camera Long/Lat/Elevation: %.3f %.3f %.3f", LongLat.X, LongLat.Y, LongLat.Z);

			ImGui::End();
		}

		// Draw
		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		ImGui::Render();
		Window->SwapBuffers();
	}
}
