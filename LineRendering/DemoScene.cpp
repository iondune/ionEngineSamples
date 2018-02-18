
#include <ionEngine.h>
#include "California.h"

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

ion::Scene::CSimpleMesh * CreatePolygonMesh(vector<vec2f> const & Points)
{
	ion::Scene::CSimpleMesh * Mesh = new ion::Scene::CSimpleMesh();

	vector<STriangle2D<float>> Triangles = TriangulateEarClipping(Points);

	for (size_t i = 0; i < Triangles.size(); i ++)
	{
		uint const Start = (uint) Mesh->Vertices.size();
		vec3f const Normal = vec3f(0, 1, 0);
		Mesh->Vertices.push_back(CSimpleMesh::SVertex(vec3f(Triangles[i].A.X, 0, Triangles[i].A.Y), Normal));
		Mesh->Vertices.push_back(CSimpleMesh::SVertex(vec3f(Triangles[i].B.X, 0, Triangles[i].B.Y), Normal));
		Mesh->Vertices.push_back(CSimpleMesh::SVertex(vec3f(Triangles[i].C.X, 0, Triangles[i].C.Y), Normal));

		CSimpleMesh::STriangle Triangle;
		Triangle.Indices[0] = Start + 0;
		Triangle.Indices[1] = Start + 1;
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
	SingletonPointer<CGraphicsAPI> GraphicsAPI;

	GraphicsAPI->Init(new COpenGLImplementation());
	WindowManager->Init(GraphicsAPI);
	TimeManager->Init(WindowManager);
	CWindow * Window = WindowManager->CreateWindow(vec2i(1600, 900), "Line Rendering", EWindowType::Windowed);

	SceneManager->Init(GraphicsAPI);

	SharedPointer<IGraphicsContext> Context = GraphicsAPI->GetWindowContext(Window);
	SharedPointer<IRenderTarget> RenderTarget = Context->GetBackBuffer();
	RenderTarget->SetClearColor(color3f(0.3f));


	///////////////////
	// Create Shader //
	///////////////////

	SharedPointer<IVertexStage> VertexShader = GraphicsAPI->CreateVertexStageFromFile("Diffuse.vert");
	SharedPointer<IPixelStage> PixelShader = GraphicsAPI->CreatePixelStageFromFile("Diffuse.frag");

	if (! VertexShader)
		std::cerr << "Failed to compile vertex shader!" << std::endl;

	if (! PixelShader)
		std::cerr << "Failed to compile pixel shader!" << std::endl;

	SharedPointer<IShader> ShaderProgram = GraphicsAPI->CreateShaderProgram();
	ShaderProgram->SetVertexStage(VertexShader);
	ShaderProgram->SetPixelStage(PixelShader);


	////////////////////
	// ionScene Setup //
	////////////////////

	CRenderPass * RenderPass = new CRenderPass(Context);
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
	//SceneObject4->SetFeatureEnabled(EDrawFeature::Wireframe, true);
	RenderPass->AddSceneObject(SceneObject4);

	srand(1234);
	vector<vec2f> Points;
	//Points.push_back(vec2f(0, 0));
	//Points.push_back(vec2f(0, 2));
	//Points.push_back(vec2f(1, 3));
	//Points.push_back(vec2f(2, 2));
	//Points.push_back(vec2f(3, 1));
	//Points.push_back(vec2f(2, 0));

	for (uint i = 0; i < ION_ARRAYSIZE(California); ++ i)
	{
		Points.push_back(California[i].XY() - vec2f(-117, 33));
	}

	SceneObject4->SetMesh(CreatePolygonMesh(Points));

	///////////////
	// Main Loop //
	///////////////

	float time = 0;
	while (WindowManager->Run())
	{
		TimeManager->Update();
		time += 0.01f;
		RenderTarget->ClearColorAndDepth();


		SceneManager->DrawAll();
		Window->SwapBuffers();
	}

	return 0;
}
