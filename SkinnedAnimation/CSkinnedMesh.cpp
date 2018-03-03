
#include "CSkinnedMesh.h"


namespace ion
{

	glm::mat4 CSkinnedMesh::CJoint::GetAbsoluteTransform() const
	{
		glm::mat4 AbsoluteTransform = RelativeTransform * AnimationTransform.Get();

		if (Parent)
			AbsoluteTransform = Parent->GetAbsoluteTransform() * AbsoluteTransform;

		return AbsoluteTransform;
	}


	CSkinnedMesh::CSkinnedMesh()
	{}

	static void RecurseJointsOnMesh(SSkinnedMeshNode * Node, vector<CSkinnedMesh::CJoint *> & Joints)
	{
		map<SSkinnedMeshBuffer::SBone *, CSkinnedMesh::CJoint *> JointMap;
		map<CSkinnedMesh::CJoint *, SSkinnedMeshBuffer::SBone *> BoneMap;
		if (Node->Buffer)
		{
			for (int i = 0; i < Node->Buffer->Bones.size(); ++ i)
			{
				CSkinnedMesh::CJoint * Joint = new CSkinnedMesh::CJoint();
				Joint->Name = Node->Buffer->Bones[i].Name;
				Joint->OffsetTransform = Node->Buffer->Bones[i].OffsetMatrix;
				Joint->RelativeTransform = Node->Buffer->Bones[i].RelativeTransform;
				Joints.push_back(Joint);
				JointMap[& Node->Buffer->Bones[i]] = Joint;
				BoneMap[Joint] = & Node->Buffer->Bones[i];
			}
		}

		if (JointMap.size())
		{
			for (auto Joint : Joints)
			{
				auto Parent = BoneMap[Joint]->Parent;
				if (Parent)
				{
					Joint->Parent = JointMap[Parent];
					JointMap[Parent]->Children.push_back(Joint);
				}
			}
		}

		for (auto Child : Node->GetChildren())
		{
			RecurseJointsOnMesh(Child, Joints);
		}
	};

	void CSkinnedMesh::Load(ion::Scene::CRenderPass * RenderPass)
	{
		Joints.clear();
		JointNames.clear();

		// Load Joints
		RecurseJointsOnMesh(Root, Joints);
		for (auto Joint : Joints)
		{
			JointNames[Joint->Name] = Joint;
		}
		uSkinningMatrices.Get().resize(Joints.size());
		for (uint i = 0; i < Joints.size(); ++ i)
		{
			Joints[i]->SkinningMatrix = & uSkinningMatrices.Get()[i];
		}

		Update(RenderPass->GetGraphicsContext());

		Root->Load(RenderPass, this);

		Loaded[RenderPass] = true;
	}

	void CSkinnedMesh::Draw(ion::Scene::CRenderPass * RenderPass)
	{
		for (auto Joint : Joints)
		{
			*(Joint->SkinningMatrix) = Joint->GetAbsoluteTransform() * Joint->OffsetTransform;
		}

		Root->Draw(RenderPass, this);
	}

	void CSkinnedMesh::Update(SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext)
	{
		LoadDataIntoBuffers(GraphicsContext);
		UpdateNodeTransformations();
	}

	void CSkinnedMesh::LoadDataIntoBuffers(SharedPointer<ion::Graphics::IGraphicsContext> GraphicsContext)
	{
		std::for_each(Buffers.begin(), Buffers.end(), [GraphicsContext](SSkinnedMeshBuffer * Buffer) { Buffer->LoadDataIntoBuffers(GraphicsContext); });
	}

	void CSkinnedMesh::UpdateNodeTransformations()
	{
		Root->CalculateAbsoluteTransformation();
	}

	CSkinnedMesh::CJoint * CSkinnedMesh::GetJoint(uint const Index)
	{
		return Joints[Index];
	}

	CSkinnedMesh::CJoint * CSkinnedMesh::GetJoint(string const & Name)
	{
		return ion::ConditionalMapAccess(JointNames, Name);
	}

	int CSkinnedMesh::GetJointCount() const
	{
		return (int) Joints.size();
	}

	void CSkinnedMesh::SeparateTriangles()
	{
		for (auto Buffer : Buffers)
		{
			std::vector<SSkinnedMeshBuffer::SVertex> newVertices;
			std::vector<SSkinnedMeshBuffer::STriangle> newTriangles;

			for (auto it = Buffer->Triangles.begin(); it != Buffer->Triangles.end(); ++ it)
			{
				for (int i = 0; i < 3; ++ i)
					newVertices.push_back(Buffer->Vertices[it->Indices[i]]);
			}

			for (uint i = 0; i < newVertices.size() / 3; ++ i)
			{
				SSkinnedMeshBuffer::STriangle tri;
				tri.Indices[0] = i * 3;
				tri.Indices[1] = i * 3 + 1;
				tri.Indices[2] = i * 3 + 2;
				newTriangles.push_back(tri);
			}

			Buffer->Vertices = newVertices;
			Buffer->Triangles = newTriangles;
		}
	}

	void CSkinnedMesh::CalculateNormalsPerFace()
	{
		for (auto Buffer : Buffers)
		{
			for (auto it = Buffer->Triangles.begin(); it != Buffer->Triangles.end(); ++ it)
			{
				it->Normal = (Buffer->Vertices[it->Indices[1]].Position - Buffer->Vertices[it->Indices[0]].Position).
					CrossProduct(Buffer->Vertices[it->Indices[2]].Position - Buffer->Vertices[it->Indices[0]].Position);

				Buffer->Vertices[it->Indices[0]].Normal =
					Buffer->Vertices[it->Indices[1]].Normal =
					Buffer->Vertices[it->Indices[2]].Normal =
					it->Normal;
			}

			for (std::vector<SSkinnedMeshBuffer::SVertex>::iterator it = Buffer->Vertices.begin(); it != Buffer->Vertices.end(); ++ it)
			{
				it->Normal.Normalize();
			}
		}
	}


}
