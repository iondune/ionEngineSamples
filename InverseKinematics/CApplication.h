
#pragma once

#include <ionEngine.h>
#include "CInverseKinematicsSolver.h"


class CApplication : public ion::Singleton<CApplication>, public IEventListener
{

public:

	void Run();
	void OnEvent(IEvent & Event);

	ion::SingletonPointer<ion::CWindowManager> WindowManager;
	ion::SingletonPointer<ion::CTimeManager> TimeManager;
	ion::SingletonPointer<ion::CSceneManager> SceneManager;
	ion::SingletonPointer<ion::CAssetManager> AssetManager;
	ion::SingletonPointer<ion::CGUIManager> GUIManager;
	ion::SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

	ion::CWindow * Window = nullptr;
	ion::SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	ion::SharedPointer<ion::Graphics::IShader> SimpleTextureShader;
	ion::SharedPointer<ion::Graphics::IShader> ColorShader;
	ion::SharedPointer<ion::Graphics::IShader> LineShader;

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	ion::Scene::CSimpleMesh * SphereMesh = nullptr;
	
	ion::SharedPointer<ion::Graphics::ITexture2D> GroundTexture;

protected:

	void InitializeEngine();
	void LoadAssets();
	void SetupScene();
	void AddSceneObjects();
	void MainLoop();

	ion::SharedPointer<ion::Graphics::IRenderTarget> RenderTarget = nullptr;
	ion::Scene::CPerspectiveCamera * FreeCamera = nullptr;

	ion::vec3f GoalPosition = ion::vec3f(1.f, 0, 1.f);
	std::vector<ion::Scene::CSimpleMeshSceneObject *> NodeObjects;
	ion::Scene::CSimpleMeshSceneObject * GoalObject;
	ion::Scene::CLineSceneObject * DebugLines = nullptr;

	bool ClearPoints = true;

	void UpdateGoalPosition();

	bool AutoSolve = true;

	ion::CInverseKinematicsSolver Solver;

private:

	friend class ion::Singleton<CApplication>;
	CApplication()
	{}

};
