
#include "CToroidalUpdater.h"


void CToroidalUpdater::DoUpdate(ion::vec2i const LastPosition, ion::vec2i const NewPosition)
{
	ion::vec2i const Move = NewPosition - LastPosition;
	TotalSamplesUploaded = 0;

	if (abs(Move.X) >= TextureResolution || abs(Move.Y) >= TextureResolution)
	{
		FinalOffset = ion::vec2i(0, 0);
		TotalSamplesUploaded += SendSample(0, 0, TextureResolution, TextureResolution, FinalOffset);
	}
	else
	{
		ion::rect2i Bounds;
		{
			ion::vec2i ActualNewPosition = NewPosition;
			if (ActualNewPosition.X < 0)
				while (ActualNewPosition.X < 0)
					ActualNewPosition.X += TextureResolution;
			else
				ActualNewPosition.X %= TextureResolution;

			if (ActualNewPosition.Y < 0)
				while (ActualNewPosition.Y < 0)
					ActualNewPosition.Y += TextureResolution;
			else
				ActualNewPosition.Y %= TextureResolution;

			Bounds.Bounds(LastPosition, ActualNewPosition);
			FinalOffset = ActualNewPosition;
		}

		bool const WrappedInX = NewPosition.X >= TextureResolution || NewPosition.X < 0;
		bool const WrappedInY = NewPosition.Y >= TextureResolution || NewPosition.Y < 0;

		if (WrappedInX && WrappedInY)
		{
			// Double wrap around, 4 quad updates around edges
			TotalSamplesUploaded += SendSample(0, 0, Bounds.OtherCorner().X, Bounds.Position.Y, FinalOffset);
			TotalSamplesUploaded += SendSample(Bounds.OtherCorner().X, 0, TextureResolution, Bounds.OtherCorner().Y, FinalOffset);
			TotalSamplesUploaded += SendSample(Bounds.Position.X, Bounds.OtherCorner().Y, TextureResolution, TextureResolution, FinalOffset);
			TotalSamplesUploaded += SendSample(0, Bounds.Position.Y, Bounds.Position.X, TextureResolution, FinalOffset);
		}
		else if (WrappedInX)
		{
			// Wrap around in X, 3 quads on horizontal edges and across
			TotalSamplesUploaded += SendSample(0, 0, Bounds.Position.X, TextureResolution, FinalOffset);
			TotalSamplesUploaded += SendSample(Bounds.Position.X, Bounds.Position.Y, Bounds.OtherCorner().X, Bounds.OtherCorner().Y, FinalOffset);
			TotalSamplesUploaded += SendSample(Bounds.OtherCorner().X, 0, TextureResolution, TextureResolution, FinalOffset);
		}
		else if (WrappedInY)
		{
			// Wrap around in Y, 3 quads on vertical edges and across
			TotalSamplesUploaded += SendSample(0, 0, TextureResolution, Bounds.Position.Y, FinalOffset);
			TotalSamplesUploaded += SendSample(Bounds.Position.X, Bounds.Position.Y, Bounds.OtherCorner().X, Bounds.OtherCorner().Y, FinalOffset);
			TotalSamplesUploaded += SendSample(0, Bounds.OtherCorner().Y, TextureResolution, TextureResolution, FinalOffset);
		}
		else
		{
			// No wrap around, 3 quads to create a plus centered on Bounds
			TotalSamplesUploaded += SendSample(Bounds.Position.X, 0, Bounds.OtherCorner().X, TextureResolution, FinalOffset);
			TotalSamplesUploaded += SendSample(0, Bounds.Position.Y, Bounds.Position.X, Bounds.OtherCorner().Y, FinalOffset);
			TotalSamplesUploaded += SendSample(Bounds.OtherCorner().X, Bounds.Position.Y, TextureResolution, Bounds.OtherCorner().Y, FinalOffset);
		}
	}
}

int CToroidalUpdater::SendSample(int const x0, int const y0, int const x1, int const y1, ion::vec2i const & NewOffset)
{
	int const Size = (x1-x0) * (y1-y0);
	if (Size)
		SampleUploader->UploadSample(ion::vec2i(x0, y0), ion::vec2i(x1, y1), NewOffset);
	return Size;
}
