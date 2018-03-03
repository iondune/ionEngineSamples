
#pragma once

#include <ionCore.h>
#include <ionMath.h>
#include <ionGraphics.h>
#include <ionScene.h>

#include "SSkinnedMeshBuffer.h"
#include "SSkinnedMeshNode.h"


namespace ion
{

	class CSkinnedMesh : public Scene::ISceneObject
	{

	public:

		class CJoint
		{

		public:

			string Name;

			STransformation3 AnimationTransform;
			glm::mat4 RelativeTransform;
			glm::mat4 OffsetTransform;
			CJoint * Parent = nullptr;
			vector<CJoint *> Children;

			glm::mat4 * SkinningMatrix = nullptr;
			glm::mat4 GetAbsoluteTransform() const;

		};

		vector<SSkinnedMeshBuffer *> Buffers;
		SSkinnedMeshNode * Root = nullptr;

		vector<CJoint *> Joints;
		Graphics::CUniform<vector<glm::mat4>> uSkinningMatrices;
		map<string, CJoint *> JointNames;

		SharedPointer<Graphics::IShader> Shader;
		SharedPointer<Graphics::ITexture> Texture;

		Graphics::CUniform<bool> DebugDoSkin = true;
		Graphics::CUniform<bool> DebugShowWeightsByJoint = false;
		Graphics::CUniform<bool> DebugShowWeightsByVertex = false;
		Graphics::CUniform<int> DebugWeightSelector = 0;

		static CSkinnedMesh * Load(std::string const & FileName);

		CSkinnedMesh();

		void Load(Scene::CRenderPass * RenderPass);
		void Draw(Scene::CRenderPass * RenderPass);

		void Update(SharedPointer<Graphics::IGraphicsContext> GraphicsContext);
		void LoadDataIntoBuffers(SharedPointer<Graphics::IGraphicsContext> GraphicsContext);
		void UpdateNodeTransformations();

		CJoint * GetJoint(uint const Index);
		CJoint * GetJoint(string const & Name);
		int GetJointCount() const;

		void SeparateTriangles();
		void CalculateNormalsPerFace();

	};

}
