
#pragma once

#include <ionEngine.h>


class CApplication : public Singleton<CApplication>, public IEventListener
{

public:

	void Run();
	void OnEvent(IEvent & Event);

	SingletonPointer<ion::CWindowManager> WindowManager;
	SingletonPointer<ion::CTimeManager> TimeManager;
	SingletonPointer<ion::CSceneManager> SceneManager;
	SingletonPointer<ion::CAssetManager> AssetManager;
	SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

	ion::CWindow * Window = nullptr;
	SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	SharedPointer<ion::Graphics::IShaderProgram> SimpleTextureShader;
	SharedPointer<ion::Graphics::IShaderProgram> SpecularShader;

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	SharedPointer<ion::Graphics::ITexture> GroundTexture;

	ion::Scene::CDirectionalLight * DirectionalLight = nullptr;
	ion::Scene::CPointLight * PointLight = nullptr;

protected:

	void InitializeEngine();
	void LoadAssets();
	void SetupScene();
	void AddSceneObjects();
	void MainLoop();

	SharedPointer<ion::Graphics::IRenderTarget> RenderTarget = nullptr;
	ion::Scene::CPerspectiveCamera * FreeCamera = nullptr;

private:

	friend class Singleton<CApplication>;
	CApplication()
	{}

};
