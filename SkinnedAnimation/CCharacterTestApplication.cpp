
#include "CCharacterTestApplication.h"
#include <glm/gtc/type_ptr.hpp>


using namespace ion;
using namespace ion::Scene;

string ToString(glm::mat4 const & m)
{
	string res;
	for (int i = 0; i < 4; ++ i)
	{
		res += String::Build("%.3f %.3f %.3f %.3f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
	}

	return res;
}

void CCharacterTestApplication::Run()
{
	LoadSettings();

	InitializeWindow();
	InitializeGUI();

	LoadAssets();
	AddObjects();
	AddCamera();

	OpenMesh(ion::CSkinnedMesh::Load("Assets/Meshes/model.dae"));

	Visualizer.Load(CurrentMesh);
	Visualizer.AddSceneObjects(RenderPass, ColorShader, InstanceColorShader);

	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();
		float const ElapsedTime = (float) TimeManager->GetElapsedTime();

		for (int i = 0; i < 4; ++ i)
		{
			CameraControllers[i]->Update(TimeManager->GetActualElapsedTime());
		}

		GUIManager->NewFrame();

		ImGui::SetNextWindowPos(ImVec2(10, 35), ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(500, 1295), ImGuiSetCond_Once);
		if (ImGui::Begin("Mesh Editor"))
		{
			if (CurrentMesh)
			{
				ImGui::Checkbox("Do Skinning?", & CurrentMesh->DebugDoSkin.Get());
				ImGui::Checkbox("Show Bone Weights By Vertex?", & CurrentMesh->DebugShowWeightsByVertex.Get());
				ImGui::Checkbox("Show Bone Weights By Joint?", & CurrentMesh->DebugShowWeightsByJoint.Get());
				ImGui::SliderInt("Current Bone", & CurrentMesh->DebugWeightSelector.Get(), 0, (int) CurrentMesh->Joints.size() - 1);

				int const CurrentJointIndex = CurrentMesh->DebugWeightSelector.Get();
				ion::CSkinnedMesh::CJoint * CurrentJoint = nullptr;

				if (CurrentJointIndex < CurrentMesh->Joints.size())
				{
					CurrentJoint = CurrentMesh->Joints[(size_t) CurrentJointIndex];

					ImGui::Text("Name: %s", CurrentJoint->Name.c_str());
					//ImGui::Text("Offset Transform:\n%s", ToString(CurrentJoint->OffsetTransform).c_str());
				}

				ImGui::Separator();

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::Text("Camera position: %.3f %.3f %.3f", Camera->GetPosition().X, Camera->GetPosition().Y, Camera->GetPosition().Z);
				ImGui::Text("Camera rotation: %.3f %.3f", CameraControllers[3]->GetTheta(), CameraControllers[3]->GetPhi());

				ImGui::Separator();
				ImGui::Text("Joints");

				for (int i = 0; i < CurrentMesh->GetJointCount(); ++ i)
				{
					string Label = String::Build(">###SelectJoint%d", i);
					if (ImGui::Button(Label.c_str()))
					{
						CurrentMesh->DebugWeightSelector.Get() = i;
					}
					ImGui::SameLine();

					auto Joint = CurrentMesh->GetJoint(i);
					float Rotation[3] =
					{
						RadToDeg<float>(Joint->AnimationTransform.GetRotation().X),
						RadToDeg<float>(Joint->AnimationTransform.GetRotation().Y),
						RadToDeg<float>(Joint->AnimationTransform.GetRotation().Z),
					};
					if (ImGui::DragFloat3(Joint->Name.c_str(), Rotation, 1.f))
					{
						vec3f Radians = vec3f(
							DegToRad<float>(Rotation[0]),
							DegToRad<float>(Rotation[1]),
							DegToRad<float>(Rotation[2]));
						Joint->AnimationTransform.SetRotation(Radians);
					}
				}

				if (ImGui::Button("Reset All"))
				{
					for (int i = 0; i < CurrentMesh->GetJointCount(); ++ i)
					{
						auto Joint = CurrentMesh->GetJoint(i);
						Joint->AnimationTransform.SetRotation(vec3f());
					}
				}
			}
		}
		ImGui::End();

		Visualizer.UpdateSceneObjects();

		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		ImGui::Render();
		Window->SwapBuffers();
	}
}

void CCharacterTestApplication::InitializeWindow()
{
	GraphicsAPI->Init(new Graphics::COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	Window = CreateWindowFromSettings("Relic::Character Test");
	Window->AddChild(this);

	GamePad->AddListener(this);
}

void CCharacterTestApplication::InitializeGUI()
{
	GUIManager->Init(Window);
	Window->AddListener(GUIManager);
}

void CCharacterTestApplication::LoadAssets()
{
	GraphicsContext = GraphicsAPI->GetWindowContext(Window);

	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);

	AssetManager->AddAssetPath("Assets/");

	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Textures/");

	GridMesh = CGeometryCreator::CreatePlane(20.f);
	CubeMesh = CGeometryCreator::CreateCube();

	SimpleShader = AssetManager->LoadShader("Simple");
	ColorShader = AssetManager->LoadShader("Color");
	SkinnedShader = AssetManager->LoadShader("Skinned");
	InstanceColorShader = AssetManager->LoadShader("InstanceColor");

	RenderTarget = GraphicsContext->GetBackBuffer();
	RenderTarget->SetClearColor(color3f(0.3f));
}

