
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
	SingletonPointer<ion::CGUIManager> GUIManager;
	SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

	ion::CWindow * Window = nullptr;
	SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	SharedPointer<ion::Graphics::IShaderProgram> SimpleShader;
	SharedPointer<ion::Graphics::IShaderProgram> SimpleTextureShader;
	SharedPointer<ion::Graphics::IShaderProgram> DiffuseShader;
	SharedPointer<ion::Graphics::IShaderProgram> DiffuseTextureShader;

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	
	SharedPointer<ion::Graphics::ITexture> GroundTexture;

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
