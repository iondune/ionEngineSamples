
#pragma once

#include <ionScene.h>



class CBillboardSystemSceneObject : public ion::Scene::ISceneObject
{

public:

	struct SBillboard
	{
		float Size = 1;
		vec3f Position;

		SBillboard()
		{}

		SBillboard(vec3f const & Position)
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

	SharedPointer<ion::Graphics::IShaderProgram> Shader;
	SharedPointer<ion::Graphics::ITexture2D> Texture;
	vector<SBillboard> Billboards;

protected:

	bool NeedToLoadInstances = true;

	SharedPointer<ion::Graphics::IPipelineState> PipelineState;
	SharedPointer<ion::Graphics::IVertexBuffer> InstanceBuffer;

	ion::Graphics::CUniform<float> uGlobalScale = 1;

};
