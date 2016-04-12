
#pragma once

#include <ionEngine.h>
#include "CGeometryClipmapsSceneObject.h"


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

	SharedPointer<ion::Graphics::IShaderProgram> GeometryClipmapsShader;

protected:

	void InitializeEngine();
	void LoadAssets();
	void SetupScene();
	void AddSceneObjects();
	void MainLoop();

	SharedPointer<ion::Graphics::IRenderTarget> RenderTarget = nullptr;
	ion::Scene::CPerspectiveCamera * FreeCamera = nullptr;

	CGeometryClipmapsSceneObject * GeometryClipmapsObject = nullptr;

private:

	friend class Singleton<CApplication>;
	CApplication()
	{}

};
