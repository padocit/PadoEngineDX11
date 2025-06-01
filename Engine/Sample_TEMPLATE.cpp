#include "Sample_TEMPLATE.h"

Sample_TEMPLATE::Sample_TEMPLATE() : Engine() {}

bool Sample_TEMPLATE::InitLevel()
{
    // Level »ý¼º


    Engine::InitLevel();
    return true;
}

void Sample_TEMPLATE::UpdateGUI()
{
    Engine::UpdateGUI();
}

void Sample_TEMPLATE::Update(float dt)
{
    Engine::Update(dt);
}

void Sample_TEMPLATE::Render()
{
    Engine::Render();
}
