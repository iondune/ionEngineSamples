
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

		static const float OffsetSpeed = 0.025f;

		if (! KeyboardEvent.Pressed)
		{
			switch (KeyboardEvent.Key)
			{
			case EKey::F1:
				break;
			case EKey::F:
				RenderPass->SetActiveCamera(FreeCamera);
				break;
			case EKey::I:
				GoalPosition.Z += OffsetSpeed;
				Configuration = DoCCD_IK(GoalPosition);
				break;
			case EKey::J:
				GoalPosition.X += OffsetSpeed;
				Configuration = DoCCD_IK(GoalPosition);
				break;
			case EKey::K:
				GoalPosition.Z -= OffsetSpeed;
				Configuration = DoCCD_IK(GoalPosition);
				break;
			case EKey::L:
				GoalPosition.X -= OffsetSpeed;
				Configuration = DoCCD_IK(GoalPosition);
				break;
			case EKey::U:
				GoalPosition.Y += OffsetSpeed;
				Configuration = DoCCD_IK(GoalPosition);
				break;
			case EKey::O:
				GoalPosition.Y -= OffsetSpeed;
				Configuration = DoCCD_IK(GoalPosition);
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
	Configuration = DoCCD_IK(GoalPosition);
	NodeObjects.resize(Configuration.size());

	for (int i = 0; i < NodeObjects.size(); ++ i)
	{
		NodeObjects[i] = new CSimpleMeshSceneObject();
		NodeObjects[i]->SetMesh(CubeMesh);
		NodeObjects[i]->SetShader(ColorShader);
		NodeObjects[i]->SetScale(0.15f);
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
			NodeObjects[i]->SetPosition(Configuration[i] * 3.f);
		}

		GoalObject->SetPosition(GoalPosition * 3.f);

		// Draw
		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		ImGui::Render();
		Window->SwapBuffers();
	}
}

vector<vec3f> CApplication::DoCCD_IK(vec3f const & Goal)
{
	struct Joint
	{
		Joint * Parent;
		vec3f Rotation = vec3f(0, 3.1415f, 0);
		float Length;

		Joint()
			: Parent(0), Length(0)
		{}

		glm::mat4 getLocalTransformation()
		{
			glm::mat4 Trans = glm::translate(glm::mat4(1.f), glm::vec3(Length, 0, 0));

			glm::mat4 Rot = glm::mat4(1.f);
			Rot = glm::rotate(Rot, Rotation.Z, glm::vec3(0, 0, 1));
			Rot = glm::rotate(Rot, Rotation.Y, glm::vec3(0, 1, 0));
			Rot = glm::rotate(Rot, Rotation.X, glm::vec3(1, 0, 0));

			return Rot * Trans;
		}

		glm::mat4 getLocalHalfTransformation()
		{
			glm::mat4 Trans = glm::translate(glm::mat4(1.f), glm::vec3(Length / 2.f, 0, 0));

			glm::mat4 Rot = glm::mat4(1.f);
			Rot = glm::rotate(Rot, Rotation.Z, glm::vec3(0, 0, 1));
			Rot = glm::rotate(Rot, Rotation.Y, glm::vec3(0, 1, 0));
			Rot = glm::rotate(Rot, Rotation.X, glm::vec3(1, 0, 0));

			return Rot * Trans;
		}

		glm::mat4 getTransformation()
		{
			glm::mat4 Trans = getLocalTransformation();

			if (Parent)
				Trans = Parent->getTransformation() * Trans;

			return Trans;
		}

		glm::mat4 getHalfTransformation()
		{
			glm::mat4 Trans = getLocalHalfTransformation();

			if (Parent)
				Trans = Parent->getTransformation() * Trans;

			return Trans;
		}

		vec3f const getLocation()
		{
			glm::vec4 v(0, 0, 0, 1);
			v = getTransformation() * v;

			return vec3f(v.x, v.y, v.z);
		}

		vec3f const getHalfLocation()
		{
			glm::vec4 v(0, 0, 0, 1);
			v = getHalfTransformation() * v;

			return vec3f(v.x, v.y, v.z);
		}
	};

	Joint Root, Joint1, Hand;
	Root.Length = Joint1.Length = 0.25f;
	Joint1.Parent = & Root;
	Hand.Parent = & Joint1;

	Joint * Joints[] = { & Root, & Joint1 };

	vec3f EndEffector(Goal.X, Goal.Y, Goal.Z);

	{
		auto GetValue = [&]() -> float
		{
			vec3f const HandLoc = Hand.getLocation();
			return Sq(EndEffector.GetDistanceFrom(HandLoc));
		};

		float Delta = DegToRad(30.f);
		for (int i = 0; i < 500; ++ i)
		{
			for (int t = 0; t < ION_ARRAYSIZE(Joints); ++t)
			{
				for (int u = 0; u < 3; ++ u)
				{
					float const LastValue = GetValue();
					Joints[t]->Rotation[u] += Delta;
					float const AddValue = GetValue();
					Joints[t]->Rotation[u] -= 2 * Delta;
					float const SubValue = GetValue();
					Joints[t]->Rotation[u] += Delta;

					if (LastValue < AddValue && LastValue < SubValue)
					{
					}
					else if (AddValue < SubValue)
					{
						Joints[t]->Rotation[u] += Delta;
					}
					else if (SubValue < AddValue)
					{
						Joints[t]->Rotation[u] -= Delta;
					}
					else
					{
					}
				}
			}
			Delta /= 1.01f;
		}
	}

	vec3f cent = vec3f(0);
	return {
		cent,
		(cent + Root.getLocation()),
		(cent + Root.getHalfLocation()),
		(cent + Joint1.getLocation()),
		(cent + Joint1.getHalfLocation()),
		(cent + Hand.getLocation()),
		(cent + Hand.getHalfLocation()),
	};
}
