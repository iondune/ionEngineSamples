
#include "CGeometryClipmapsSceneObject.h"


CGeometryClipmapsSceneObject::SLayer::SLayer(CGeometryClipmapsSceneObject * Owner, int const i, ion::Scene::CRenderPass * RenderPass)
{
	this->Owner = Owner;

	// Scaling Parameters
	Level = i;
	ScaleFactor = 1 << i;

	// Create Colormap Texture
	ColorMap = RenderPass->GetGraphicsAPI()->CreateTexture2D(
		vec2i(HeightmapResolution),
		ion::Graphics::ITexture::EMipMaps::False,
		ion::Graphics::ITexture::EFormatComponents::RGB,
		ion::Graphics::ITexture::EInternalFormatType::Fix8);
	ColorMap->SetMagFilter(ion::Graphics::ITexture::EFilter::Linear);
	ColorMap->SetMinFilter(ion::Graphics::ITexture::EFilter::Linear);
	ColorMap->SetWrapMode(ion::Graphics::ITexture::EWrapMode::Repeat);

	// Create Normalmap Texture
	NormalMap = RenderPass->GetGraphicsAPI()->CreateTexture2D(
		vec2i(HeightmapResolution),
		ion::Graphics::ITexture::EMipMaps::False,
		ion::Graphics::ITexture::EFormatComponents::RGB,
		ion::Graphics::ITexture::EInternalFormatType::Fix8);
	NormalMap->SetMagFilter(ion::Graphics::ITexture::EFilter::Linear);
	NormalMap->SetMinFilter(ion::Graphics::ITexture::EFilter::Linear);
	NormalMap->SetWrapMode(ion::Graphics::ITexture::EWrapMode::Repeat);

	// Create Heightmap Texture
	HeightMap = RenderPass->GetGraphicsAPI()->CreateTexture2D(
		vec2i(HeightmapResolution),
		ion::Graphics::ITexture::EMipMaps::False,
		ion::Graphics::ITexture::EFormatComponents::R,
		ion::Graphics::ITexture::EInternalFormatType::Float32);
	HeightMap->SetMagFilter(ion::Graphics::ITexture::EFilter::Linear);
	HeightMap->SetMinFilter(ion::Graphics::ITexture::EFilter::Linear);
	HeightMap->SetWrapMode(ion::Graphics::ITexture::EWrapMode::Repeat);

	// Determine starting ClipRegion
	vec2i const ClipPos = vec2i() - vec2i(GeometrySize / 2);
	ActiveRegion = rect2i(ClipPos, vec2i(GeometrySize));

	// Default Data Upload
	SendSample(0, 0, HeightmapResolution, HeightmapResolution, ClipPos, vec2i(0, 0));
	DataUpdated = true;

	IndexBuffer = RenderPass->GetGraphicsAPI()->CreateIndexBuffer();
	PipelineState = RenderPass->GetGraphicsContext()->CreatePipelineState();
}

