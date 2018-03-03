
#pragma once

#include <ionEngine.h>

#include "CSkinnedMesh.h"
#include "CSkinnedMeshSkeletonVisualizer.h"


class CCharacterTestApplication : public ion::CDefaultApplication, public ion::Singleton<CCharacterTestApplication>
{

public:

	void Run();

	void InitializeWindow();
	void InitializeGUI();

	void LoadAssets();
	void AddObjects();
	void AddCamera();

	void OnEvent(IEvent & Event);

	ion::CSkinnedMesh * CurrentMesh = nullptr;
	ion::Scene::CLineSceneObject * LineObject = nullptr;

	ion::CSkinnedMeshSkeletonVisualizer Visualizer;

private:

	void OpenMesh(ion::CSkinnedMesh * Mesh);

	ion::SingletonPointer<ion::CWindowManager> WindowManager;
	ion::SingletonPointer<ion::CGUIManager> GUIManager;
	ion::SingletonPointer<ion::CSceneManager> SceneManager;
	ion::SingletonPointer<ion::CTimeManager> TimeManager;
	ion::SingletonPointer<ion::CAssetManager> AssetManager;
	ion::SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

	ion::CWindow * Window = nullptr;
	ion::Scene::CPerspectiveCamera * Camera = nullptr;
	ion::Scene::COrthographicCamera * OrthoCameras[3];
	ion::CGamePadCameraController * CameraControllers[4];

	ion::Scene::CSimpleMesh * CubeMesh = nullptr;
	ion::Scene::CSimpleMesh * GridMesh = nullptr;
	ion::Scene::CSimpleMeshSceneObject * GridObject = nullptr;

	ion::SharedPointer<ion::Graphics::IShader> DiffuseShader;
	ion::SharedPointer<ion::Graphics::IShader> SimpleShader;
	ion::SharedPointer<ion::Graphics::IShader> ColorShader;
	ion::SharedPointer<ion::Graphics::IShader> InstanceColorShader;
	ion::SharedPointer<ion::Graphics::IShader> SkinnedShader;

	ion::SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext;
	ion::SharedPointer<ion::Graphics::IRenderTarget> RenderTarget;
	ion::Scene::CRenderPass * RenderPass = nullptr;

	ion::SingletonPointer<ion::CGamePad> GamePad;

};
