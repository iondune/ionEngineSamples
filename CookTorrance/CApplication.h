
#pragma once


#include <ionEngine.h>


class CApplication : public ion::CDefaultApplication
{

public:

	void Run();
	void OnEvent(IEvent & Event);

protected:

	void Init();
	void LoadAssets();
	void SetupScene();
	void AddObjects();

	// Managers

	ion::SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;
	ion::SingletonPointer<ion::CWindowManager> WindowManager;
	ion::SingletonPointer<ion::CTimeManager> TimeManager;
	ion::SingletonPointer<ion::CSceneManager> SceneManager;
	ion::SingletonPointer<ion::CAssetManager> AssetManager;
	ion::SingletonPointer<ion::CGUIManager> GUIManager;

	// Scene

	ion::SharedPointer<ion::Graphics::IGraphicsContext> Context;
	ion::SharedPointer<ion::Graphics::IRenderTarget> RenderTarget;
	ion::CWindow * Window = nullptr;
	ion::Scene::CRenderPass * RenderPass = nullptr;
	ion::Scene::CPerspectiveCamera * Camera = nullptr;
	ion::CGamePadCameraController * Controller = nullptr;

	bool TakeScreenshot = false;

	// Assets

	ion::Scene::CSimpleMesh * SphereMesh = nullptr;
	ion::Scene::CSimpleMesh * SkyBoxMesh = nullptr;
	ion::Scene::CSimpleMesh * PlaneMesh = nullptr;

	ion::SharedPointer<ion::Graphics::IShader> SimpleShader;
	ion::SharedPointer<ion::Graphics::IShader> SpecularShader;
	ion::SharedPointer<ion::Graphics::IShader> CookTorranceShader;

	ion::SharedPointer<ion::Graphics::ITextureCubeMap> SkyBoxTexture;

	// Objects

	ion::Scene::CSimpleMeshSceneObject * SpecSphereObject = nullptr;
	ion::Scene::CSimpleMeshSceneObject * SpecPlaneObject = nullptr;
	ion::Scene::CSimpleMeshSceneObject * CTSphereObject = nullptr;
	ion::Scene::CSimpleMeshSceneObject * CTPlaneObject = nullptr;

	int ShadingModel = 3;
	float Roughness = 0.5f;
	float Metalness = 0.5f;
	float IndexOfRefraction = 1.6f;

	bool LightsVisible[4];
	int DebugExclusive = 0;
	int DChoice = 0;
	int GChoice = 0;

};
