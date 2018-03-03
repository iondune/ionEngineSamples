
#pragma once

#include <ionCore.h>
#include <ionGraphics.h>


namespace ion
{

	struct SSkinnedMeshBuffer
	{

		class SVertex
		{

		public:

			SVertex();
			SVertex(vec3f const & position,
				vec3f const & normal = vec3f(),
				vec2f const & texture = vec2f(),
				color4f const & color = color4f());

			vec3f Position;
			vec3f Normal;
			vec2f TextureCoordinates;

			color4f Color;

			struct SBoneBinding
			{
				int Index = -1;
				float Weight = 0;
			};

			vector<SBoneBinding> Bones;

		};

		struct STriangle
		{
			STriangle();
			STriangle(uint const index0, uint const index1, uint const index2);

			uint Indices[3];
			vec3f Normal;
		};

		struct SBone
		{

			glm::mat4 OffsetMatrix, RelativeTransform;
			string Name;
			SBone * Parent = nullptr;

		};


		SSkinnedMeshBuffer();

		void LoadDataIntoBuffers(SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext);
		void WriteObjMesh(std::string const & FileName);

		vector<SVertex> Vertices;
		vector<STriangle> Triangles;
		vector<SBone> Bones;

		SharedPointer<ion::Graphics::IVertexBuffer> VertexBuffer;
		SharedPointer<ion::Graphics::IIndexBuffer> IndexBuffer;
		SharedPointer<ion::Graphics::IPipelineState> PipelineState;

	};

}
