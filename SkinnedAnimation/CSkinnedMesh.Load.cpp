
#include "CSkinnedMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



namespace ion
{

	SSkinnedMeshBuffer * LoadBuffer(aiNode * RootNode, aiMesh * Mesh);
	SSkinnedMeshNode * TraverseMesh(aiScene const * Scene, aiNode * Node, vector<SSkinnedMeshBuffer *> const & Buffers);


	CSkinnedMesh * CSkinnedMesh::Load(std::string const & FileName)
	{
		Assimp::Importer Importer;

		unsigned int pFlags =
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_GenNormals;

		aiScene const * const Scene = Importer.ReadFile(FileName, pFlags);

		if (! Scene)
		{
			std::cerr << "Failed to import mesh file '" << FileName << "': " << Importer.GetErrorString() << std::endl;
			return 0;
		}

		CSkinnedMesh * Result = new CSkinnedMesh();

		for (uint i = 0; i < Scene->mNumMeshes; ++ i)
			Result->Buffers.push_back(LoadBuffer(Scene->mRootNode, Scene->mMeshes[i]));

		Result->Root = TraverseMesh(Scene, Scene->mRootNode, Result->Buffers);

		return Result;
	}


	glm::mat4 AItoGLM(aiMatrix4x4 const & ai)
	{
		return glm::mat4(
			ai.a1, ai.b1, ai.c1, ai.d1,
			ai.a2, ai.b2, ai.c2, ai.d2,
			ai.a3, ai.b3, ai.c3, ai.d3,
			ai.a4, ai.b4, ai.c4, ai.d4
		);
	}

	void FindParentName(string const & ChildName, aiNode * Node, string & OutString, glm::mat4 & OutMatrix)
	{
		for (uint i = 0; i < Node->mNumChildren; ++ i)
		{
			if (ChildName == Node->mChildren[i]->mName.C_Str())
			{
				OutString = Node->mName.C_Str();
				OutMatrix = AItoGLM(Node->mChildren[i]->mTransformation);
			}
			else
			{
				FindParentName(ChildName, Node->mChildren[i], OutString, OutMatrix);
				if (OutString.size())
					return;
			}
		}
	}

	SSkinnedMeshBuffer * LoadBuffer(aiNode * RootNode, aiMesh * Mesh)
	{
		SSkinnedMeshBuffer * Buffer = new SSkinnedMeshBuffer();

		// Vertices
		Buffer->Vertices.reserve(Mesh->mNumVertices);
		for (uint j = 0; j < Mesh->mNumVertices; ++ j)
		{
			SSkinnedMeshBuffer::SVertex Vertex;
			Vertex.Position = vec3f(Mesh->mVertices[j].x, Mesh->mVertices[j].y, Mesh->mVertices[j].z);
			if (Mesh->HasNormals())
				Vertex.Normal = vec3f(Mesh->mNormals[j].x, Mesh->mNormals[j].y, Mesh->mNormals[j].z);
			if (Mesh->HasTextureCoords(0))
				Vertex.TextureCoordinates = vec2f(Mesh->mTextureCoords[0][j].x, Mesh->mTextureCoords[0][j].y);
			if (Mesh->HasVertexColors(0))
				Vertex.Color = color4f(Mesh->mColors[0][j].r, Mesh->mColors[0][j].g, Mesh->mColors[0][j].b, Mesh->mColors[0][j].a);

			Buffer->Vertices.push_back(Vertex);
		}

		// Faces
		Buffer->Triangles.reserve(Mesh->mNumFaces);
		for (uint j = 0; j < Mesh->mNumFaces; ++ j)
		{
			SSkinnedMeshBuffer::STriangle Triangle;
			for (uint k = 0; k < 3; ++ k)
				Triangle.Indices[k] = Mesh->mFaces[j].mIndices[k];

			Buffer->Triangles.push_back(Triangle);
		}

		// Bones
		for (uint j = 0; j < Mesh->mNumBones; ++ j)
		{
			SSkinnedMeshBuffer::SBone Bone;
			Bone.Name = Mesh->mBones[j]->mName.C_Str();
			Bone.OffsetMatrix = AItoGLM(Mesh->mBones[j]->mOffsetMatrix);

			for (uint k = 0; k < Mesh->mBones[j]->mNumWeights; ++ k)
			{
				SSkinnedMeshBuffer::SVertex::SBoneBinding Binding;
				Binding.Index = j;
				Binding.Weight = Mesh->mBones[j]->mWeights[k].mWeight;

				Buffer->Vertices[Mesh->mBones[j]->mWeights[k].mVertexId].Bones.push_back(Binding);
			}

			Buffer->Bones.push_back(Bone);
		}

		for (auto & Bone : Buffer->Bones)
		{
			string ParentName;
			glm::mat4 MyTransform;
			FindParentName(Bone.Name, RootNode, ParentName, MyTransform);

			for (auto & Parent : Buffer->Bones)
			{
				if (ParentName == Parent.Name)
					Bone.Parent = & Parent;
			}
			Bone.RelativeTransform = MyTransform;
		}

		return Buffer;
	}

	SSkinnedMeshNode * TraverseMesh(aiScene const * Scene, aiNode * Node, vector<SSkinnedMeshBuffer *> const & Buffers)
	{
		SSkinnedMeshNode * Result = new SSkinnedMeshNode();

		Result->Transformation = AItoGLM(Node->mTransformation);

		if (Node->mNumMeshes)
		{
			Result->Buffer = Buffers[Node->mMeshes[0]];

			if (Node->mNumMeshes > 1)
			{
				Log::Error("Node has more than one mesh!");
			}
		}

		for (uint i = 0; i < Node->mNumChildren; ++ i)
			Result->AddChild(TraverseMesh(Scene, Node->mChildren[i], Buffers));

		return Result;
	}

}
