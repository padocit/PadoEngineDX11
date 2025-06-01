#include <iostream>

#include "Sample_Basic.h"
#include "Common.h"

using namespace std;

int main()
{
    unique_ptr<Engine> sample = make_unique<Sample_Basic>();

	if (!Engine::Create(move(sample)))
	{
        cout << "Engine already created." << endl;
        return -1;
	}

	Engine* engine = Engine::Get();
	if (!engine->Initialize(1024, 1024))
	{
		cout << "Initialization failed." << endl;
		return -1;
	}

	return engine->Run();
}