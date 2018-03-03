
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
		JointSpheresObject->SetMesh(CGeometryCreator::CreateSphere(0.05f, 4, 2));
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
			Color::Hex(0xBE90D4),
			Color::Hex(0xD490C0),
			Color::Hex(0xD49090),
			Color::Hex(0x909AD4),
			Color::Hex(0x90BDD4),
			Color::Hex(0x90D4BD),
			Color::Hex(0x96D490),
			Color::Hex(0xD4D390),
			Color::Hex(0xD4B390),
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
