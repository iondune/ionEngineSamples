
#include "SSkinnedMeshBuffer.h"


namespace ion
{

	SSkinnedMeshBuffer::STriangle::STriangle()
	{
		Indices[0] = 0;
		Indices[1] = 0;
		Indices[2] = 0;
	}

	SSkinnedMeshBuffer::STriangle::STriangle(uint const index0, uint const index1, uint const index2)
	{
		Indices[0] = index0;
		Indices[1] = index1;
		Indices[2] = index2;
	}

}
