#pragma once
#include <Core/engine.h>


class Game : public GalacticEngine::Core::Engine {
public:
    using Engine::Engine; // Inherit Engine constructor

protected:
    //innit game
    void OnStart() override;
    //update game
    void OnUpdate() override;
};
