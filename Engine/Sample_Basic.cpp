#include "Sample_Basic.h"
#include "GeometryGenerator.h"
#include "Actor.h"

using namespace std;

Sample_Basic::Sample_Basic(int Width, int Height) 
    : Engine(Width, Height)
{
}

bool Sample_Basic::InitLevel()
{
    Engine::camera.Reset(Vector3(0.0f, 0.0f, -2.5f), 0.0f, 0.0f);
    Engine::InitLevel();
      
    // ¹Ú½º
    {   
        MeshData boxData = GeometryGenerator::MakeBox(0.5f);
        boxData.albedoTextureFilename = "../Assets/Textures/box.png";

        auto newActor = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{boxData});
        newActor->materialConsts.GetCpu().albedoFactor = Vector3(1.0f);
        newActor->UpdateConstantBuffers(renderer.GetDevice(),
                                        renderer.GetContext());
        newActor->name = "Box";
        level.AddActor(newActor);
    }
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
