
#include <ionEngine.h>

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


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
	Camera->SetLookAtTarget(vec3f(0, 0, 0));
	Camera->SetFocalLength(0.4f);
	RenderPass->SetActiveCamera(Camera);

	CCameraController * Controller = new CCameraController(Camera);
	Controller->SetTheta(15.f * Constants32::Pi / 48.f);
	Controller->SetPhi(-Constants32::Pi / 16.f);
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

	CSimpleMesh * Mesh = new CSimpleMesh();

	CSimpleMeshSceneObject * SceneObject4 = new CSimpleMeshSceneObject();
	SceneObject4->SetShader(ShaderProgram);
	SceneObject4->SetFeatureEnabled(EDrawFeature::Wireframe, true);
	RenderPass->AddSceneObject(SceneObject4);


	///////////////
	// Main Loop //
	///////////////

	float time = 0;
	while (WindowManager->Run())
	{
		TimeManager->Update();
		time += 0.01f;
		RenderTarget->ClearColorAndDepth();

		Mesh->Clear();

		srand(1234);
		vec3f const Center = vec3f(0, -50, 0);
		vec3f X = vec3f(0, -1, 0);

		static uint const SubdivisionNodes = 6;
		for (uint i = 0; i < SubdivisionNodes; ++ i)
		{
			vec3f const Next = X + vec3f(nrand(), 0, nrand());

			vec3f const Normal = (X - Center).GetNormalized();
			vec3f const Tangent = (Next - X).GetNormalized();
			vec3f const Bitangent = Cross(Normal, Tangent).GetNormalized();

			float const Offset = 0.1f;
			float const Overshoot = 0.05f * (1 - cos(time));

			uint const Start = (uint) Mesh->Vertices.size();

			Mesh->Vertices.push_back(CSimpleMesh::SVertex(X - Tangent * Overshoot + Bitangent * Offset, Normal));
			Mesh->Vertices.push_back(CSimpleMesh::SVertex(X - Tangent * Overshoot - Bitangent * Offset, Normal));
			Mesh->Vertices.push_back(CSimpleMesh::SVertex(Next + Tangent * Overshoot + Bitangent * Offset, Normal));
			Mesh->Vertices.push_back(CSimpleMesh::SVertex(Next + Tangent * Overshoot - Bitangent * Offset, Normal));

			CSimpleMesh::STriangle Triangle;
			Triangle.Indices[0] = Start + 0;
			Triangle.Indices[1] = Start + 1;
			Triangle.Indices[2] = Start + 3;
			Mesh->Triangles.push_back(Triangle);

			Triangle.Indices[0] = Start + 0;
			Triangle.Indices[1] = Start + 3;
			Triangle.Indices[2] = Start + 2;
			Mesh->Triangles.push_back(Triangle);

			X = Next;
		}
		SceneObject4->SetMesh(Mesh);

		SceneManager->DrawAll();
		Window->SwapBuffers();
	}

	return 0;
}
