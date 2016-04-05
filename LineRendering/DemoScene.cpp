
#include <ionEngine.h>

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


ion::Scene::CSimpleMesh * CreateLineMesh(vector<vec3f> const & Points, vec3f const & Center, float const Width = 0.1f, float const Overshoot = 0.1f)
{
	ion::Scene::CSimpleMesh * Mesh = new ion::Scene::CSimpleMesh();

	for (size_t i = 1; i < Points.size(); ++ i)
	{
		vec3f const & Current = Points[i - 1];
		vec3f const & Next = Points[i];

		vec3f const Normal = (Current - Center).GetNormalized();
		vec3f const Tangent = (Next - Current).GetNormalized();
		vec3f const Bitangent = Cross(Normal, Tangent).GetNormalized();

		uint const Start = (uint) Mesh->Vertices.size();

		Mesh->Vertices.push_back(CSimpleMesh::SVertex(Current - Tangent * Overshoot + Bitangent * Width, Normal));
		Mesh->Vertices.push_back(CSimpleMesh::SVertex(Current - Tangent * Overshoot - Bitangent * Width, Normal));
		Mesh->Vertices.push_back(CSimpleMesh::SVertex(Next + Tangent * Overshoot + Bitangent * Width, Normal));
		Mesh->Vertices.push_back(CSimpleMesh::SVertex(Next + Tangent * Overshoot - Bitangent * Width, Normal));

		CSimpleMesh::STriangle Triangle;
		Triangle.Indices[0] = Start + 0;
		Triangle.Indices[1] = Start + 1;
		Triangle.Indices[2] = Start + 3;
		Mesh->Triangles.push_back(Triangle);

		Triangle.Indices[0] = Start + 0;
		Triangle.Indices[1] = Start + 3;
		Triangle.Indices[2] = Start + 2;
		Mesh->Triangles.push_back(Triangle);
	}

	return Mesh;
}

int main()
{
	////////////////////
	// ionEngine Init //
	////////////////////

	Log::AddDefaultOutputs();

	SingletonPointer<CWindowManager> WindowManager;
	SingletonPointer<CTimeManager> TimeManager;
	SingletonPointer<CSceneManager> SceneManager;

	WindowManager->Init();
	TimeManager->Init();
	CWindow * Window = WindowManager->CreateWindow(vec2i(1600, 900), "DemoScene", EWindowType::Windowed);

	IGraphicsAPI * GraphicsAPI = new COpenGLAPI();
	SceneManager->Init(GraphicsAPI);

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> RenderTarget = Context->GetBackBuffer();
	RenderTarget->SetClearColor(color3f(0.3f));


	///////////////////
	// Create Shader //
	///////////////////

	SharedPointer<IVertexShader> VertexShader = GraphicsAPI->CreateVertexShaderFromFile("Diffuse.vert");
	SharedPointer<IPixelShader> PixelShader = GraphicsAPI->CreatePixelShaderFromFile("Diffuse.frag");

	if (! VertexShader)
		std::cerr << "Failed to compile vertex shader!" << std::endl;

	if (! PixelShader)
		std::cerr << "Failed to compile pixel shader!" << std::endl;

	SharedPointer<IShaderProgram> ShaderProgram = GraphicsAPI->CreateShaderProgram();
	ShaderProgram->SetVertexStage(VertexShader);
	ShaderProgram->SetPixelStage(PixelShader);


	////////////////////
	// ionScene Setup //
	////////////////////

	CRenderPass * RenderPass = new CRenderPass(GraphicsAPI, Context);
	RenderPass->SetRenderTarget(RenderTarget);
	SceneManager->AddRenderPass(RenderPass);

	CPerspectiveCamera * Camera = new CPerspectiveCamera(Window->GetAspectRatio());
	Camera->SetPosition(vec3f(0, 4, 0.01f));
	Camera->SetFocalLength(0.4f);
	RenderPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(-15.f * Constants32::Pi / 48.f);
	Controller->SetPhi(-Constants32::Pi / 2.2f);
	Window->AddListener(Controller);
	TimeManager->MakeUpdateTick(0.02)->AddListener(Controller);


	///////////////////
	// Scene Objects //
	///////////////////


	CSimpleMesh * Mesh3 = CGeometryCreator::CreateCube();
	CSimpleMeshSceneObject * SceneObject3 = new CSimpleMeshSceneObject();
	SceneObject3->SetMesh(Mesh3);
	SceneObject3->SetShader(ShaderProgram);
	SceneObject3->SetPosition(vec3f(-4, 0, 0));
	RenderPass->AddSceneObject(SceneObject3);

	CSimpleMeshSceneObject * SceneObject4 = new CSimpleMeshSceneObject();
	SceneObject4->SetShader(ShaderProgram);
	SceneObject4->SetFeatureEnabled(EDrawFeature::Wireframe, true);
	RenderPass->AddSceneObject(SceneObject4);

	srand(1234);
	vec3f const Center = vec3f(0, -50, 0);
	vector<vec3f> Points;
	Points.push_back(vec3f(0, -1, 0));

	for (uint i = 0; i < 12; ++ i)
	{
		Points.push_back(Points.back() + vec3f(nrand()*(frand() + 1), 0, nrand()*(frand() + 1)));
	}

	///////////////
	// Main Loop //
	///////////////

	float time = 0;
	while (WindowManager->Run())
	{
		TimeManager->Update();
		time += 0.01f;
		RenderTarget->ClearColorAndDepth();

		SceneObject4->SetMesh(CreateLineMesh(Points, Center, 0.1f, 0.05f*(1 - cos(time))));

		SceneManager->DrawAll();
		Window->SwapBuffers();
	}

	return 0;
}
