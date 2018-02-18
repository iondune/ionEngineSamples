
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
	ion::SingletonPointer<ion::CGUIManager> GUIManager;
	ion::SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

	ion::CWindow * Window = nullptr;
	ion::SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	ion::SharedPointer<ion::Graphics::IShader> SimpleShader;
	ion::SharedPointer<ion::Graphics::IShader> SimpleTextureShader;
	ion::SharedPointer<ion::Graphics::IShader> DiffuseShader;
	ion::SharedPointer<ion::Graphics::IShader> DiffuseTextureShader;

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	
	ion::SharedPointer<ion::Graphics::ITexture> GroundTexture;

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
