
#pragma once

#include <ionEngine.h>
#include "CBillboardSystemSceneObject.h"


class CApplication : public Singleton<CApplication>, public IEventListener
{

public:

	void Run();
	void OnEvent(IEvent & Event);

	SingletonPointer<CWindowManager> WindowManager;
	SingletonPointer<CTimeManager> TimeManager;
	SingletonPointer<ion::Scene::CSceneManager> SceneManager;
	SingletonPointer<ion::CAssetManager> AssetManager;
	SingletonPointer<CGUIManager> GUIManager;

	CWindow * Window = nullptr;
	ion::Graphics::IGraphicsAPI * GraphicsAPI = nullptr;
	SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	SharedPointer<ion::Graphics::IShaderProgram> SimpleTextureShader;
	SharedPointer<ion::Graphics::IShaderProgram> BillboardShader;

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	
	SharedPointer<ion::Graphics::ITexture2D> GroundTexture;
	SharedPointer<ion::Graphics::ITexture2D> BillboardTexture;

protected:

	void InitializeEngine();
	void LoadAssets();
	void SetupScene();
	void AddSceneObjects();
	void MainLoop();

	SharedPointer<ion::Graphics::IRenderTarget> RenderTarget = nullptr;
	ion::Scene::CPerspectiveCamera * FreeCamera = nullptr;

	CBillboardSystemSceneObject * BillboardSystem = nullptr;

private:

	friend class Singleton<CApplication>;
	CApplication()
	{}

};
