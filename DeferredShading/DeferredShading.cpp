
#include <ionWindow.h>
#include <ionGraphics.h>
#include <ionGraphicsGL.h>
#include <ionScene.h>
#include <ionApplication.h>
#include <ionGUI.h>

#include <random>

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


float lerp(float const a, float const b, float const f)
{
	return a + f * (b - a);
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

	GraphicsAPI->Init(new Graphics::COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);
	SceneManager->Init(GraphicsAPI);
	AssetManager->Init(GraphicsAPI);

	CWindow * Window = WindowManager->CreateWindowOnMonitor(2, "DeferredShading");

	GUIManager->Init(Window);
	Window->AddListener(GUIManager);

	AssetManager->AddAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Images/");

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> BackBuffer = Context->GetBackBuffer();
	BackBuffer->SetClearColor(color3f(0.0f));

	SharedPointer<IFrameBuffer> SceneFrameBuffer = Context->CreateFrameBuffer();

	SharedPointer<ITexture2D> SceneColor = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGB, ITexture::EInternalFormatType::Fix8);
	SceneColor->SetMinFilter(ITexture::EFilter::Nearest);
	SceneColor->SetMagFilter(ITexture::EFilter::Nearest);
	SceneColor->SetWrapMode(ITexture::EWrapMode::Clamp);
	SharedPointer<ITexture2D> SceneNormal = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGB, ITexture::EInternalFormatType::Fix8);
	SceneNormal->SetMinFilter(ITexture::EFilter::Nearest);
	SceneNormal->SetMagFilter(ITexture::EFilter::Nearest);
	SceneNormal->SetWrapMode(ITexture::EWrapMode::Clamp);
	SharedPointer<ITexture2D> SceneDepth = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::R, ITexture::EInternalFormatType::Depth);
	SceneFrameBuffer->AttachColorTexture(SceneColor, 0);
	SceneFrameBuffer->AttachColorTexture(SceneNormal, 1);
	SceneFrameBuffer->AttachDepthTexture(SceneDepth);
	if (! SceneFrameBuffer->CheckCorrectness())
	{
		Log::Error("Frame buffer not valid!");
	}



	/////////////////
	// Load Assets //
	/////////////////

	CSimpleMesh * SphereMesh = CGeometryCreator::CreateSphere();
	CSimpleMesh * PlaneMesh = CGeometryCreator::CreatePlane(vec2f(100.f));

	SharedPointer<IShader> GeometryShader = AssetManager->LoadShader("Geometry");
	SharedPointer<IShader> DeferredDebugShader = AssetManager->LoadShader("Debug");
	SharedPointer<IShader> DeferredPointLightShader = AssetManager->LoadShader("PointLight");



	////////////////////
	// ionScene Setup //
	////////////////////

	CRenderPass * RenderPass = new CRenderPass(Context);
	RenderPass->SetRenderTarget(SceneFrameBuffer);
	SceneManager->AddRenderPass(RenderPass);

	CRenderPass * DeferredPass = new CRenderPass(Context);
	DeferredPass->SetRenderTarget(BackBuffer);
	SceneManager->AddRenderPass(DeferredPass);
	
	CPerspectiveCamera * Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(-1.7f, 2.8f, 3.4f));
	Camera->SetFocalLength(0.4f);
	Camera->SetNearPlane(0.1f);
	Camera->SetFarPlane(50.f);
	RenderPass->SetActiveCamera(Camera);
	DeferredPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(-0.08f);
	Controller->SetPhi(-0.26f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);


	/////////////////
	// Add Objects //
	/////////////////

	CSimpleMeshSceneObject * Sphere1 = new CSimpleMeshSceneObject();
	Sphere1->SetMesh(SphereMesh);
	Sphere1->SetShader(GeometryShader);
	Sphere1->SetPosition(vec3f(0, 0, 0));
	Sphere1->SetScale(2.f);
	Sphere1->GetMaterial().Diffuse *= color3f(1.0, 0.8f, 0.8f);
	RenderPass->AddSceneObject(Sphere1);

	CSimpleMeshSceneObject * Sphere2 = new CSimpleMeshSceneObject();
	Sphere2->SetMesh(SphereMesh);
	Sphere2->SetShader(GeometryShader);
	Sphere2->SetPosition(vec3f(4, 0, 0));
	Sphere2->SetScale(3.f);
	Sphere2->GetMaterial().Diffuse *= color3f(0.8f, 1, 0.8f);
	RenderPass->AddSceneObject(Sphere2);

	CSimpleMeshSceneObject * Sphere3 = new CSimpleMeshSceneObject();
	Sphere3->SetMesh(SphereMesh);
	Sphere3->SetShader(GeometryShader);
	Sphere3->SetPosition(vec3f(12, 0, 0));
	Sphere3->SetScale(4.f);
	Sphere3->GetMaterial().Diffuse *= color3f(0.8f, 0.9f, 1);
	RenderPass->AddSceneObject(Sphere3);

	CSimpleMeshSceneObject * Sphere4 = new CSimpleMeshSceneObject();
	Sphere4->SetMesh(SphereMesh);
	Sphere4->SetShader(GeometryShader);
	Sphere4->SetPosition(vec3f(3, 0, 6));
	Sphere4->GetMaterial().Diffuse *= color3f(0.9f, 1, 1);
	RenderPass->AddSceneObject(Sphere4);

	CSimpleMeshSceneObject * PlaneObject = new CSimpleMeshSceneObject();
	PlaneObject->SetMesh(PlaneMesh);
	PlaneObject->SetShader(GeometryShader);
	RenderPass->AddSceneObject(PlaneObject);

	CSimpleMesh * DragonMesh = AssetManager->LoadMeshMerged("dragon10k.obj");
	CSimpleMeshSceneObject * DragonObject = new CSimpleMeshSceneObject();
	DragonObject->SetMesh(DragonMesh);
	DragonObject->SetShader(GeometryShader);
	DragonObject->SetPosition(vec3f(6.f, 1.1f, 4.f));
	DragonObject->SetScale(4.f);
	DragonObject->GetMaterial().Diffuse *= color3f(1.f, 1.f, 0.2f);
	RenderPass->AddSceneObject(DragonObject);

	CSimpleMesh * StairMesh = AssetManager->LoadMeshMerged("SM_StairCase_02.obj");
	CSimpleMeshSceneObject * StairObject = new CSimpleMeshSceneObject();
	StairObject->SetMesh(StairMesh);
	StairObject->SetShader(GeometryShader);
	StairObject->SetPosition(vec3f(12.f, 0.f, 5.f));
	StairObject->SetScale(1.f);
	StairObject->SetRotation(vec3f(0.f, DegToRad(90.f), 0.f));
	StairObject->GetMaterial().Diffuse *= color3f(1.f, 0.6f, 0.2f);
	RenderPass->AddSceneObject(StairObject);

	int DebugMode = 0;

	CSimpleMeshSceneObject * PostProcessObject = new CSimpleMeshSceneObject();
	PostProcessObject->SetMesh(CGeometryCreator::CreateScreenTriangle());
	PostProcessObject->SetShader(DeferredDebugShader);
	PostProcessObject->SetTexture("tSceneColor", SceneColor);
	PostProcessObject->SetTexture("tSceneNormals", SceneNormal);
	PostProcessObject->SetTexture("tSceneDepth", SceneDepth);
	PostProcessObject->SetUniform("uMode", std::make_shared<CUniformReference<int>>(&DebugMode));
	DeferredPass->AddSceneObject(PostProcessObject);

	vector<CSimpleMeshSceneObject *> LightObjects;

	const int NumLights = 30;

	for (int i = 0; i < NumLights; ++ i)
	{
		const vec3f LightPosition = vec3f(nrand() * 30.f, frand() * 3.f, nrand() * 30.f);
		const color3f LightColor = Color::HSV(frand(), 1.f, 1.f);

		CSimpleMeshSceneObject * LightObject = new CSimpleMeshSceneObject();
		LightObject->SetMesh(CGeometryCreator::CreateScreenTriangle());
		LightObject->SetShader(DeferredPointLightShader);
		LightObject->SetTexture("tSceneColor", SceneColor);
		LightObject->SetTexture("tSceneNormals", SceneNormal);
		LightObject->SetTexture("tSceneDepth", SceneDepth);
		LightObject->SetBlendMode(EBlendMode::Additive);
		LightObject->SetUniform("uPosition", CUniform<vec3f>(LightPosition));
		LightObject->SetUniform("uColor", CUniform<vec3f>(LightColor));
		DeferredPass->AddSceneObject(LightObject);

		LightObjects.push_back(LightObject);
	}

	CPointLight * Light1 = new CPointLight();
	Light1->SetPosition(vec3f(0, 6, 0));
	RenderPass->AddLight(Light1);


	///////////////
	// Main Loop //
	///////////////

	TimeManager->Init(WindowManager);
	while (WindowManager->Run())
	{
		TimeManager->Update();

		GUIManager->NewFrame();
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiSetCond_Once);
		if (ImGui::Begin("Settings"))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Camera position: %.3f %.3f %.3f", Camera->GetPosition().X, Camera->GetPosition().Y, Camera->GetPosition().Z);
			ImGui::Text("Camera rotation: %.3f %.3f", Controller->GetTheta(), Controller->GetPhi());

			ImGui::Separator();

			ImGui::RadioButton("Deferred", DebugMode == -1);
			ImGui::RadioButton("Simple Shading", DebugMode == 0);
			ImGui::RadioButton("Colors", DebugMode == 1);
			ImGui::RadioButton("Normals", DebugMode == 2);
			ImGui::RadioButton("Depth", DebugMode == 3);

			ImGui::End();
		}

		DebugMode = -1;
		if (Window->IsKeyDown(EKey::H))
		{
			DebugMode = 0;
		}
		else if (Window->IsKeyDown(EKey::J))
		{
			DebugMode = 1;
		}
		else if (Window->IsKeyDown(EKey::K))
		{
			DebugMode = 2;
		}
		else if (Window->IsKeyDown(EKey::L))
		{
			DebugMode = 3;
		}

		PostProcessObject->SetVisible(DebugMode >= 0);
		for (auto LightObject : LightObjects)
		{
			LightObject->SetVisible(DebugMode == -1);
		}

		SceneFrameBuffer->ClearColorAndDepth();
		BackBuffer->ClearColorAndDepth();
		SceneManager->DrawAll();
		GUIManager->Draw();
		Window->SwapBuffers();
	}

	return 0;
}
