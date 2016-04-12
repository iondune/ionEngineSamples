
#include "CGeometryClipmapsSceneObject.h"


using namespace ion;
using namespace Scene;

CGeometryClipmapsSceneObject::CGeometryClipmapsSceneObject()
{
	// Since we are sampling a heightmap texture we need to know the texel size
	// If we used texelFetch in the vertex shader instead we would not need to know
	// this necessarily. But that's also assuming we don't want to do any sort of
	// bicubic interpolation. An optimized version of bicubic interpolation would use
	// the built in interpolation engine and not just fetch singular texels.
	uTexelSize = 1.f / HeightmapResolution;
	uHeightmapResolution = (float) HeightmapResolution;
}

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

		PipelineState->SetUniform("uHeightmapResolution", uHeightmapResolution);
		PipelineState->SetUniform("uModelMatrix", Layer->uTransformation);
		PipelineState->SetUniform("uDataOffset", Layer->uDataOffset);
		PipelineState->SetUniform("uSamplingMode", uSamplingMode);
		PipelineState->SetUniform("uDebugDisplay", uDebugDisplay);

		//PipelineState->SetUniform("uScaleFactor", Layer->uScaleFactor);
		//PipelineState->SetUniform("uTexelSize", uTexelSize);

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
		// This constant specifies how far the camera should move in any given direction
		// before we yank the camera back towards the origin and apply a global offset to
		// the terrain in the same direction.
		// 
		// E.g. if the camera moves too far in the positive X direction, we pull it back
		// a certain distance and then move the entire terrain back in the same distance.
		// This creates the illusion of moving across the terrain while keeping the camera
		// coordinates (at least X and Z) relatively small.
		//
		// If we don't do this, if we try to explore terrain far away from the origin,
		// lack of precision in the floating point coordinates of the camera make it
		// difficult to move slowly and cause graphical glitches.
		//
		// The value here is entirely arbitrary. In theory it could be as small as one,
		// causing a global offset change ever unit of movement. It could also be
		// extraordinarily large, though if it's too large we'll get the above precision
		// issues at the threshold anyway.
		static int const UpdateRegion = 100;

		vec3f CameraPosition = ActiveCamera->GetPosition();
		
		//for (int i = 0; i < 3; i += 2) // X=0, then Z=2
		//{
		//	while (CameraPosition[i] >= UpdateRegion)
		//	{
		//		CameraPosition[i] -= UpdateRegion;
		//		GlobalSystemOffset[i / 2] += UpdateRegion;
		//	}
		//	while (CameraPosition[i] < 0)
		//	{
		//		CameraPosition[i] += UpdateRegion;
		//		GlobalSystemOffset[i / 2] -= UpdateRegion;
		//	}
		//}

		ActiveCamera->SetPosition(CameraPosition);
		ActiveCameraPosition = ActiveCamera->GetPosition();
	}
	
	if (DoCameraUpdate)
	{
		// Only if we are tracking camera position:
		//
		// Add the global offset to the active camera position (this is where the
		// camera would be if we weren't adjusting above).
		ActiveCameraPositionAfterGlobalOffset = vec2i(
			(int) std::floor(ActiveCameraPosition.X) + GlobalSystemOffset.X,
			(int) std::floor(ActiveCameraPosition.Z) + GlobalSystemOffset.Y);
	}


	///////////////////////////////////////////////////////////
	// Initial Pass - Calculate regions and generate samples //
	///////////////////////////////////////////////////////////

	// The budget is the number of texels of heightmap data we can load before we
	// quit and start disabiling layers instead of loading the data they need to
	// be visible.
	//
	// This is a heuristic - if we have a remaining budget of `1` and an update
	// would take `1000` texels, we still do the update.
	//static int const StartingBudget = 1;
	//int Budget = StartingBudget;

	for (auto it = Layers.begin(); it != Layers.end(); ++ it)
		(* it)->DataUpdated = false;

	for (int i = LayerCount - 1; i >= 0; -- i)
	{
		// We start at the largest/coarsest/farthest layer and move in/downwards
		SLayer * Layer = Layers[i];

		// Figure out how far we need to move the active region
		vec2i const DesiredActiveRegion = Layer->GetDesiredActiveRegion(ActiveCameraPositionAfterGlobalOffset);
		vec2i const DataOffsetMove = DesiredActiveRegion - Layer->ActiveRegion.Position;

		if (DataOffsetMove.X == 0 && DataOffsetMove.Y == 0)
		{
			Layer->DataUpdated = true;
		}
		else //if (Budget > 0)
		{
			Layer->SetActiveRegion(DesiredActiveRegion);
			/*Budget -= */Layer->GenerateAndUploadNewData(DataOffsetMove);
			Layer->DataUpdated = true;
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

		if (! Layer->DataUpdated)
			continue;

		if (Layer->Level < DrawLevel)
			continue;
		
		Layer->Visible = true;

		SLayer * NextLevelDown = nullptr;
		SRect2i NextLevelActiveRegion;
		if (Layer->Level > 0)
		{
			NextLevelDown = *(it - 1);
			if (NextLevelDown->Active && NextLevelDown->DataUpdated)
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
				if (Layer->Level && (* (it - 1))->Active && (* (it - 1))->DataUpdated)
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
		
		vec3f const Translation = vec3f(
			(float) (Layer->ActiveRegion.Position.X * Layer->ScaleFactor - GlobalSystemOffset.X),
			0.f,
			(float) (Layer->ActiveRegion.Position.Y * Layer->ScaleFactor - GlobalSystemOffset.Y));

		vec3f const Scale = vec3f(
			(float) Layer->ScaleFactor,
			1,
			(float) Layer->ScaleFactor);

		Layer->Transformation.SetTranslation(Translation);
		Layer->Transformation.SetScale(Scale);
		Layer->uTransformation = Layer->Transformation;
		Layer->uScaleFactor = Layer->ScaleFactor;
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
