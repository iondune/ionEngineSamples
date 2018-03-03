
#include "CSkinnedMeshSkeletonVisualizer.h"

using namespace ion;
using namespace ion::Scene;
using namespace ion::Graphics;


namespace ion
{

	void CSkinnedMeshSkeletonVisualizer::Load(CSkinnedMesh * SkinnedMesh)
	{
		this->SkinnedMesh = SkinnedMesh;
	}

	void CSkinnedMeshSkeletonVisualizer::AddSceneObjects(
		ion::Scene::CRenderPass * RenderPass,
		SharedPointer<ion::Graphics::IShader> LineShader,
		SharedPointer<ion::Graphics::IShader> InstanceShader)
	{
		SingletonPointer<ion::CGraphicsAPI> GraphicsAPI;

		Graphics::SInputLayoutElement InstanceLayout[] =
		{
			{ "iPosition",    3, Graphics::EAttributeType::Float },
			{ "iColor",       3, Graphics::EAttributeType::Float },
		};

		JointInstanceBuffer = GraphicsAPI->CreateVertexBuffer();
		JointInstanceBuffer->SetInstancingEnabled(true);
		JointInstanceBuffer->SetInputLayout(InstanceLayout, ION_ARRAYSIZE(InstanceLayout));

		JointLinesObject = new CLineSceneObject();
		JointLinesObject->SetShader(LineShader);
		JointLinesObject->Load(RenderPass);
		JointLinesObject->SetFeatureEnabled(EDrawFeature::DisableDepthTest, true);
		JointLinesObject->SetRenderCategory(1);
		RenderPass->AddSceneObject(JointLinesObject);

		JointSpheresObject = new CSimpleMeshSceneObject();
		JointSpheresObject->SetShader(InstanceShader);
		JointSpheresObject->SetMesh(CGeometryCreator::CreateSphere(0.09f, 4, 2));
		JointSpheresObject->SetFeatureEnabled(EDrawFeature::Wireframe, true);
		JointSpheresObject->SetFeatureEnabled(EDrawFeature::DisableDepthTest, true);
		JointSpheresObject->SetVertexBuffer(1, JointInstanceBuffer);
		JointSpheresObject->SetRenderCategory(1);
		RenderPass->AddSceneObject(JointSpheresObject);
	}

	void CSkinnedMeshSkeletonVisualizer::UpdateSceneObjects()
	{
		static color3f Colors[] =
		{
			color3i(121,96,191),
			color3i(255,128,196),
			color3i(82,0,204),
			color3i(109,204,0),
			color3i(229,115,130),
			color3i(191,0,102),
			color3i(217,54,54),
			color3i(217,137,108),
			color3i(102,156,204),
			color3i(51,194,204),
			color3i(217,181,108),
			color3i(217,145,0),
			color3i(0,136,255),
			color3i(191,77,0),
			color3i(191,179,0),
			color3i(217,0,202),
			color3i(108,217,137),
		};

		int i = 0;
		vector<float> InstanceData;

		JointLinesObject->ResetLines();

		if (SkinnedMesh)
		{
			for (CSkinnedMesh::CJoint * Joint : SkinnedMesh->Joints)
			{
				color3f const Color = Colors[i++ % ION_ARRAYSIZE(Colors)];

				vec4f Position = vec4f(0, 0, 0, 1);
				Position.Transform(SkinnedMesh->GetTransformation().GetGLMMat4() * Joint->GetAbsoluteTransform());

				InstanceData.push_back(Position.X);
				InstanceData.push_back(Position.Y);
				InstanceData.push_back(Position.Z);

				InstanceData.push_back(Color.Red);
				InstanceData.push_back(Color.Green);
				InstanceData.push_back(Color.Blue);

				for (CSkinnedMesh::CJoint * Child : Joint->Children)
				{
					vec4f ChildPosition = vec4f(0, 0, 0, 1);
					ChildPosition.Transform(SkinnedMesh->GetTransformation().GetGLMMat4() * Child->GetAbsoluteTransform());

					JointLinesObject->AddLine(Position.XYZ(), ChildPosition.XYZ(), Color);
				}
			}
		}


		JointInstanceBuffer->UploadData(InstanceData);
		JointSpheresObject->SetInstanceCount(i);
	}

	void CSkinnedMeshSkeletonVisualizer::SetVisible(bool const Visible)
	{
		JointLinesObject->SetVisible(Visible);
		JointSpheresObject->SetVisible(Visible);
	}

}
