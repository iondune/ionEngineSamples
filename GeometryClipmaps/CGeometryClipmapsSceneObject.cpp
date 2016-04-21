
#include "CGeometryClipmapsSceneObject.h"


using namespace ion;
using namespace Scene;

CGeometryClipmapsSceneObject::CGeometryClipmapsSceneObject()
{}

void CGeometryClipmapsSceneObject::Load(ion::Scene::CRenderPass * RenderPass)
{
	Log::Info("Terrain object is loading!");
	CStopWatch StopWatch;
	StopWatch.Start();

	// Generate a singular 2D vertex grid for rendering.
	// Surprisingly enough this is all the geometry we actuall use for the terrain.
	// We generate indices on the fly to specify which tiles in the grid are visible
	// for any given layer, and use a heightmap to figure out the Y coordinates.
	vector<int> VertexData;
	for (int y = 0; y < GeometrySize + 1; ++ y)
	{
		for (int x = 0; x < GeometrySize + 1; ++ x)
		{
			VertexData.push_back(x);
			VertexData.push_back(y);
		}
	}
	VertexBuffer = RenderPass->GetGraphicsAPI()->CreateVertexBuffer();
	VertexBuffer->UploadData(VertexData);
	Graphics::SInputLayoutElement InputLayout[] =
	{
		{ "vPosition", 2, Graphics::EAttributeType::Int },
	};
	VertexBuffer->SetInputLayout(InputLayout, ION_ARRAYSIZE(InputLayout));

	// Each layer is one ring of visible terrain (except the 0th layer which is a solid
	// grid).
	for (int i = 0; i < LayerCount; ++ i)
	{
		Layers.push_back(new SLayer(this, i, RenderPass));
		SLayer * Layer = Layers.back();
		SharedPointer<Graphics::IPipelineState> PipelineState = Layer->PipelineState;
		PipelineState->SetProgram(Shader);

		PipelineState->SetVertexBuffer(0, VertexBuffer);

		PipelineState->SetUniform("uDataOffset", Layer->uDataOffset);
		PipelineState->SetUniform("uDebugDisplay", uDebugDisplay);

		PipelineState->SetUniform("uScale", Layer->uScale);
		PipelineState->SetUniform("uTranslation", Layer->uTranslation);

		PipelineState->SetTexture("uHeightMap", Layer->HeightMap);
		PipelineState->SetTexture("uColorMap", Layer->ColorMap);
		PipelineState->SetTexture("uNormalMap", Layer->NormalMap);

		PipelineState->SetIndexBuffer(Layer->IndexBuffer);
		RenderPass->PreparePipelineStateForRendering(PipelineState, this);
	}

	Loaded = true;
	Log::Info("... done! (%.3f s)", StopWatch.Stop());
}

static inline bool IsEven(int x)
{
	return (x % 2) == 0;
}

static inline bool IsOdd(int x)
{
	return (x % 2) == 1;
}

