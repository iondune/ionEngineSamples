
#include "CApplication.h"


using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


void CApplication::Run()
{
	LoadSettings();

	Init();
	LoadAssets();
	SetupScene();
	AddObjects();

	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();

		GUIManager->NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_Once);
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Camera position: %.3f %.3f %.3f", Camera->GetPosition().X, Camera->GetPosition().Y, Camera->GetPosition().Z);
			ImGui::Text("Camera rotation: %.3f %.3f", Controller->GetTheta(), Controller->GetPhi());
			ImGui::Text("Camera speed: %.3f %.3f %.3f", Controller->GetCurrentSpeed().X, Controller->GetCurrentSpeed().Y, Controller->GetCurrentSpeed().Z);

			ImGui::Separator();

			ImGui::Text("Lights");
			ImGui::PushID("LightsToggles");
			float const Spacing = 50;
			ImGui::Checkbox("1", &LightsVisible[0]);
			ImGui::SameLine(Spacing * 1.f);
			ImGui::Checkbox("2", &LightsVisible[1]);
			ImGui::SameLine(Spacing * 2.f);
			ImGui::Checkbox("3", &LightsVisible[2]);
			ImGui::SameLine(Spacing * 3.f);
			ImGui::Checkbox("4", &LightsVisible[3]);

			if (ImGui::Button("All"))
			{
				for (int i = 0; i < 4; ++ i)
				{
					LightsVisible[i] = true;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("None"))
			{
				for (int i = 0; i < 4; ++ i)
				{
					LightsVisible[i] = false;
				}
			}
			ImGui::PopID();


			ImGui::Combo("Shading Model", &ShadingModel, { "Phong", "Blinn-Phong", "Beckmann", "Cook-Torrance" });

			ImGui::SliderFloat("Roughness", &Roughness, 0.0001f, 0.9999f, "%.4f");

			if (ShadingModel == 3)
			{
				ImGui::SliderFloat("Metalness", &Metalness, 0.f, 1.f, "%.4f");
				ImGui::SliderFloat("IndexOfRefraction", &IndexOfRefraction, 0.5f, 2.5f, "%.4f");
				ImGui::Text("F0: %.4f", Sq((IndexOfRefraction - 1) / (IndexOfRefraction + 1)));

				ImGui::Separator();

				ImGui::Text("Exclusive Debug");
				ImGui::PushID("ExclusiveButtons");
				ImGui::RadioButton("Off", &DebugExclusive, 0);
				ImGui::RadioButton("D", &DebugExclusive, 1);
				ImGui::RadioButton("G", &DebugExclusive, 2);
				ImGui::RadioButton("F", &DebugExclusive, 3);
				ImGui::RadioButton("Denom", &DebugExclusive, 4);
				ImGui::PopID();

				ImGui::Separator();

				ImGui::Text("D Choice");
				{
					ImGui::scoped_id s("DChoice");
					ImGui::RadioButton("Blinn-Phong", &DChoice, 0);
					ImGui::RadioButton("Beckmann", &DChoice, 1);
					ImGui::RadioButton("GGX / Trowbridge-Reitz", &DChoice, 2);
				}

				ImGui::Text("G Choice");
				{
					ImGui::scoped_id s("GChoice");
					ImGui::RadioButton("Cook-Torrance", &GChoice, 0);
					ImGui::RadioButton("GGX", &GChoice, 1);
				}
			}

		}
		ImGui::End();


		if (ShadingModel == 3)
		{
			SpecSphereObject->SetVisible(false);
			SpecPlaneObject->SetVisible(false);
			CTSphereObject->SetVisible(true);
			CTPlaneObject->SetVisible(true);
		}
		else
		{
			SpecSphereObject->SetVisible(true);
			SpecPlaneObject->SetVisible(true);
			CTSphereObject->SetVisible(false);
			CTPlaneObject->SetVisible(false);
		}

		RenderTarget->ClearColorAndDepth();
		SceneManager->DrawAll();
		GUIManager->Draw();
		Window->SwapBuffers();


		if (TakeScreenshot)
		{
			CImage * Image = RenderTarget->ReadImage();

			static int ScreenshotCounter = 0;
			string FileName = String::Build("Screenshot%06d.png", ScreenshotCounter);
			ScreenshotCounter ++;

			vec2i const Center = Window->GetSize() / 2;
			vec2i const Size = vec2i(640, 512);

			Image->Crop(Center - Size / 2, Size);
			Image->Write(FileName);
			TakeScreenshot = false;
		}
	}

	GUIManager->Shutdown();
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
			case EKey::E:
				ShadingModel = 0;
				break;

			case EKey::R:
				ShadingModel = 1;
				break;

			case EKey::T:
				ShadingModel = 2;
				break;

			case EKey::Y:
				ShadingModel = 3;
				break;

			case EKey::P:
				TakeScreenshot = true;
				break;
			}
		}
	}
}

