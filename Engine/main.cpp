#include <iostream>

#include "Common.h"
#include "Sample_Basic.h"
#include "Sample_Phong.h"
#include "Sample_IBL.h"
#include "Sample_Mipmap.h"
#include "Sample_PBR.h"
#include "Sample_Night.h"

using namespace std;

int main()
{
    unique_ptr<Engine> sample = make_unique<Sample_Night>();

	if (!Engine::Create(move(sample)))
	{
        cout << "Engine already created." << endl;
        return -1;
	}

	Engine* engine = Engine::Get();
	if (!engine->Initialize())
	{
		cout << "Initialization failed." << endl;
		return -1;
	}

	return engine->Run();
}