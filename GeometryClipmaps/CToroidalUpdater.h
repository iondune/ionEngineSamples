
#pragma once

#include <ionApplication.h>


class CToroidalUpdater
{

public:

	class ISampleUploader
	{

	public:

		virtual void UploadSample(vec2i const & LowerBound, vec2i const & UpperBound, vec2i const & NewDataOffset) = 0;

	};

	// Inputs
	ISampleUploader * SampleUploader = nullptr;
	int TextureResolution = 0;

	// Outputs
	int TotalSamplesUploaded = 0;
	vec2i FinalOffset;

	void DoUpdate(vec2i const LastPosition, vec2i const NewPosition);

protected:

	int SendSample(int const x0, int const y0, int const x1, int const y1, vec2i const & FinalOffset);

};
