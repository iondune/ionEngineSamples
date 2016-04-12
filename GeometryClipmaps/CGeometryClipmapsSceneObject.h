
#pragma once

#include <ionEngine.h>
#include "CToroidalUpdater.h"


class CGeometryClipmapsSceneObject : public ion::Scene::ISceneObject
{

public:

	static int const GeometrySize = 64;
	static int const HeightmapResolution = GeometrySize + 2;
	static int const LayerCount = 6;

	int DrawLevel = 0;

	bool DoCameraUpdate = true;

	CGeometryClipmapsSceneObject();

	virtual void Load(ion::Scene::CRenderPass * RenderPass);
	virtual void Draw(ion::Scene::CRenderPass * RenderPass);

	ion::Graphics::CUniform<int> uSamplingMode = 0;
	ion::Graphics::CUniform<int> uDebugDisplay = 0;
	bool UseCameraPosition = false;
	bool UseCameraHeight = false;

	vec2i GlobalSystemOffset;
	vec2i ActiveCameraPositionAfterGlobalOffset;

	SharedPointer<ion::Graphics::IShaderProgram> Shader;

	class SLayer : public CToroidalUpdater::ISampleUploader
	{

	public:

		// Index data used for rendering - generated once per frame
		vector<unsigned int> IndexData;
		SharedPointer<ion::Graphics::IIndexBuffer> IndexBuffer;

		SharedPointer<ion::Graphics::ITexture2D> HeightMap;
		SharedPointer<ion::Graphics::ITexture2D> ColorMap;
		SharedPointer<ion::Graphics::ITexture2D> NormalMap;

		SharedPointer<ion::Graphics::IPipelineState> PipelineState;

		int Level, ScaleFactor;
		bool Active = true, DataUpdated = true;
		bool Visible = true;

		STransformation3 Transformation;

		rect2i ActiveRegion;
		
		// Offset of data "origin" within texture
		vec2i DataOffset;
		CToroidalUpdater Updater;

		// Uniforms sent to shader
		ion::Graphics::CUniform<glm::mat4> uTransformation;
		ion::Graphics::CUniform<vec2i> uDataOffset;
		ion::Graphics::CUniform<int> uScaleFactor;

		SLayer(CGeometryClipmapsSceneObject * Owner, int const i, ion::Scene::CRenderPass * RenderPass);

		//! Returns the offset in data needed for full update
		vec2i GetDesiredActiveRegion(vec2i const & ViewerPosition) const;
		void SetActiveRegion(vec2i const & ActiveRegion);
		int GenerateAndUploadNewData(vec2i const & DataOffsetMove);

	protected:
		
		int SendSample(int const x1, int const y1, int const x2, int const y2, vec2i const & NewClipPos, vec2i const & NewDataOffset);
		void UploadSample(vec2i const & LowerBound, vec2i const & UpperBound, vec2i const & NewDataOffset);

	};

	void SetWireframeEnabled(bool const Enabled);

	std::vector<SLayer *> Layers;

protected:

	ion::Graphics::CUniform<float> uTexelSize;
	ion::Graphics::CUniform<float> uHeightmapResolution;

	SharedPointer<ion::Graphics::IVertexBuffer> VertexBuffer = nullptr;

	bool Wireframe = false;

	vec3f ActiveCameraPosition;

};
