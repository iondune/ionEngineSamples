
#pragma once

#include <ionScene.h>



class CBillboardSystemSceneObject : public ion::Scene::ISceneObject
{

public:

	struct SBillboard
	{
		float Size = 1;
		ion::vec3f Position;

		SBillboard()
		{}

		SBillboard(ion::vec3f const & Position)
		{
			this->Position = Position;
		}
	};

	CBillboardSystemSceneObject();

	void SendBillboardsToGPU();

	void Load(ion::Scene::CRenderPass * RenderPass);
	void Draw(ion::Scene::CRenderPass * RenderPass);

	void SetGlobalScale(float const Scale);
	float GetGlobalScale() const;

	void SetTexture(ion::SharedPointer<ion::Graphics::ITexture2D> Texture);

	ion::SharedPointer<ion::Graphics::IShader> Shader;
	ion::SharedPointer<ion::Graphics::ITexture2D> Texture;
	ion::vector<SBillboard> Billboards;

protected:

	bool NeedToLoadInstances = true;

	ion::SharedPointer<ion::Graphics::IPipelineState> PipelineState;
	ion::SharedPointer<ion::Graphics::IVertexBuffer> InstanceBuffer;

	ion::Graphics::CUniform<float> uGlobalScale = 1;

};