void CCharacterTestApplication::AddObjects()
{
	RenderPass = new CRenderPass(GraphicsContext);
	RenderPass->SetRenderTarget(RenderTarget);
	SceneManager->AddRenderPass(RenderPass);

	GridObject = new CSimpleMeshSceneObject();
	GridObject->SetShader(SimpleShader);
	GridMesh = CGeometryCreator::CreatePlane();
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(0.5f, 0, 0.5f), vec3f(1.5f, 0, 0.5f));
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(), vec3f(2, 0, 0));
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(), vec3f(4, 0, 0));
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(), vec3f(8, 0, 0));
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(), vec3f(0, 0, 1));
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(), vec3f(0, 0, 2));
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(), vec3f(0, 0, 4));
	GridMesh = CGeometryCreator::Intersect(GridMesh, GridMesh, vec3f(), vec3f(0, 0, 8));
	GridObject->SetMesh(GridMesh);
	GridObject->SetFeatureEnabled(ion::Graphics::EDrawFeature::Wireframe, true);
	GridObject->SetPosition(vec3f(-8, 0, -8));
	RenderPass->AddSceneObject(GridObject);

	CCoordinateFrameSceneObject * SceneAxis = new CCoordinateFrameSceneObject();
	SceneAxis->SetShader(ColorShader);
	RenderPass->AddSceneObject(SceneAxis);

	Scene::CDirectionalLight * Light = new Scene::CDirectionalLight();
	Light->SetColor(Color::White);
	Light->SetDirection(vec3f(2, -6, 3).GetNormalized());
	RenderPass->AddLight(Light);

	LineObject = new CLineSceneObject();
	LineObject->SetShader(ColorShader);
	RenderPass->AddSceneObject(LineObject);
}

void CCharacterTestApplication::AddCamera()
{
	Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(-2.75f, 6.1f, -2.75f));
	Camera->SetLookAtTarget(vec3f(0, 3.6f, 0));
	RenderPass->SetActiveCamera(Camera);

	CameraControllers[3] = new ion::CGamePadCameraController(Camera);
	GUIManager->AddListener(CameraControllers[3]);

	for (int i = 0; i < 3; ++ i)
	{
		OrthoCameras[i] = new COrthographicCamera(6.f, Window->GetAspectRatio());

		if (i == 1)
		{
			OrthoCameras[i]->SetUpVector(vec3f(1, 0, 0));
		}

		vec3f Position;
		Position.Y = 3.f;
		Position[i] = 50.f;

		if (i == 2)
		{
			Position.Z *= -1;
		}

		OrthoCameras[i]->SetPosition(Position);
		OrthoCameras[i]->SetLookAtTarget(vec3f(0.f, 3.f, 0.f));

		CameraControllers[i] = new ion::CGamePadCameraController(OrthoCameras[i]);
		CameraControllers[i]->SetActive(false);
		GUIManager->AddListener(CameraControllers[i]);
	}

	Camera->SetPosition(vec3f(-2.75f, 6.1f, -2.75f));
	Camera->SetLookAtTarget(vec3f(0, 3.6f, 0));
	RenderPass->SetActiveCamera(Camera);
}

void CCharacterTestApplication::OnEvent(IEvent & Event)
{
	if (InstanceOf<SKeyboardEvent>(Event))
	{
		SKeyboardEvent KeyboardEvent = As<SKeyboardEvent>(Event);

		if (! KeyboardEvent.Pressed)
		{
			switch (KeyboardEvent.Key)
			{
			case EKey::F1:
				for (int i = 0; i < 4; ++ i)
				{
					CameraControllers[i]->SetActive(false);
				}
				CameraControllers[3]->SetActive(true);
				RenderPass->SetActiveCamera(Camera);
				break;

			case EKey::F2:
				for (int i = 0; i < 4; ++ i)
				{
					CameraControllers[i]->SetActive(false);
				}
				CameraControllers[0]->SetActive(true);
				RenderPass->SetActiveCamera(OrthoCameras[0]);
				break;

			case EKey::F3:
				for (int i = 0; i < 4; ++ i)
				{
					CameraControllers[i]->SetActive(false);
				}
				CameraControllers[1]->SetActive(true);
				RenderPass->SetActiveCamera(OrthoCameras[1]);
				break;

			case EKey::F4:
				for (int i = 0; i < 4; ++ i)
				{
					CameraControllers[i]->SetActive(false);
				}
				CameraControllers[2]->SetActive(true);
				RenderPass->SetActiveCamera(OrthoCameras[2]);
				break;
			}
		}
	}
	else if (InstanceOf<SGamePadButtonEvent>(Event))
	{
		SGamePadButtonEvent GamePadButtonEvent = As<SGamePadButtonEvent>(Event);

		switch (GamePadButtonEvent.Button)
		{
		case EGamePadButton::A:
			break;
		case EGamePadButton::B:
			break;
		}
	}
}

void CCharacterTestApplication::OpenMesh(ion::CSkinnedMesh * Mesh)
{
	if (Mesh)
	{
		CurrentMesh = Mesh;
		CurrentMesh->SeparateTriangles();
		CurrentMesh->CalculateNormalsPerFace();
		CurrentMesh->Shader = SkinnedShader;
		CurrentMesh->Texture = AssetManager->LoadTexture("diffuse.png");
		CurrentMesh->Load(RenderPass);
		CurrentMesh->SetRotation(vec3f(glm::radians(-90.f), glm::radians(180.f), 0));
		RenderPass->AddSceneObject(CurrentMesh);
	}
}
