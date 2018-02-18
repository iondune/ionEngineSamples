
#pragma once

#include <ionEngine.h>
#include "CBillboardSystemSceneObject.h"


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
	ion::SharedPointer<ion::Graphics::IShader> BillboardShader;

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	
	ion::SharedPointer<ion::Graphics::ITexture2D> GroundTexture;
	ion::SharedPointer<ion::Graphics::ITexture2D> BillboardTexture1;
	ion::SharedPointer<ion::Graphics::ITexture2D> BillboardTexture2;

protected:

	void InitializeEngine();
	void LoadAssets();
	void SetupScene();
	void AddSceneObjects();
	void MainLoop();

	ion::SharedPointer<ion::Graphics::IRenderTarget> RenderTarget = nullptr;
	ion::Scene::CPerspectiveCamera * FreeCamera = nullptr;

	CBillboardSystemSceneObject * BillboardSystem = nullptr;

private:

	friend class ion::Singleton<CApplication>;
	CApplication()
	{}

};
