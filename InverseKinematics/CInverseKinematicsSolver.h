#pragma once

#include <ionCore.h>


namespace ion
{

	class CInverseKinematicsSolver
	{

	public:

		struct SJoint
		{
			SJoint * Parent = nullptr;
			vec3f Rotation = vec3f(0, 3.1415f, 0);
			float Length = 0.75f;

			glm::mat4 getLocalTransformation()
			{
				glm::mat4 Trans = glm::translate(glm::mat4(1.f), glm::vec3(Length, 0, 0));

				glm::mat4 Rot = glm::mat4(1.f);
				Rot = glm::rotate(Rot, Rotation.Z, glm::vec3(0, 0, 1));
				Rot = glm::rotate(Rot, Rotation.Y, glm::vec3(0, 1, 0));
				Rot = glm::rotate(Rot, Rotation.X, glm::vec3(1, 0, 0));

				return Rot * Trans;
			}

			glm::mat4 getLocalHalfTransformation()
			{
				glm::mat4 Trans = glm::translate(glm::mat4(1.f), glm::vec3(Length / 2.f, 0, 0));

				glm::mat4 Rot = glm::mat4(1.f);
				Rot = glm::rotate(Rot, Rotation.Z, glm::vec3(0, 0, 1));
				Rot = glm::rotate(Rot, Rotation.Y, glm::vec3(0, 1, 0));
				Rot = glm::rotate(Rot, Rotation.X, glm::vec3(1, 0, 0));

				return Rot * Trans;
			}

			glm::mat4 getTransformation()
			{
				glm::mat4 Trans = getLocalTransformation();

				if (Parent)
					Trans = Parent->getTransformation() * Trans;

				return Trans;
			}

			glm::mat4 getHalfTransformation()
			{
				glm::mat4 Trans = getLocalHalfTransformation();

				if (Parent)
					Trans = Parent->getTransformation() * Trans;

				return Trans;
			}

			vec3f const getLocation()
			{
				glm::vec4 v(0, 0, 0, 1);
				v = getTransformation() * v;

				return vec3f(v.x, v.y, v.z);
			}

			vec3f const getHalfLocation()
			{
				glm::vec4 v(0, 0, 0, 1);
				v = getHalfTransformation() * v;

				return vec3f(v.x, v.y, v.z);
			}
		};

		vector<SJoint *> Joints;

		float GetValue(vec3f const & GoalPosition)
		{
			vec3f const HandLoc = Joints.back()->getLocation();
			return Sq(GoalPosition.GetDistanceFrom(HandLoc));
		}

		void Run(vec3f const & GoalPosition)
		{
			float Delta = DegToRad(30.f);
			for (int i = 0; i < 500; ++ i)
			{
				for (int t = 0; t < Joints.size(); ++t)
				{
					for (int u = 0; u < 3; ++ u)
					{
						float const LastValue = GetValue(GoalPosition);
						Joints[t]->Rotation[u] += Delta;
						float const AddValue = GetValue(GoalPosition);
						Joints[t]->Rotation[u] -= 2 * Delta;
						float const SubValue = GetValue(GoalPosition);
						Joints[t]->Rotation[u] += Delta;

						if (LastValue < AddValue && LastValue < SubValue)
						{
						}
						else if (AddValue < SubValue)
						{
							Joints[t]->Rotation[u] += Delta;
						}
						else if (SubValue < AddValue)
						{
							Joints[t]->Rotation[u] -= Delta;
						}
						else
						{
						}
					}
				}
				Delta /= 1.01f;
			}
		}

	};

}
