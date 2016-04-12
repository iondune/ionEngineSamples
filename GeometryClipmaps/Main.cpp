
#include "CApplication.h"


int main()
{
	Log::AddDefaultOutputs();

	SingletonPointer<CApplication> Application;
	Application->Run();
	return 0;
}
