
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
	Window->AddListener(GUIManager);

	AssetManager->AddAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Images/");

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> BackBuffer = Context->GetBackBuffer();
	BackBuffer->SetClearColor(color3f(0.3f));

	SharedPointer<IFrameBuffer> ShadowBuffer = Context->CreateFrameBuffer();

	SharedPointer<ITexture2D> ShadowTexture = GraphicsAPI->CreateTexture2D(vec2i(4096), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGBA, ITexture::EInternalFormatType::Fix8);
	SharedPointer<ITexture2D> ShadowDepth = GraphicsAPI->CreateTexture2D(vec2i(4096), ITexture::EMipMaps::False, ITexture::EFormatComponents::R, ITexture::EInternalFormatType::Depth);
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

	SharedPointer<IShader> DiffuseShader = AssetManager->LoadShader("Diffuse");
	SharedPointer<IShader> ColorShader = AssetManager->LoadShader("Color");
	SharedPointer<IShader> QuadCopyShader = AssetManager->LoadShader("QuadCopy");


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
	Camera->SetFarPlane(1000.f);
	ColorPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(2.347f);
	Controller->SetPhi(-0.326f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);

	COrthographicCamera * LightCamera = new COrthographicCamera(-1, 1, -1, 1);
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
	Sphere2->GetMaterial().Ambient *= Color::Basic::Yellow;
	Sphere2->GetMaterial().Diffuse *= Color::Basic::Yellow;
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
	Sphere4->GetMaterial().Ambient *= Color::Basic::Red;
	Sphere4->GetMaterial().Diffuse *= Color::Basic::Red;
	ColorPass->AddSceneObject(Sphere4);
	ShadowPass->AddSceneObject(Sphere4);

	CSimpleMeshSceneObject * Cube1 = new CSimpleMeshSceneObject();
	Cube1->SetMesh(CubeMesh);
	Cube1->SetShader(DiffuseShader);
	Cube1->SetPosition(vec3f(-4, 4, 0));
	Cube1->SetScale(3.f);
	Cube1->GetMaterial().Ambient *= Color::Basic::Cyan;
	Cube1->GetMaterial().Diffuse *= Color::Basic::Cyan;
	ColorPass->AddSceneObject(Cube1);
	ShadowPass->AddSceneObject(Cube1);

	CSimpleMeshSceneObject * Cube2 = new CSimpleMeshSceneObject();
	Cube2->SetMesh(CubeMesh);
	Cube2->SetShader(DiffuseShader);
	Cube2->SetPosition(vec3f(-12, 2, 0));
	Cube2->SetScale(4.f);
	Cube2->GetMaterial().Ambient *= Color::Basic::Blue;
	Cube2->GetMaterial().Diffuse *= Color::Basic::Blue;
	ColorPass->AddSceneObject(Cube2);
	ShadowPass->AddSceneObject(Cube2);

	CSimpleMeshSceneObject * Plane = new CSimpleMeshSceneObject();
	Plane->SetMesh(PlaneMesh);
	Plane->SetShader(DiffuseShader);
	Plane->GetMaterial().Ambient *= Color::Basic::Green;
	Plane->GetMaterial().Diffuse *= Color::Basic::Green;
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

	CLineSceneObject * Lines = new CLineSceneObject();
	Lines->SetShader(ColorShader);
	ColorPass->AddSceneObject(Lines);

	CSimpleMeshSceneObject * PostProcessObject = new CSimpleMeshSceneObject();
	PostProcessObject->SetMesh(CreateScreenQuad());
	PostProcessObject->SetShader(QuadCopyShader);
	PostProcessObject->SetTexture("uTexture", ShadowDepth);
	PostProcess->AddSceneObject(PostProcessObject);

	CDirectionalLight * Light1 = new CDirectionalLight();
	ColorPass->AddLight(Light1);
	ShadowPass->AddLight(Light1);

	CSimpleMeshSceneObject * LightSphere = new CSimpleMeshSceneObject();
	LightSphere->SetMesh(SphereMesh);
	LightSphere->SetShader(DiffuseShader);
	LightSphere->SetScale(0.4f);
	LightSphere->GetMaterial().Ambient = 1.f / 0.75f;
	LightSphere->GetMaterial().Diffuse = 0;
	ColorPass->AddSceneObject(LightSphere);


	CUniform<bool> uDebugShadows = false;
	CUniform<float> uShadowBias = 0.005f;
	CUniform<glm::mat4> uLightMatrix;

	ColorPass->SetUniform("uLightMatrix", uLightMatrix);
	ColorPass->SetUniform("uDebugShadows", uDebugShadows);
	ColorPass->SetUniform("uShadowBias", uShadowBias);
	ColorPass->SetTexture("uShadowMap", ShadowDepth);

	// Obviously the shadow pass does not need these, but this will suppress warnings
	// An object that supports different shaders for different passes is needed
	ShadowPass->SetUniform("uLightMatrix", uLightMatrix);
	ShadowPass->SetUniform("uDebugShadows", uDebugShadows);
	ShadowPass->SetUniform("uShadowBias", uShadowBias);
	ShadowPass->SetTexture("uShadowMap", ShadowDepth);


	///////////////
	// Main Loop //
	///////////////

	vec3f LightDirection = Normalize( vec3f(2, -12, 2) );
	float LightViewSize = 20.f;
	float LightNear = 1.f;
	float LightFar = 30.f;
	float LightDistance = 15.f;


	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();

		PostProcessObject->SetVisible(Window->IsKeyDown(EKey::F1));

		GUIManager->NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiSetCond_Once);
		if (ImGui::Begin("Settings"))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Camera position: %.3f %.3f %.3f", Camera->GetPosition().X, Camera->GetPosition().Y, Camera->GetPosition().Z);
			ImGui::Text("Camera rotation: %.3f %.3f", Controller->GetTheta(), Controller->GetPhi());

			ImGui::Separator();

			ImGui::SliderFloat("Light Camera Size", &LightViewSize, 1.f, 200.f);
			ImGui::SliderFloat("Light Near Plane", &LightNear, 1.f, 300.f);
			ImGui::SliderFloat("Light Far Plane", &LightFar, 1.f, 600.f);

			ImGui::SliderFloat("Light Camera Distance", &LightDistance, 1.f, 200.f);
			ImGui::Text("Light Position: %.3f %.3f %.3f", LightCamera->GetPosition().X, LightCamera->GetPosition().Y, LightCamera->GetPosition().Z);

			ImGui::Separator();

			ImGui::Checkbox("Debug Shadows", &uDebugShadows.Get());
			ImGui::SliderFloat("Shadow Bias", &uShadowBias.Get(), 0.0f, 0.5f, "%.6f", 2.f);
		}
		ImGui::End();

		ImGui::SetNextWindowPos(vec2f(600, 10), ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(vec2f(300), ImGuiSetCond_Once);
		if (ImGui::Begin("Shadow Map"))
		{
			vec2f const AvailableSpace = ImGui::GetContentRegionAvail();
			ImGui::Image(GUIManager->GetTextureID(ShadowDepth), AvailableSpace);
		}
		ImGui::End();

		Light1->SetDirection(LightDirection);
		LightSphere->SetPosition(LightDirection * LightDistance);

		LightCamera->SetLeft(-LightViewSize);
		LightCamera->SetRight(LightViewSize);
		LightCamera->SetBottom(-LightViewSize);
		LightCamera->SetTop(LightViewSize);
		LightCamera->SetPosition(-LightDirection * LightDistance);
		LightCamera->SetLookDirection(LightDirection);
		LightCamera->SetNearPlane(LightNear);
		LightCamera->SetFarPlane(LightFar);
		LightCamera->Update();
		uLightMatrix = LightCamera->GetProjectionMatrix() * LightCamera->GetViewMatrix();

		vec3f const W = - Normalize(LightCamera->GetLookDirecton());
		vec3f const U = Normalize(Cross(LightCamera->GetUpVector(), W));
		vec3f const V = Cross(W, U);
		vec3f const P = LightCamera->GetPosition();

		vec3f Box[8];

		float const BoxSize = LightViewSize;
		Box[0] = P - U * BoxSize - V * BoxSize - W * LightNear;
		Box[1] = P + U * BoxSize - V * BoxSize - W * LightNear;
		Box[2] = P - U * BoxSize + V * BoxSize - W * LightNear;
		Box[3] = P + U * BoxSize + V * BoxSize - W * LightNear;

		Box[4] = P - U * BoxSize - V * BoxSize - W * LightFar;
		Box[5] = P + U * BoxSize - V * BoxSize - W * LightFar;
		Box[6] = P - U * BoxSize + V * BoxSize - W * LightFar;
		Box[7] = P + U * BoxSize + V * BoxSize - W * LightFar;

		Lines->ResetLines();
		Lines->AddLine(vec3f(0, 0, 0), P, Color::Basic::Cyan);
		Lines->AddLine(P, P + U, Color::Basic::Red);
		Lines->AddLine(P, P + V, Color::Basic::Green);
		Lines->AddLine(P, P + W, Color::Basic::Blue);

		Lines->AddLine(Box[0], Box[1], Color::Basic::White);
		Lines->AddLine(Box[0], Box[2], Color::Basic::White);
		Lines->AddLine(Box[1], Box[3], Color::Basic::White);
		Lines->AddLine(Box[2], Box[3], Color::Basic::White);

		Lines->AddLine(Box[4], Box[5], Color::Basic::White);
		Lines->AddLine(Box[4], Box[6], Color::Basic::White);
		Lines->AddLine(Box[5], Box[7], Color::Basic::White);
		Lines->AddLine(Box[6], Box[7], Color::Basic::White);

		Lines->AddLine(Box[0], Box[4], Color::Basic::White);
		Lines->AddLine(Box[1], Box[5], Color::Basic::White);
		Lines->AddLine(Box[2], Box[6], Color::Basic::White);
		Lines->AddLine(Box[3], Box[7], Color::Basic::White);
		//Lines->AddLine(vec3f(0, 0, 0), vec3f(0, 10, 0), Color::Basic::Blue);

		ShadowBuffer->ClearColorAndDepth();
		BackBuffer->ClearColorAndDepth();
		SceneManager->DrawAll();


		GUIManager->Draw();

		Window->SwapBuffers();
	}

	return 0;
}