void CApplication::Init()
{
	GraphicsAPI->Init(new Graphics::COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);
	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);

	Window = CreateWindowFromSettings("ShadingModels");

	GUIManager->Init(Window);
	Window->AddListener(GUIManager);
	GUIManager->AddListener(this);

	AssetManager->AddAssetPath("Assets");
	AssetManager->SetShaderPath("Shaders");
	AssetManager->SetTexturePath("Images");

	Context = GraphicsAPI->GetWindowContext(Window);
	RenderTarget = Context->GetBackBuffer();
	color3f ClearColor = color3i(104, 68, 196);
	ClearColor *= 0.25f;
	RenderTarget->SetClearColor(ClearColor);
}

void CApplication::LoadAssets()
{
	SphereMesh = CGeometryCreator::CreateSphere(0.5f, 128, 64);
	SkyBoxMesh = CGeometryCreator::CreateCube();
	PlaneMesh = CGeometryCreator::CreatePlane(vec2f(100.f));

	SimpleShader = AssetManager->LoadShader("Simple");
	SpecularShader = AssetManager->LoadShader("Specular");
	CookTorranceShader = AssetManager->LoadShader("Cook-Torrance");
}

void CApplication::SetupScene()
{
	RenderPass = new CRenderPass(Context);
	RenderPass->SetRenderTarget(RenderTarget);
	SceneManager->AddRenderPass(RenderPass);

	Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(5.f, 2.5f, 4.f));
	Camera->SetFocalLength(0.4f);
	Camera->SetFarPlane(250.f);
	RenderPass->SetActiveCamera(Camera);

	Controller = new CGamePadCameraController(Camera);
	Controller->SetTheta(2.36f);
	Controller->SetPhi(0.175f);
	AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);
}