void CGeometryClipmapsSceneObject::Draw(ion::Scene::CRenderPass * RenderPass)
{
	ICamera * ActiveCamera = RenderPass->GetActiveCamera();

	if (UseCameraPosition)
	{
		vec3f TransformedPosition = ActiveCamera->GetPosition();
		TransformedPosition.Transform(glm::inverse(Transformation.Get()), 1.f);
		ActiveCameraPosition = vec2i(
			(int) std::floor(TransformedPosition.X),
			(int) std::floor(TransformedPosition.Z));
	}

	///////////////////////////////////////////////////////////
	// Initial Pass - Calculate regions and generate samples //
	///////////////////////////////////////////////////////////
	
	for (int i = LayerCount - 1; i >= 0; -- i)
	{
		// We start at the largest/coarsest/farthest layer and move in/downwards
		SLayer * Layer = Layers[i];

		// Figure out how far we need to move the active region
		vec2i const DesiredActiveRegion = Layer->GetDesiredActiveRegion(ActiveCameraPosition);
		vec2i const DataOffsetMove = DesiredActiveRegion - Layer->ActiveRegion.Position;

		if (DataOffsetMove.X != 0 || DataOffsetMove.Y != 0)
		{
			Layer->SetActiveRegion(DesiredActiveRegion);
			Layer->GenerateAndUploadNewData(DataOffsetMove);
		}
	}


	///////////////////////////////////////////////
	// Second Pass - Generate Indices and Render //
	///////////////////////////////////////////////

	for (auto it = Layers.begin(); it != Layers.end(); ++ it)
	{
		SLayer * const Layer = * it;

		Layer->Visible = false;

		if (! Layer->Active)
			continue;

		if (Layer->Level < DrawLevel)
			continue;
		
		Layer->Visible = true;

		SLayer * NextLevelDown = nullptr;
		SRect2i NextLevelActiveRegion;
		if (Layer->Level > 0)
		{
			NextLevelDown = *(it - 1);
			if (NextLevelDown->Active)
			{
				NextLevelActiveRegion = ((* (it - 1))->ActiveRegion);
				NextLevelActiveRegion.Position /= 2;
				NextLevelActiveRegion.Size /= 2;
			}
			else
			{
				NextLevelDown = nullptr;
			}
		}

		//// Index Buffer ////
		Layer->IndexData.clear();
		for (int x = Layer->ActiveRegion.Position.X; x < Layer->ActiveRegion.OtherCorner().X; ++ x)
		{
			for (int y = Layer->ActiveRegion.Position.Y; y < Layer->ActiveRegion.OtherCorner().Y; ++ y)
			{
				// Check if next level down is active here - if so, skip triangles
				if (Layer->Level && (* (it - 1))->Active)
				{
					bool const IsInOtherActiveRegion =
						x >= NextLevelActiveRegion.Position.X &&
						x < NextLevelActiveRegion.OtherCorner().X &&
						y >= NextLevelActiveRegion.Position.Y &&
						y < NextLevelActiveRegion.OtherCorner().Y;

					if (IsInOtherActiveRegion && NextLevelActiveRegion.Size.X && NextLevelActiveRegion.Size.Y)
						continue;
				}

				int const X = x - Layer->ActiveRegion.Position.X;
				int const Y = y - Layer->ActiveRegion.Position.Y;

				if (X > GeometrySize || Y > GeometrySize || X < 0 || Y < 0)
				{
					printf("Error: Out of bound index-buffer location\n");
					continue;
				}

				bool TopEdge = false;
				bool LeftEdge = false;
				bool BottomEdge = false;
				bool RightEdge = false;
				if (Y == 0)
					TopEdge = true;
				if (Y == GeometrySize - 1)
					BottomEdge = true;
				if (X == 0)
					LeftEdge = true;
				if (X == GeometrySize - 1)
					RightEdge = true;

				// Standard LL
				if ((! LeftEdge) && 
					(! RightEdge || (IsOdd(Y))) && 
					(! TopEdge || (IsEven(X))) &&
					(! BottomEdge))
				{
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 1));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 1));
				}

				// Standard UR
				if ((! LeftEdge || (IsEven(Y))) && 
					(! RightEdge) && 
					(! TopEdge) &&
					(! BottomEdge || (IsOdd(X))))
				{
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 1));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 0));
				}

				// Bridge Up
				if (TopEdge && IsEven(X))
				{
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 1));
					Layer->IndexData.push_back(X + 2 + (GeometrySize + 1) * (Y + 0));
				}

				// Bridge Down
				if (BottomEdge && IsEven(X))
				{
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 1));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 2 + (GeometrySize + 1) * (Y + 1));
				}

				// Bridge Left
				if (LeftEdge && IsEven(Y))
				{
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 2));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 1));
				}

				// Bridge Right
				if (RightEdge && IsEven(Y))
				{
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 1));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 2));
				}

				// Odd-Facing Left/Top
				if ((LeftEdge && IsOdd(Y) && ! BottomEdge) || (TopEdge && IsOdd(X) && ! RightEdge))
				{
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 1));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 1));
				}

				// Odd-Facing Right/Bottom
				if ((RightEdge && IsEven(Y) && ! TopEdge) || (BottomEdge && IsEven(X) && ! LeftEdge))
				{
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 0));
					Layer->IndexData.push_back(X + 0 + (GeometrySize + 1) * (Y + 1));
					Layer->IndexData.push_back(X + 1 + (GeometrySize + 1) * (Y + 0));
				}
			}
		}
		Layer->IndexBuffer->UploadData(Layer->IndexData.data(), Layer->IndexData.size(), ion::Graphics::EValueType::UnsignedInt32);

		Layer->uDataOffset = Layer->DataOffset + 1;
		
		Layer->uTranslation = vec3f(
			(float) (Layer->ActiveRegion.Position.X * Layer->ScaleFactor),
			0.f,
			(float) (Layer->ActiveRegion.Position.Y * Layer->ScaleFactor));

		Layer->uScale = vec3f(
			(float) Layer->ScaleFactor,
			1,
			(float) Layer->ScaleFactor);
	}

	for (auto Layer : Layers)
	{
		if (Layer->Visible)
		{
			RenderPass->SubmitPipelineStateForRendering(Layer->PipelineState, this);
		}
	}
}

void CGeometryClipmapsSceneObject::SetWireframeEnabled(bool const Enabled)
{
	for (auto Layer : Layers)
	{
		if (Layer->PipelineState)
		{
			Layer->PipelineState->SetFeatureEnabled(ion::Graphics::EDrawFeature::Wireframe, Enabled);
		}
	}
}

vec3f CGeometryClipmapsSceneObject::IHeightInput::GetTerrainNormal(vec2i const & Position)
{
	double const s01 = GetTerrainHeight(Position + vec2i(-1, 0));
	double const s21 = GetTerrainHeight(Position + vec2i(+1, 0));
	double const s10 = GetTerrainHeight(Position + vec2i(0, -1));
	double const s12 = GetTerrainHeight(Position + vec2i(0, +1));

	vec3d const va = Normalize(vec3d(2, s21 - s01, 0));
	vec3d const vb = Normalize(vec3d(0, s12 - s10, 2));
	return Normalize(Cross(vb, va));
}
