
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
	GraphicsAPI->Init(new COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);

	Window = WindowManager->CreateWindow(vec2i(1600, 900), "Billboards", EWindowType::Windowed);
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
	GUIManager->AddFontFromFile("Assets/GUI/OpenSans.ttf", 18.f);
	Window->AddListener(GUIManager);
}

void CApplication::LoadAssets()
{
	CubeMesh = CGeometryCreator::CreateCube();

	BillboardShader = AssetManager->LoadShader("Billboard");
	SimpleTextureShader = AssetManager->LoadShader("SimpleTexture");

	GroundTexture = AssetManager->LoadTexture("Ground.png");
	if (GroundTexture)
	{
		GroundTexture->SetMagFilter(ITexture::EFilter::Nearest);
		GroundTexture->SetWrapMode(ITexture::EWrapMode::Clamp);
	}

	BillboardTexture1 = AssetManager->LoadTexture("donut.png");
	BillboardTexture2 = AssetManager->LoadTexture("donut2.png");
}

void CApplication::SetupScene()
{
	RenderPass = new CRenderPass(GraphicsContext);
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

	BillboardSystem = new CBillboardSystemSceneObject();
	BillboardSystem->Shader = BillboardShader;
	BillboardSystem->SetTexture(BillboardTexture1);
	RenderPass->AddSceneObject(BillboardSystem);

	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(3, 4, 2)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(7, 4, 2)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(3, 8, 2)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(3, 2, 2)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-3, 2, -5)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-3, 2, -3)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-1, 1, 0)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-2, 1, 0)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-3, 1, 0)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-4, 1, 0)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-5, 1, 0)));
	BillboardSystem->Billboards.push_back(CBillboardSystemSceneObject::SBillboard(vec3f(-6, 1, 0)));
	BillboardSystem->SendBillboardsToGPU();

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
			ImGui::End();
		}

		//BillboardSystem->SetGlobalScale(3.f + 2.f * (float) sin(TimeManager->GetRunTime()));

		BillboardSystem->Billboards[0].Size = 1.f + sin((float) TimeManager->GetRunTime());
		BillboardSystem->Billboards[1].Size = 1.f + sin((float) TimeManager->GetRunTime());
		BillboardSystem->Billboards[2].Size = 1.f + sin((float) TimeManager->GetRunTime());
		BillboardSystem->SendBillboardsToGPU();

		if (Window->IsKeyDown(EKey::Space))
		{
			BillboardSystem->SetTexture(BillboardTexture2);
		}
		else
		{
			BillboardSystem->SetTexture(BillboardTexture1);
		}

		// Draw
		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		ImGui::Render();
		Window->SwapBuffers();
	}
}
