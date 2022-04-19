#pragma
#include <AgarGame.hpp>
#include <vector>

class AgarGameView
{
    using GameState = std::vector<AgarGame::EntityPair>;

public:
    AgarGameView();
    void ForceGameView(GameState&& new_state);

    void UpdateGameView(double dt);

    void SetPlayerInput(std::string player_u_id, Vec2 input);

    GameState* GetGameView();
private:

    GameEntity* GetEntityById(std::string id);
    GameEntity* GetDisplayEntityById(std::string id);
    GameState display_state_{};
    GameState last_state_{};
};