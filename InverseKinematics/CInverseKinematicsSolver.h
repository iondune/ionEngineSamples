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
			vec3f Rotation = vec3f(0, 0, 0);
			float Length = 0.75f;

			// For FABRIK
			vec3f InboardLocation, OutboardLocation;

			glm::mat4 GetLocalOutboardTransformation()
			{
				glm::mat4 Trans = glm::translate(glm::mat4(1.f), glm::vec3(Length, 0, 0));

				glm::mat4 Rot = glm::mat4(1.f);
				Rot = glm::rotate(Rot, Rotation.Z, glm::vec3(0, 0, 1));
				Rot = glm::rotate(Rot, Rotation.Y, glm::vec3(0, 1, 0));
				Rot = glm::rotate(Rot, Rotation.X, glm::vec3(1, 0, 0));

				return Rot * Trans;
			}

			glm::mat4 GetLocalHalfpointTransformation()
			{
				glm::mat4 Trans = glm::translate(glm::mat4(1.f), glm::vec3(Length / 2.f, 0, 0));

				glm::mat4 Rot = glm::mat4(1.f);
				Rot = glm::rotate(Rot, Rotation.Z, glm::vec3(0, 0, 1));
				Rot = glm::rotate(Rot, Rotation.Y, glm::vec3(0, 1, 0));
				Rot = glm::rotate(Rot, Rotation.X, glm::vec3(1, 0, 0));

				return Rot * Trans;
			}

			glm::mat4 GetLocalInboardTransformation()
			{
				glm::mat4 Rot = glm::mat4(1.f);
				Rot = glm::rotate(Rot, Rotation.Z, glm::vec3(0, 0, 1));
				Rot = glm::rotate(Rot, Rotation.Y, glm::vec3(0, 1, 0));
				Rot = glm::rotate(Rot, Rotation.X, glm::vec3(1, 0, 0));

				return Rot;
			}

			glm::mat4 GetOutboardTransformation()
			{
				glm::mat4 Trans = GetLocalOutboardTransformation();

				if (Parent)
				{
					Trans = Parent->GetOutboardTransformation() * Trans;
				}

				return Trans;
			}

			glm::mat4 GetHalfpointTransformation()
			{
				glm::mat4 Trans = GetLocalHalfpointTransformation();

				if (Parent)
					Trans = Parent->GetOutboardTransformation() * Trans;

				return Trans;
			}

			glm::mat4 GetInboardTransformation()
			{
				glm::mat4 Trans = GetLocalInboardTransformation();

				if (Parent)
					Trans = Parent->GetOutboardTransformation() * Trans;

				return Trans;
			}

			vec3f const GetOutboardLocation()
			{
				glm::vec4 v(0, 0, 0, 1);
				v = GetOutboardTransformation() * v;

				return vec3f(v.x, v.y, v.z);
			}

			vec3f const GetHalfpointLocation()
			{
				glm::vec4 v(0, 0, 0, 1);
				v = GetHalfpointTransformation() * v;

				return vec3f(v.x, v.y, v.z);
			}

			vec3f const GetInboardLocation()
			{
				glm::vec4 v(0, 0, 0, 1);
				v = GetInboardTransformation() * v;

				return vec3f(v.x, v.y, v.z);
			}
		};

		vector<SJoint *> Joints;
		float Delta = DegToRad(30.f);
		bool FullReset = false;
		bool UseFABRIK = true;

		float GetValue(vec3f const & GoalPosition)
		{
			vec3f const HandLoc = Joints.back()->GetOutboardLocation();
			return Sq(GoalPosition.GetDistanceFrom(HandLoc));
		}

		void RunIK(vec3f const & GoalPosition)
		{
			Delta = DegToRad(30.f);

			if (FullReset)
			{
				for (auto Joint : Joints)
				{
					Joint->Rotation = 0;
				}
			}

			for (int i = 0; i < 500; ++ i)
			{
				StepIK(GoalPosition);
			}
		}

		void StepIK(vec3f const & GoalPosition)
		{
			if (UseFABRIK)
			{
				StepFABRIK(GoalPosition);
			}
			else
			{
				StepCCD(GoalPosition);
			}
		}

		void StepCCD(vec3f const & GoalPosition)
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

				Joints[t]->InboardLocation = Joints[t]->GetInboardLocation();
				Joints[t]->OutboardLocation = Joints[t]->GetOutboardLocation();
			}

			Delta /= 1.01f;
		}

		void StepFABRIK(vec3f const & GoalPosition)
		{
			vec3f RootPosition = Joints[0]->GetInboardLocation();

			// First pass - front to back
			vec3f CurrentGoal = GoalPosition;

			for (int t = (int) Joints.size() - 1; t >= 0; -- t)
			{
				Joints[t]->InboardLocation = Joints[t]->GetInboardLocation();
				Joints[t]->OutboardLocation = Joints[t]->GetOutboardLocation();

				Joints[t]->OutboardLocation = CurrentGoal;
				vec3f const CurrentLine = Normalize(Joints[t]->OutboardLocation - Joints[t]->InboardLocation);

				Joints[t]->InboardLocation = Joints[t]->OutboardLocation - CurrentLine * Joints[t]->Length;
				CurrentGoal = Joints[t]->InboardLocation;
			}

			// Second pass - back to front
			CurrentGoal = RootPosition;

			for (int t = 0; t < Joints.size(); ++ t)
			{
				Joints[t]->InboardLocation = CurrentGoal;
				vec3f const CurrentLine = Normalize(Joints[t]->InboardLocation - Joints[t]->OutboardLocation);

				Joints[t]->OutboardLocation = Joints[t]->InboardLocation - CurrentLine * Joints[t]->Length;
				CurrentGoal = Joints[t]->OutboardLocation;
			}

			}
		}

	};

}
