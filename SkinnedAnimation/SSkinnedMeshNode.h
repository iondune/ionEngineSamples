
#pragma once

#include <ionCore.h>
#include <ionMath.h>
#include <ionGraphics.h>
#include <ionScene.h>

#include "SSkinnedMeshBuffer.h"


namespace ion
{

	class CSkinnedMesh;

	struct SSkinnedMeshNode : public ion::ITreeNode<SSkinnedMeshNode>
	{
		SSkinnedMeshBuffer * Buffer = nullptr;

		glm::mat4 Transformation;
		ion::Graphics::CUniform<glm::mat4> AbsoluteTransformation;

		void CalculateAbsoluteTransformation(glm::mat4 const & ParentTransformation = glm::mat4(1.f));

		void Load(ion::Scene::CRenderPass * RenderPass, CSkinnedMesh * SkinnedMesh);
		void Draw(ion::Scene::CRenderPass * RenderPass, CSkinnedMesh * SkinnedMesh);
	};

}