int CGeometryClipmapsSceneObject::SLayer::SendSample(int const x1, int const y1, int const x2, int const y2, vec2i const & NewClipPos, vec2i const & NewDataOffset)
{
	//////////////////////////
	// Parameter Validation //
	//////////////////////////

	if (x2 <= x1)
	{
		std::cerr << "Invalid sample size parameters! x2=" << x2 << " x1=" << x1 << ". Aborting!" << std::endl;
		//DebugBreak();
		return -1;
	}

	if (y2 <= y1)
	{
		std::cerr << "Invalid sample size parameters! y2=" << y1 << " y1=" << y1 << ". Aborting!" << std::endl;
		//DebugBreak();
		return -1;
	}

	if ((x2 - x1) > HeightmapResolution || (y2 - y1) > HeightmapResolution || x2 > HeightmapResolution || y2 > HeightmapResolution)
	{
		std::cerr << "Sample size parameters too large! x2=" << x2 << " x1=" << x1 << " y2=" << y2 << " y1=" << y1 << ". Aborting!" << std::endl;
		//DebugBreak();
		return -1;
	}


	///////////////////////
	// Buffer Generation //
	///////////////////////

	int const Size = (x2 - x1) * (y2 - y1);
	float * const HeightData = new float[Size];
	float * const ColorData = new float[Size * 3];
	float * const NormalData = new float[Size * 3];

	bool Succeeded = true;

	for (int y = y1; y < y2; ++ y)
	{
		for (int x = x1; x < x2; ++ x)
		{
			int const Index = (x - x1) + (y - y1) * (x2 - x1);
			if (Index >= Size || Index < 0)
			{
				std::cerr << "Out of bounds index!" << std::endl;
				Succeeded = false;
			}
			else
			{
				vec2i Tile = vec2i(x, y);
				
				if (Tile.X < NewDataOffset.X)
					Tile.X += (HeightmapResolution);
				if (Tile.Y < NewDataOffset.Y)
					Tile.Y += (HeightmapResolution);
				Tile -= NewDataOffset;
				Tile += (NewClipPos - vec2i(1));
				Tile *= ScaleFactor;

				float Height = 0;
				vec3f Normal = vec3f(0, 1, 0);
				color3f Color = Colors::Blue;

				if (Owner->HeightInput)
				{
					Height = Owner->HeightInput->GetTerrainHeight(Tile);
					Normal = Owner->HeightInput->GetTerrainNormal(Tile);
					Color = Owner->HeightInput->GetTerrainColor(Tile);
				}

				HeightData[Index] = Height;

				ColorData[Index * 3 + 0] = Color.Red;
				ColorData[Index * 3 + 1] = Color.Green;
				ColorData[Index * 3 + 2] = Color.Blue;

				NormalData[Index * 3 + 0] = Normal.X;
				NormalData[Index * 3 + 1] = Normal.Y;
				NormalData[Index * 3 + 2] = Normal.Z;
			}
		}
	}


	/////////////////////////////
	// Buffer Upload & Cleanup //
	/////////////////////////////

	HeightMap->UploadSubRegion(HeightData, vec2u(x1, y1), vec2u(x2 - x1, y2 - y1), ion::Graphics::ITexture::EFormatComponents::R, ion::Graphics::EScalarType::Float);
	ColorMap->UploadSubRegion(ColorData, vec2u(x1, y1), vec2u(x2 - x1, y2 - y1), ion::Graphics::ITexture::EFormatComponents::RGB, ion::Graphics::EScalarType::Float);
	NormalMap->UploadSubRegion(NormalData, vec2u(x1, y1), vec2u(x2 - x1, y2 - y1), ion::Graphics::ITexture::EFormatComponents::RGB, ion::Graphics::EScalarType::Float);

	delete [] HeightData;
	delete [] ColorData;
	delete [] NormalData;

	if (Succeeded)
		return (x2 - x1) * (y2 - y1);

	return -1;
}

vec2i CGeometryClipmapsSceneObject::SLayer::GetDesiredActiveRegion(vec2i const & ViewerPosition) const
{
	static const auto MakeEven = [](int const i) -> int
	{
		return i % 2 ? i - 1 : i;
	};

	vec2i CenterPosition = ViewerPosition / ScaleFactor;

	// only allow even coordinates so that inner layers occupy the same grid as outer ones
	CenterPosition.X = MakeEven(CenterPosition.X);
	CenterPosition.Y = MakeEven(CenterPosition.Y);

	return CenterPosition - vec2i(GeometrySize / 2);
}

void CGeometryClipmapsSceneObject::SLayer::SetActiveRegion(vec2i const & NewActiveRegion)
{
	ActiveRegion.Position = NewActiveRegion;
}

int CGeometryClipmapsSceneObject::SLayer::GenerateAndUploadNewData(vec2i const & DataOffsetMove)
{
	Updater.SampleUploader = this;
	Updater.TextureResolution = HeightmapResolution;
	//Log::Info("About to do update on level %d", Level);
	Updater.DoUpdate(DataOffset, DataOffset + DataOffsetMove);

	DataOffset = Updater.FinalOffset;

	return Updater.TotalSamplesUploaded;
}

void CGeometryClipmapsSceneObject::SLayer::UploadSample(vec2i const & LowerBound, vec2i const & UpperBound, vec2i const & NewDataOffset)
{
	SendSample(LowerBound.X, LowerBound.Y, UpperBound.X, UpperBound.Y, ActiveRegion.Position, NewDataOffset);
}
