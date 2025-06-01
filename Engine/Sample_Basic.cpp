#include "Sample_Basic.h"
#include "GeometryGenerator.h"

Sample_Basic::Sample_Basic()
{
}

bool Sample_Basic::InitLevel()
{
    // MeshData (vertices, indices) 持失
    auto boxData = GeometryGenerator::MakeBox(0.5f);

    // Mesh (vertices, indices buffer) 持失
    renderer.CreateMesh(boxData, box);


    Engine::InitLevel();
    return true;
}

void Sample_Basic::UpdateGUI()
{
    Engine::UpdateGUI();
}

void Sample_Basic::Update(float dt)
{
    Engine::Update(dt);
}

void Sample_Basic::Render()
{
    Engine::Render();
}