void CApplication::AddObjects()
{
	SpecSphereObject = new CSimpleMeshSceneObject();
	SpecSphereObject->SetMesh(SphereMesh);
	SpecSphereObject->SetShader(SpecularShader);
	SpecSphereObject->SetPosition(vec3f(3, 3, 6));
	SpecSphereObject->GetMaterial().Ambient = vec3f(0.05f);
	SpecSphereObject->SetUniform("uShadingModel", std::make_shared<CUniformReference<int>>(&ShadingModel));
	SpecSphereObject->SetUniform("uRoughness", std::make_shared<CUniformReference<float>>(&Roughness));
	for (int i = 0; i < 4; ++ i)
	{
		SpecSphereObject->SetUniform(String::Build("uLightsVisible%d", i), std::make_shared<CUniformReference<bool>>(&LightsVisible[i]));
	}
	RenderPass->AddSceneObject(SpecSphereObject);

	SpecPlaneObject = new CSimpleMeshSceneObject();
	SpecPlaneObject->SetMesh(PlaneMesh);
	SpecPlaneObject->SetShader(SpecularShader);
	SpecPlaneObject->GetMaterial().Ambient = vec3f(0.05f, 0.05f, 0.1f);
	SpecPlaneObject->SetUniform("uShadingModel", std::make_shared<CUniformReference<int>>(&ShadingModel));
	SpecPlaneObject->SetUniform("uRoughness", std::make_shared<CUniformReference<float>>(&Roughness));
	for (int i = 0; i < 4; ++ i)
	{
		SpecPlaneObject->SetUniform(String::Build("uLightsVisible%d", i), std::make_shared<CUniformReference<bool>>(&LightsVisible[i]));
	}
	RenderPass->AddSceneObject(SpecPlaneObject);

	CTSphereObject = new CSimpleMeshSceneObject();
	CTSphereObject->SetMesh(SphereMesh);
	CTSphereObject->SetShader(CookTorranceShader);
	CTSphereObject->SetPosition(vec3f(3, 3, 6));
	CTSphereObject->GetMaterial().Ambient = vec3f(0.05f);
	CTSphereObject->SetUniform("uRoughness", std::make_shared<CUniformReference<float>>(&Roughness));
	CTSphereObject->SetUniform("uMetalness", std::make_shared<CUniformReference<float>>(&Metalness));
	CTSphereObject->SetUniform("uIOR", std::make_shared<CUniformReference<float>>(&IndexOfRefraction));
	for (int i = 0; i < 4; ++ i)
	{
		CTSphereObject->SetUniform(String::Build("uLightsVisible%d", i), std::make_shared<CUniformReference<bool>>(&LightsVisible[i]));
	}
	CTSphereObject->SetUniform("uDebugExclusive", std::make_shared<CUniformReference<int>>(&DebugExclusive));
	CTSphereObject->SetUniform("uDChoice", std::make_shared<CUniformReference<int>>(&DChoice));
	CTSphereObject->SetUniform("uGChoice", std::make_shared<CUniformReference<int>>(&GChoice));
	RenderPass->AddSceneObject(CTSphereObject);

	CTPlaneObject = new CSimpleMeshSceneObject();
	CTPlaneObject->SetMesh(PlaneMesh);
	CTPlaneObject->SetShader(CookTorranceShader);
	CTPlaneObject->GetMaterial().Ambient = vec3f(0.05f, 0.05f, 0.1f);
	CTPlaneObject->SetUniform("uRoughness", std::make_shared<CUniformReference<float>>(&Roughness));
	CTPlaneObject->SetUniform("uMetalness", std::make_shared<CUniformReference<float>>(&Metalness));
	CTPlaneObject->SetUniform("uIOR", std::make_shared<CUniformReference<float>>(&IndexOfRefraction));
	for (int i = 0; i < 4; ++ i)
	{
		CTPlaneObject->SetUniform(String::Build("uLightsVisible%d", i), std::make_shared<CUniformReference<bool>>(&LightsVisible[i]));
	}
	CTPlaneObject->SetUniform("uDebugExclusive", std::make_shared<CUniformReference<int>>(&DebugExclusive));
	CTPlaneObject->SetUniform("uDChoice", std::make_shared<CUniformReference<int>>(&DChoice));
	CTPlaneObject->SetUniform("uGChoice", std::make_shared<CUniformReference<int>>(&GChoice));
	RenderPass->AddSceneObject(CTPlaneObject);

	float const Radius = 10.f;

	CPointLight * Lights[4] = {};

	Lights[0] = new CPointLight();
	Lights[0]->SetPosition(vec3f(0, 1, 0));
	Lights[0]->SetColor(Color::Basic::Red);
	Lights[0]->SetRadius(Radius);
	RenderPass->AddLight(Lights[0]);

	Lights[1] = new CPointLight();
	Lights[1]->SetPosition(vec3f(4, 2, 0));
	Lights[1]->SetColor(Color::Basic::Green);
	Lights[1]->SetRadius(Radius);
	RenderPass->AddLight(Lights[1]);

	Lights[2] = new CPointLight();
	Lights[2]->SetPosition(vec3f(12, 3, 0));
	Lights[2]->SetColor(Color::Basic::Blue);
	Lights[2]->SetRadius(Radius);
	RenderPass->AddLight(Lights[2]);

	Lights[3] = new CPointLight();
	Lights[3]->SetPosition(vec3f(4, 15, 3));
	Lights[3]->SetColor(Color::Basic::White);
	Lights[3]->SetRadius(Radius * 1.5f);
	RenderPass->AddLight(Lights[3]);

	for (int i = 0; i < 4; ++ i)
	{
		CSimpleMeshSceneObject * LightSphere = new CSimpleMeshSceneObject();
		LightSphere->SetMesh(SphereMesh);
		LightSphere->SetShader(SimpleShader);
		LightSphere->SetPosition(Lights[i]->GetPosition());
		LightSphere->SetScale(0.1f);
		LightSphere->GetMaterial().Diffuse = Lights[i]->GetColor();
		RenderPass->AddSceneObject(LightSphere);

		LightsVisible[i] = true;
	}
}
