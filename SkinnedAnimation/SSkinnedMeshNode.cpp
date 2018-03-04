
#include "SSkinnedMeshNode.h"
#include "CSkinnedMesh.h"


namespace ion
{

	void SSkinnedMeshNode::CalculateAbsoluteTransformation(glm::mat4 const & ParentTransformation)
	{
		AbsoluteTransformation = ParentTransformation * Transformation;
		RecurseOnChildren<void, glm::mat4 const &>(& SSkinnedMeshNode::CalculateAbsoluteTransformation, AbsoluteTransformation);
	}

	void SSkinnedMeshNode::Load(ion::Scene::CRenderPass * RenderPass, CSkinnedMesh * SkinnedMesh)
	{
		if (Buffer)
		{
			Buffer->PipelineState->SetShader(SkinnedMesh->Shader);
			Buffer->PipelineState->SetUniform("uSkinningMatrix[0]", SkinnedMesh->uSkinningMatrices);
			Buffer->PipelineState->OfferUniform("uDebugDoSkin", SkinnedMesh->DebugDoSkin);
			Buffer->PipelineState->OfferUniform("uDebugShowWeightsByVertex", SkinnedMesh->DebugShowWeightsByVertex);
			Buffer->PipelineState->OfferUniform("uDebugShowWeightsByJoint", SkinnedMesh->DebugShowWeightsByJoint);
			Buffer->PipelineState->OfferUniform("uDebugWeightSelector", SkinnedMesh->DebugWeightSelector);
			Buffer->PipelineState->OfferUniform("uUseDualQuaternions", std::make_shared<Graphics::CUniformReference<bool>>(& SkinnedMesh->UseDualQuaternions));
			Buffer->PipelineState->OfferTexture("uTexture", SkinnedMesh->Texture);
			RenderPass->PreparePipelineStateForRendering(Buffer->PipelineState, SkinnedMesh);
		}

		RecurseOnChildren(& SSkinnedMeshNode::Load, RenderPass, SkinnedMesh);
	}

	void SSkinnedMeshNode::Draw(ion::Scene::CRenderPass * RenderPass, CSkinnedMesh * SkinnedMesh)
	{
		if (Buffer)
		{
			RenderPass->SubmitPipelineStateForRendering(Buffer->PipelineState, SkinnedMesh);
		}

		RecurseOnChildren(& SSkinnedMeshNode::Draw, RenderPass, SkinnedMesh);
	}

}
