
#include "CCharacterTestApplication.h"


int main()
{
	ion::Log::AddDefaultOutputs();

	ion::SingletonPointer<CCharacterTestApplication> Application;
	Application->Run();

	return 0;
}
