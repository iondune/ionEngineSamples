
#include <ionWindow.h>
#include <ionGraphics.h>
#include <ionGraphicsGL.h>
#include <ionScene.h>
#include <ionApplication.h>
#include <ionGUI.h>

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;



CSimpleMesh * CreateScreenQuad()
{
	CSimpleMesh * Mesh = new CSimpleMesh();

	Mesh->Vertices.resize(4);
	Mesh->Triangles.resize(2);

	Mesh->Vertices[0].Position = vec3f(-1, -1, 0);
	Mesh->Vertices[1].Position = vec3f(1, -1, 0);
	Mesh->Vertices[2].Position = vec3f(1, 1, 0);
	Mesh->Vertices[3].Position = vec3f(-1, 1, 0);

	Mesh->Triangles[0].Indices[0] = 0;
	Mesh->Triangles[0].Indices[1] = 1;
	Mesh->Triangles[0].Indices[2] = 2;
	Mesh->Triangles[1].Indices[0] = 0;
	Mesh->Triangles[1].Indices[1] = 2;
	Mesh->Triangles[1].Indices[2] = 3;

	return Mesh;
}

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
	SingletonPointer<CGUIManager> GUIManager;

	GraphicsAPI->Init(new COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);
	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);

	CWindow * Window = WindowManager->CreateWindow(vec2i(1920, 1080), "Shadow Maps", EWindowType::Windowed);

	GUIManager->Init(Window);

	AssetManager->AddAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Images/");

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> BackBuffer = Context->GetBackBuffer();
	BackBuffer->SetClearColor(color3f(0.3f));

	SharedPointer<IFrameBuffer> ShadowBuffer = Context->CreateFrameBuffer();

	SharedPointer<ITexture2D> ShadowTexture = GraphicsAPI->CreateTexture2D(vec2u(4096), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGBA, ITexture::EInternalFormatType::Fix8);
	SharedPointer<ITexture2D> ShadowDepth = GraphicsAPI->CreateTexture2D(vec2u(4096), ITexture::EMipMaps::False, ITexture::EFormatComponents::R, ITexture::EInternalFormatType::Depth);
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
	CSimpleMesh * CubeMesh = CGeometryCreator::CreateCube();

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
	Camera->SetPosition(vec3f(15.25f, 7.3f, -11.85f));
	Camera->SetFocalLength(0.4f);
	Camera->SetFarPlane(200.f);
	ColorPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(2.347f);
	Controller->SetPhi(-0.326f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);

	vec3f LightDirection = vec3f(2, -12, 2);
	float LightViewSize = 20.f;
	float LightNear = 110.f;
	float LightFar = 130.f;

	COrthographicCamera * LightCamera = new COrthographicCamera(-LightViewSize, LightViewSize, -LightViewSize, LightViewSize);
	LightCamera->SetPosition(-LightDirection * 10.f);
	LightCamera->SetLookDirection(LightDirection);
	LightCamera->SetNearPlane(LightNear);
	LightCamera->SetFarPlane(LightFar);
	ShadowPass->SetActiveCamera(LightCamera);


	/////////////////
	// Add Objects //
	/////////////////

	CSimpleMeshSceneObject * Sphere1 = new CSimpleMeshSceneObject();
	Sphere1->SetMesh(SphereMesh);
	Sphere1->SetShader(DiffuseShader);
	Sphere1->SetPosition(vec3f(0, 1, 0));
	Sphere1->SetScale(2.f);
	ColorPass->AddSceneObject(Sphere1);
	ShadowPass->AddSceneObject(Sphere1);

	CSimpleMeshSceneObject * Sphere2 = new CSimpleMeshSceneObject();
	Sphere2->SetMesh(SphereMesh);
	Sphere2->SetShader(DiffuseShader);
	Sphere2->SetPosition(vec3f(4, 4, 0));
	Sphere2->SetScale(3.f);
	Sphere2->GetMaterial().Ambient *= Colors::Yellow;
	Sphere2->GetMaterial().Diffuse *= Colors::Yellow;
	ColorPass->AddSceneObject(Sphere2);
	ShadowPass->AddSceneObject(Sphere2);

	CSimpleMeshSceneObject * Sphere3 = new CSimpleMeshSceneObject();
	Sphere3->SetMesh(SphereMesh);
	Sphere3->SetShader(DiffuseShader);
	Sphere3->SetPosition(vec3f(12, 2, 0));
	Sphere3->SetScale(4.f);
	ColorPass->AddSceneObject(Sphere3);
	ShadowPass->AddSceneObject(Sphere3);

	CSimpleMeshSceneObject * Sphere4 = new CSimpleMeshSceneObject();
	Sphere4->SetMesh(SphereMesh);
	Sphere4->SetShader(DiffuseShader);
	Sphere4->SetPosition(vec3f(3, 4, 6));
	Sphere4->GetMaterial().Ambient *= Colors::Red;
	Sphere4->GetMaterial().Diffuse *= Colors::Red;
	ColorPass->AddSceneObject(Sphere4);
	ShadowPass->AddSceneObject(Sphere4);

	CSimpleMeshSceneObject * Cube1 = new CSimpleMeshSceneObject();
	Cube1->SetMesh(CubeMesh);
	Cube1->SetShader(DiffuseShader);
	Cube1->SetPosition(vec3f(-4, 4, 0));
	Cube1->SetScale(3.f);
	Cube1->GetMaterial().Ambient *= Colors::Cyan;
	Cube1->GetMaterial().Diffuse *= Colors::Cyan;
	ColorPass->AddSceneObject(Cube1);
	ShadowPass->AddSceneObject(Cube1);

	CSimpleMeshSceneObject * Cube2 = new CSimpleMeshSceneObject();
	Cube2->SetMesh(CubeMesh);
	Cube2->SetShader(DiffuseShader);
	Cube2->SetPosition(vec3f(-12, 2, 0));
	Cube2->SetScale(4.f);
	Cube2->GetMaterial().Ambient *= Colors::Blue;
	Cube2->GetMaterial().Diffuse *= Colors::Blue;
	ColorPass->AddSceneObject(Cube2);
	ShadowPass->AddSceneObject(Cube2);

	CSimpleMeshSceneObject * Plane = new CSimpleMeshSceneObject();
	Plane->SetMesh(PlaneMesh);
	Plane->SetShader(DiffuseShader);
	Plane->GetMaterial().Ambient *= Colors::Green;
	Plane->GetMaterial().Diffuse *= Colors::Green;
	ColorPass->AddSceneObject(Plane);
	ShadowPass->AddSceneObject(Plane);

	vector<CSimpleMesh *> Meshes = CGeometryCreator::LoadOBJFile("bunny.obj");
	for (auto Mesh : Meshes)
	{
		CSimpleMeshSceneObject * PlaneObject = new CSimpleMeshSceneObject();
		PlaneObject->SetMesh(Mesh);
		PlaneObject->SetShader(DiffuseShader);
		PlaneObject->SetPosition(vec3f(3, -1.f, -6));
		PlaneObject->SetScale(vec3f(2.5f));
		PlaneObject->SetRotation(vec3f(0, 3.14f / 4.f, 0));
		ColorPass->AddSceneObject(PlaneObject);
		ShadowPass->AddSceneObject(PlaneObject);
	}

	CSimpleMeshSceneObject * PostProcessObject = new CSimpleMeshSceneObject();
	PostProcessObject->SetMesh(CreateScreenQuad());
	PostProcessObject->SetShader(QuadCopyShader);
	PostProcessObject->SetTexture("uTexture", ShadowDepth);
	PostProcess->AddSceneObject(PostProcessObject);

	CDirectionalLight * Light1 = new CDirectionalLight();
	Light1->SetDirection(LightDirection);
	ColorPass->AddLight(Light1);
	ShadowPass->AddLight(Light1);

	CSimpleMeshSceneObject * LightSphere = new CSimpleMeshSceneObject();
	LightSphere->SetMesh(SphereMesh);
	LightSphere->SetShader(DiffuseShader);
	LightSphere->SetPosition(LightDirection * -2);
	LightSphere->SetScale(0.4f);
	LightSphere->GetMaterial().Ambient = 1.f / 0.75f;
	LightSphere->GetMaterial().Diffuse = 0;
	ColorPass->AddSceneObject(LightSphere);

	CUniform<glm::mat4> uLightMatrix;
	ColorPass->SetUniform("uLightMatrix", uLightMatrix);
	ColorPass->SetTexture("uShadowMap", ShadowDepth);

	// Obviously the shadow pass does not need these, but this will suppress warnings
	// An object that supports different shaders for different passes is needed
	ShadowPass->SetUniform("uLightMatrix", uLightMatrix);
	ShadowPass->SetTexture("uShadowMap", ShadowDepth);


	///////////////
	// Main Loop //
	///////////////

	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();

		PostProcessObject->SetVisible(Window->IsKeyDown(EKey::F1));

		GUIManager->NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_Once);
		if (ImGui::Begin("Settings"))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Camera position: %.3f %.3f %.3f", Camera->GetPosition().X, Camera->GetPosition().Y, Camera->GetPosition().Z);
			ImGui::Text("Camera rotation: %.3f %.3f", Controller->GetTheta(), Controller->GetPhi());

			ImGui::Separator();

			ImGui::SliderFloat("Light Camera Size", &LightViewSize, 1.f, 200.f);
			ImGui::SliderFloat("Light Near Plane", &LightNear, 1.f, 300.f);
			ImGui::SliderFloat("Light Far Plane", &LightFar, 1.f, 600.f);
			//ImGui::SliderFloat3("Light Direction", LightDirection.Values, -30.f, 30.f);
			ImGui::Text("Light Position: %.3f %.3f %.3f", LightCamera->GetPosition().X, LightCamera->GetPosition().Y, LightCamera->GetPosition().Z);
		}
		ImGui::End();

		LightCamera->SetLeft(-LightViewSize);
		LightCamera->SetRight(LightViewSize);
		LightCamera->SetBottom(-LightViewSize);
		LightCamera->SetTop(LightViewSize);
		LightCamera->SetPosition(-LightDirection * 10.f);
		LightCamera->SetLookDirection(LightDirection);
		LightCamera->SetNearPlane(LightNear);
		LightCamera->SetFarPlane(LightFar);
		LightCamera->Update();
		uLightMatrix = LightCamera->GetProjectionMatrix() * LightCamera->GetViewMatrix();

		ShadowBuffer->ClearColorAndDepth();
		BackBuffer->ClearColorAndDepth();
		SceneManager->DrawAll();


		GUIManager->Draw();

		Window->SwapBuffers();
	}

	return 0;
}
