
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

		static const float OffsetSpeed = 0.06f;

		if (! KeyboardEvent.Pressed)
		{
			switch (KeyboardEvent.Key)
			{
			case EKey::F1:
				break;
			case EKey::F:
				RenderPass->SetActiveCamera(FreeCamera);
				break;
			}
		}
		else if (KeyboardEvent.Pressed)
		{
			switch (KeyboardEvent.Key)
			{
			case EKey::I:
				GoalPosition.Z += OffsetSpeed;
				DoCCD_IK(GoalPosition);
				break;
			case EKey::J:
				GoalPosition.X += OffsetSpeed;
				DoCCD_IK(GoalPosition);
				break;
			case EKey::K:
				GoalPosition.Z -= OffsetSpeed;
				DoCCD_IK(GoalPosition);
				break;
			case EKey::L:
				GoalPosition.X -= OffsetSpeed;
				DoCCD_IK(GoalPosition);
				break;
			case EKey::U:
				GoalPosition.Y += OffsetSpeed;
				DoCCD_IK(GoalPosition);
				break;
			case EKey::O:
				GoalPosition.Y -= OffsetSpeed;
				DoCCD_IK(GoalPosition);
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

	Window = WindowManager->CreateWindow(vec2i(1600, 900), "Inverse Kinematics", EWindowType::Windowed);
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
	CubeMesh = CGeometryCreator::CreateCube();
	SphereMesh = CGeometryCreator::CreateSphere();

	SimpleTextureShader = AssetManager->LoadShader("SimpleTexture");
	ColorShader = AssetManager->LoadShader("Color");

	GroundTexture = AssetManager->LoadTexture("Ground.png");
	if (GroundTexture)
	{
		GroundTexture->SetMagFilter(ITexture::EFilter::Nearest);
		GroundTexture->SetWrapMode(ITexture::EWrapMode::Clamp);
	}
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
	Solver.Joints.push_back(new CInverseKinematicsSolver::SJoint());
	Solver.Joints.push_back(new CInverseKinematicsSolver::SJoint());
	Solver.Joints.push_back(new CInverseKinematicsSolver::SJoint());

	Solver.Joints[2]->Parent = Solver.Joints[1];
	Solver.Joints[1]->Parent = Solver.Joints[0];

	DoCCD_IK(GoalPosition);
	NodeObjects.resize(Solver.Joints.size() * 2);

	for (int i = 0; i < NodeObjects.size(); ++ i)
	{
		NodeObjects[i] = new CSimpleMeshSceneObject();
		NodeObjects[i]->SetMesh(CubeMesh);
		NodeObjects[i]->SetShader(ColorShader);
		NodeObjects[i]->SetUniformValue("uColor", Color::HSV((float) i / NodeObjects.size(), 0.8f, 0.9f));
		RenderPass->AddSceneObject(NodeObjects[i]);
	}

	GoalObject = new CSimpleMeshSceneObject();
	GoalObject->SetMesh(SphereMesh);
	GoalObject->SetShader(ColorShader);
	GoalObject->SetScale(0.2f);
	GoalObject->SetUniformValue("uColor", color3f(0.6f));
	RenderPass->AddSceneObject(GoalObject);

	TimeManager->Start();
	while (WindowManager->Run())
	{
		TimeManager->Update();
		
		// GUI
		GUIManager->NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		static bool OverlayOpen = true;
		if (ImGui::Begin("Fixed Overlay", &OverlayOpen, ImVec2(0, 0), -1.f,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}


		for (int i = 0; i < NodeObjects.size(); ++ i)
		{
			if (i % 2)
			{
				NodeObjects[i]->SetTransformation(Solver.Joints[i/2]->getTransformation() * glm::scale(glm::mat4(1.f), glm::vec3(0.15f)));
			}
			else
			{
				NodeObjects[i]->SetTransformation(Solver.Joints[i/2]->getHalfTransformation() * glm::scale(glm::mat4(1.f), glm::vec3(0.5f, 0.1f, 0.1f)));
			}
		}

		GoalObject->SetPosition(GoalPosition);

		// Draw
		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		ImGui::Render();
		Window->SwapBuffers();
	}
}

void CApplication::DoCCD_IK(vec3f const & Goal)
{
	Solver.Run(Goal);
}
