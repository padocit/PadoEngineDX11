#include "Level.h"
#include "Actor.h"

Level::Level()
{
}

Level::~Level()
{
}

void Level::AddActor(std::shared_ptr<Actor> newActor)
{
    actors.push_back(newActor);
}

void Level::Render()
{
    // Actor->Render();
}