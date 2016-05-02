
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

	CWindow * Window = WindowManager->CreateWindow(vec2i(1600, 900), "DemoApplication", EWindowType::Windowed);

	GUIManager->Init(Window);

	AssetManager->AddAssetPath("Assets/");
	AssetManager->SetShaderPath("Shaders/");
	AssetManager->SetTexturePath("Images/");

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> BackBuffer = Context->GetBackBuffer();
	BackBuffer->SetClearColor(color3f(0.3f));

	SharedPointer<IFrameBuffer> FrameBuffer = Context->CreateFrameBuffer();

	SharedPointer<ITexture2D> SceneColor = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGB, ITexture::EInternalFormatType::Fix8);
	SceneColor->SetMinFilter(ITexture::EFilter::Nearest);
	SceneColor->SetMagFilter(ITexture::EFilter::Nearest);
	SceneColor->SetWrapMode(ITexture::EWrapMode::Clamp);
	SharedPointer<ITexture2D> ScenePosition = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGBA, ITexture::EInternalFormatType::Float16);
	ScenePosition->SetMinFilter(ITexture::EFilter::Nearest);
	ScenePosition->SetMagFilter(ITexture::EFilter::Nearest);
	ScenePosition->SetWrapMode(ITexture::EWrapMode::Clamp);
	SharedPointer<ITexture2D> SceneNormal = GraphicsAPI->CreateTexture2D(Window->GetSize(), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGB, ITexture::EInternalFormatType::Float16);
	SceneNormal->SetMinFilter(ITexture::EFilter::Nearest);
	SceneNormal->SetMagFilter(ITexture::EFilter::Nearest);
	SceneNormal->SetWrapMode(ITexture::EWrapMode::Clamp);
	SharedPointer<IDepthBuffer> SceneDepth = GraphicsAPI->CreateDepthBuffer(Window->GetSize());
	FrameBuffer->AttachColorTexture(SceneColor, 0);
	FrameBuffer->AttachColorTexture(ScenePosition, 1);
	FrameBuffer->AttachColorTexture(SceneNormal, 2);
	FrameBuffer->AttachDepthBuffer(SceneDepth);
	if (! FrameBuffer->CheckCorrectness())
	{
		Log::Error("Frame buffer not valid!");
	}

	/////////////////
	// Load Assets //
	/////////////////

	CSimpleMesh * SphereMesh = CGeometryCreator::CreateSphere();
	CSimpleMesh * PlaneMesh = CGeometryCreator::CreatePlane(vec2f(100.f));

	SharedPointer<IShaderProgram> GeometryShader = AssetManager->LoadShader("Geometry");
	SharedPointer<IShaderProgram> SSAOShader = AssetManager->LoadShader("SSAO");
	SharedPointer<IShaderProgram> QuadCopyShader = AssetManager->LoadShader("QuadCopy");


	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;

	// Sample kernel
	std::vector<vec3f> ssaoKernel;
	for (uint i = 0; i < 64; ++i)
	{
		vec3f sample(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1, randomFloats(generator));
		sample.Normalize();
		sample *= randomFloats(generator);
		float scale = float(i) / 64.f;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// Noise texture
	std::vector<float> NoiseData;
	uint const NoiseTexSize = 4;
	for (uint i = 0; i < NoiseTexSize * NoiseTexSize; i++)
	{
		NoiseData.push_back(randomFloats(generator) * 2 - 1);
		NoiseData.push_back(randomFloats(generator) * 2 - 1);
		NoiseData.push_back(0.0);
	}
	SharedPointer<ITexture2D> SSAONoise = GraphicsAPI->CreateTexture2D(vec2u(NoiseTexSize), ITexture::EMipMaps::False, ITexture::EFormatComponents::RGB, ITexture::EInternalFormatType::Float16);
	SSAONoise->Upload(NoiseData.data(), vec2u(NoiseTexSize), ITexture::EFormatComponents::RGB, EScalarType::Float);
	SSAONoise->SetMinFilter(ITexture::EFilter::Nearest);
	SSAONoise->SetMagFilter(ITexture::EFilter::Nearest);


	////////////////////
	// ionScene Setup //
	////////////////////

	CRenderPass * RenderPass = new CRenderPass(Context);
	RenderPass->SetRenderTarget(FrameBuffer);
	SceneManager->AddRenderPass(RenderPass);

	CRenderPass * PostProcess = new CRenderPass(Context);
	PostProcess->SetRenderTarget(BackBuffer);
	SceneManager->AddRenderPass(PostProcess);

	CPerspectiveCamera * Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(0, 3, -5));
	Camera->SetFocalLength(0.4f);
	Camera->SetNearPlane(0.1f);
	Camera->SetFarPlane(50.f);
	RenderPass->SetActiveCamera(Camera);
	PostProcess->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(15.f * Constants32::Pi / 48.f);
	Controller->SetPhi(-Constants32::Pi / 16.f);
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

	//vector<CSimpleMesh *> Meshes = CGeometryCreator::LoadOBJFile("terrain.obj");
	//for (auto Mesh : Meshes)
	//{
	//	CSimpleMeshSceneObject * PlaneObject = new CSimpleMeshSceneObject();
	//	PlaneObject->SetMesh(Mesh);
	//	PlaneObject->SetShader(GeometryShader);
	//	RenderPass->AddSceneObject(PlaneObject);
	//}

	float SSAORadius = 1.0f;

	CSimpleMeshSceneObject * PostProcessObject = new CSimpleMeshSceneObject();
	PostProcessObject->SetMesh(CGeometryCreator::CreateScreenTriangle());
	PostProcessObject->SetShader(SSAOShader);
	PostProcessObject->SetTexture("gPositionDepth", ScenePosition);
	PostProcessObject->SetTexture("gNormal", SceneNormal);
	PostProcessObject->SetTexture("gPositionDepth", ScenePosition);
	PostProcessObject->SetTexture("texNoise", SSAONoise);
	PostProcessObject->SetUniform("uTanHalfFOV", CUniform<float>(Tan(Camera->GetFieldOfView() / 2.f)));
	PostProcessObject->SetUniform("uAspectRatio", CUniform<float>(Window->GetAspectRatio()));
	PostProcessObject->SetUniform("samples[0]", CUniform<vector<vec3f>>(ssaoKernel));
	PostProcessObject->SetUniform("radius", std::make_shared<CUniformReference<float>>(&SSAORadius));
	PostProcess->AddSceneObject(PostProcessObject);

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

			ImGui::SliderFloat("SSAO Radius", &SSAORadius, 0.1f, 20.f);

			ImGui::End();
		}

		FrameBuffer->ClearColorAndDepth();
		BackBuffer->ClearColorAndDepth();
		SceneManager->DrawAll();
		GUIManager->Draw();
		Window->SwapBuffers();
	}

	return 0;
}
