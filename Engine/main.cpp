#include <iostream>

#include "EngineCore.h"
#include "Common.h"

using namespace std;

int main()
{
	if (!EngineCore::Get().Initialize())
	{
		cout << "EngineCore initialization failed." << endl;
		return -1;
	}

	return EngineCore::Get().Run();
}