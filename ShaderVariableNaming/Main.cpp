
#include "CApplication.h"


int main()
{
	ion::Log::AddDefaultOutputs();

	ion::SingletonPointer<CApplication> Application;
	Application->Run();
	return 0;
}
