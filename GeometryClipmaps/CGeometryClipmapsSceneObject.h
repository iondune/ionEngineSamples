
#pragma once

#include <ionApplication.h>
#include "CToroidalUpdater.h"


ion::vec3f CartToLatLong(ion::vec3f const & c);

class CGeometryClipmapsSceneObject : public ion::Scene::ISceneObject
{

public:

	class IHeightInput
	{

	public:

		virtual float GetTerrainHeight(ion::vec2i const & Position) = 0;
		virtual ion::color3f GetTerrainColor(ion::vec2i const & Position) = 0;
		virtual ion::vec3f GetTerrainNormal(ion::vec2i const & Position);

	};

	IHeightInput * HeightInput = nullptr;

	static int const GeometrySize = 64;
	static int const HeightmapResolution = GeometrySize + 2;
	static int const LayerCount = 6;

	int DrawLevel = 0;

	CGeometryClipmapsSceneObject();

	virtual void Load(ion::Scene::CRenderPass * RenderPass);
	virtual void Draw(ion::Scene::CRenderPass * RenderPass);

	ion::Graphics::CUniform<int> uDebugDisplay = 0;
	bool UseCameraPosition = false;
	bool UseCameraHeight = false;

	ion::SharedPointer<ion::Graphics::IShader> Shader;

	class SLayer : public CToroidalUpdater::ISampleUploader
	{

	public:

		// Index data used for rendering - generated once per frame
		ion::vector<unsigned int> IndexData;
		ion::SharedPointer<ion::Graphics::IIndexBuffer> IndexBuffer;

		ion::SharedPointer<ion::Graphics::ITexture2D> HeightMap;
		ion::SharedPointer<ion::Graphics::ITexture2D> ColorMap;
		ion::SharedPointer<ion::Graphics::ITexture2D> NormalMap;

		ion::SharedPointer<ion::Graphics::IPipelineState> PipelineState;

		int Level, ScaleFactor;
		bool Active = true;
		bool Visible = true;

		ion::rect2i ActiveRegion;
		
		// Offset of data "origin" within texture
		ion::vec2i DataOffset;
		CToroidalUpdater Updater;

		// Uniforms sent to shader
		ion::Graphics::CUniform<ion::vec3f> uScale;
		ion::Graphics::CUniform<ion::vec3f> uTranslation;
		ion::Graphics::CUniform<ion::vec2i> uDataOffset;

		SLayer(CGeometryClipmapsSceneObject * Owner, int const i, ion::Scene::CRenderPass * RenderPass);

		//! Returns the offset in data needed for full update
		ion::vec2i GetDesiredActiveRegion(ion::vec2i const & ViewerPosition) const;
		void SetActiveRegion(ion::vec2i const & ActiveRegion);
		int GenerateAndUploadNewData(ion::vec2i const & DataOffsetMove);

	protected:

		CGeometryClipmapsSceneObject * Owner = nullptr;
		
		int SendSample(int const x1, int const y1, int const x2, int const y2, ion::vec2i const & NewClipPos, ion::vec2i const & NewDataOffset);
		void UploadSample(ion::vec2i const & LowerBound, ion::vec2i const & UpperBound, ion::vec2i const & NewDataOffset);

	};

	void SetWireframeEnabled(bool const Enabled);

	std::vector<SLayer *> Layers;

protected:

	// Shared vertex buffer for each later
	ion::SharedPointer<ion::Graphics::IVertexBuffer> VertexBuffer = nullptr;

	bool Wireframe = false;

	ion::vec2i ActiveCameraPosition;

};
