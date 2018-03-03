
#include "SSkinnedMeshBuffer.h"


using namespace ion;


namespace ion
{

	SSkinnedMeshBuffer::SSkinnedMeshBuffer()
	{}

	void SSkinnedMeshBuffer::LoadDataIntoBuffers(SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext)
	{
		SingletonPointer<CGraphicsAPI> GraphicsAPI;

		VertexBuffer = GraphicsAPI->CreateVertexBuffer();
		{
			vector<float> Data;
			size_t DataIndex = 0;

			const int numJoints = 4;

			Data.resize(Vertices.size() * (11 + numJoints * 2));
			for (auto it = Vertices.begin(); it != Vertices.end(); ++ it)
			{
				for (uint j = 0; j < 3; ++ j)
					Data[DataIndex++] = it->Position[j];

				for (uint j = 0; j < 3; ++ j)
					Data[DataIndex++] = it->Normal[j];

				for (uint j = 0; j < 3; ++ j)
					Data[DataIndex++] = it->Color[j];

				for (uint j = 0; j < 2; ++ j)
					Data[DataIndex++] = it->TextureCoordinates[j];


				for (uint j = 0; j < numJoints; ++ j)
					Data[DataIndex++] = ((it->Bones.size() > j) ? it->Bones[j].Weight : 0.f);

				for (uint j = 0; j < numJoints; ++ j)
					Data[DataIndex++] = (float) ((it->Bones.size() > j) ? it->Bones[j].Index : -1);

				if (it->Bones.size() > numJoints)
				{
					Log::Warn("Vertex in mesh has more than two bone weights = %d", it->Bones.size());
				}

			}
			VertexBuffer->UploadData(Data);
			Graphics::SInputLayoutElement InputLayout[] =
			{
				{ "vPosition",     3, Graphics::EAttributeType::Float },
				{ "vNormal",       3, Graphics::EAttributeType::Float },
				{ "vColor",        3, Graphics::EAttributeType::Float },
				{ "vTexCoords",    2, Graphics::EAttributeType::Float },
				{ "vBoneWeights",  numJoints, Graphics::EAttributeType::Float },
				{ "vBoneIndices",  numJoints, Graphics::EAttributeType::Float },
			};
			VertexBuffer->SetInputLayout(InputLayout, ION_ARRAYSIZE(InputLayout));
		}

		IndexBuffer = GraphicsAPI->CreateIndexBuffer();
		{
			vector<uint> Data;
			size_t DataIndex = 0;

			Data.resize(Triangles.size() * 3);
			for (auto it = Triangles.begin(); it != Triangles.end(); ++ it)
			{
				for (uint j = 0; j < 3; ++ j)
					Data[DataIndex++] = it->Indices[j];
			}
			IndexBuffer->UploadData(Data);
		}

		PipelineState = GraphicsContext->CreatePipelineState();
		PipelineState->SetVertexBuffer(0, VertexBuffer);
		PipelineState->SetIndexBuffer(IndexBuffer);
	}

	void SSkinnedMeshBuffer::WriteObjMesh(std::string const & fileName)
	{
		std::ofstream File(fileName);

		File << "#vertices" << std::endl;
		for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
			File << "v " << it->Position.X << " " << it->Position.Y << " " << it->Position.Z << std::endl;

		File << "#normals" << std::endl;
		for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
			File << "vn " << it->Normal.X << " " << it->Normal.Y << " " << it->Normal.Z << std::endl;

		File << std::endl << "#faces" << std::endl;
		for (std::vector<STriangle>::iterator it = Triangles.begin(); it != Triangles.end(); ++ it)
			File << "f " <<
			it->Indices[0] + 1 << "//" << it->Indices[0] + 1 << " " <<
			it->Indices[1] + 1 << "//" << it->Indices[1] + 1 << " " <<
			it->Indices[2] + 1 << "//" << it->Indices[2] + 1 << std::endl;

		File.close();
	}

}
