
#pragma once

#include <ionEngine.h>


class CApplication : public ion::Singleton<CApplication>, public IEventListener
{

public:

	void Run();
	void OnEvent(IEvent & Event);

	ion::SingletonPointer<ion::CWindowManager> WindowManager;
	ion::SingletonPointer<ion::CTimeManager> TimeManager;
	ion::SingletonPointer<ion::CSceneManager> SceneManager;
	ion::SingletonPointer<ion::CAssetManager> AssetManager;
	ion::SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

	ion::CWindow * Window = nullptr;
	ion::SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	ion::SharedPointer<ion::Graphics::IShader> SimpleTextureShader;
	ion::SharedPointer<ion::Graphics::IShader> SpecularShader;

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	ion::SharedPointer<ion::Graphics::ITexture> GroundTexture;

	ion::Scene::CDirectionalLight * DirectionalLight = nullptr;
	ion::Scene::CPointLight * PointLight = nullptr;

protected:

	void InitializeEngine();
	void LoadAssets();
	void SetupScene();
	void AddSceneObjects();
	void MainLoop();

	ion::SharedPointer<ion::Graphics::IRenderTarget> RenderTarget = nullptr;
	ion::Scene::CPerspectiveCamera * FreeCamera = nullptr;

private:

	friend class ion::Singleton<CApplication>;
	CApplication()
	{}

};
