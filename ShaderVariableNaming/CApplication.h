
#pragma once

#include <ionEngine.h>


class CApplication : public Singleton<CApplication>, public IEventListener
{

public:

	void Run();
	void OnEvent(IEvent & Event);

	SingletonPointer<CWindowManager> WindowManager;
	SingletonPointer<CTimeManager> TimeManager;
	SingletonPointer<ion::Scene::CSceneManager> SceneManager;
	SingletonPointer<ion::CAssetManager> AssetManager;

	CWindow * Window = nullptr;
	ion::Graphics::IGraphicsAPI * GraphicsAPI = nullptr;
	SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	SharedPointer<ion::Graphics::IShaderProgram> SimpleShader;
	SharedPointer<ion::Graphics::IShaderProgram> SimpleTextureShader;
	SharedPointer<ion::Graphics::IShaderProgram> DiffuseShader;
	SharedPointer<ion::Graphics::IShaderProgram> SpecularShader;
	SharedPointer<ion::Graphics::IShaderProgram> DiffuseTextureShader;

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
