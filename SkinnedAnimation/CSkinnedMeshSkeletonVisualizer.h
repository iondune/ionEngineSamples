
#pragma once

#include "CSkinnedMesh.h"


namespace ion
{

	class CSkinnedMeshSkeletonVisualizer
	{

	public:

		CSkinnedMesh * SkinnedMesh = nullptr;


		void Load(CSkinnedMesh * SkinnedMesh);
		void AddSceneObjects(ion::Scene::CRenderPass * RenderPass,
			SharedPointer<ion::Graphics::IShader> LineShader,
			SharedPointer<ion::Graphics::IShader> InstanceShader);

		void UpdateSceneObjects();
		void SetVisible(bool const Visible);

		ion::Scene::CSimpleMeshSceneObject * JointSpheresObject = nullptr;
		ion::Scene::CLineSceneObject * JointLinesObject = nullptr;

		SharedPointer<ion::Graphics::IVertexBuffer> JointInstanceBuffer;

	};

}
