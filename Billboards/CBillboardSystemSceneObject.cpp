
#include "CBillboardSystemSceneObject.h"

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


CBillboardSystemSceneObject::CBillboardSystemSceneObject()
{
}

void CBillboardSystemSceneObject::SendBillboardsToGPU()
{
	NeedToLoadInstances = true;
}

void CBillboardSystemSceneObject::Load(CRenderPass * RenderPass)
{
	vector<f32> const Vertices
	{
		0.5f,  0.5f,   1, 1,
		0.5f, -0.5f,   1, 0,
		-0.5f, -0.5f,   0, 0,
		-0.5f,  0.5f,   0, 1,
	};

	vector<u32> const Indices
	{
		0, 1, 2,
		0, 2, 3,
	};

	SharedPointer<IIndexBuffer> IndexBuffer = RenderPass->GetGraphicsAPI()->CreateIndexBuffer();
	IndexBuffer->UploadData(Indices);
	SharedPointer<IVertexBuffer> VertexBuffer = RenderPass->GetGraphicsAPI()->CreateVertexBuffer();
	VertexBuffer->UploadData(Vertices);
	SInputLayoutElement InputLayout[] =
	{
		{ "vPosition", 2, EAttributeType::Float },
		{ "vTexCoords", 2, EAttributeType::Float },
	};
	VertexBuffer->SetInputLayout(InputLayout, ION_ARRAYSIZE(InputLayout));

	InstanceBuffer = RenderPass->GetGraphicsAPI()->CreateVertexBuffer();
	InstanceBuffer->SetInstancingEnabled(true);
	SInputLayoutElement InstanceLayout[] =
	{
		{ "vInstanceLocation", 3, EAttributeType::Float },
		{ "vInstanceSize", 1, EAttributeType::Float },
	};
	InstanceBuffer->SetInputLayout(InstanceLayout, ION_ARRAYSIZE(InstanceLayout));

	PipelineState = RenderPass->GetGraphicsContext()->CreatePipelineState();
	PipelineState->SetProgram(Shader);
	PipelineState->SetIndexBuffer(IndexBuffer);
	PipelineState->SetVertexBuffer(0, VertexBuffer);
	PipelineState->SetVertexBuffer(1, InstanceBuffer);
	PipelineState->SetTexture("uTexture", Texture);
	PipelineState->SetUniform("uGlobalScale", uGlobalScale);
	PipelineState->SetBlendMode(EBlendMode::Alpha);

	RenderPass->PreparePipelineStateForRendering(PipelineState, this);
	Loaded[RenderPass] = true;
}

void CBillboardSystemSceneObject::Draw(CRenderPass * RenderPass)
{
	if (NeedToLoadInstances)
	{
		static uint const FloatsPerVertex = 4;

		vector<float> InstanceData;
		InstanceData.reserve(Billboards.size() * FloatsPerVertex);
		for (auto const & Particle : Billboards)
		{
			InstanceData.push_back(Particle.Position.X);
			InstanceData.push_back(Particle.Position.Y);
			InstanceData.push_back(Particle.Position.Z);
			InstanceData.push_back(Particle.Size);
		}
		InstanceBuffer->UploadData(InstanceData);

		NeedToLoadInstances = false;
	}

	RenderPass->SubmitPipelineStateForRendering(PipelineState, this, (uint) Billboards.size(), 1);
}

void CBillboardSystemSceneObject::SetGlobalScale(float const Scale)
{
	uGlobalScale = Scale;
}

float CBillboardSystemSceneObject::GetGlobalScale() const
{
	return uGlobalScale;
}

void CBillboardSystemSceneObject::SetTexture(SharedPointer<ion::Graphics::ITexture2D> Texture)
{
	this->Texture = Texture;
	TriggerReload();
}
