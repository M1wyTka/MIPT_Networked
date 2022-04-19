#include <AgarGameView.hpp>

AgarGameView::AgarGameView()
{

}

void AgarGameView::SetPlayerInput(std::string player_u_id, Vec2 input) {

    GameEntity* player = GetDisplayEntityById(player_u_id);
    if (player)
        player->vel = input;
}

void AgarGameView::ForceGameView(AgarGameView::GameState&& new_state)
{
    display_state_.clear();
    for(const auto& entity : new_state)
    {
        AgarGame::EntityPair display_pair = entity;
        GameEntity* last = GetEntityById(entity.uid);
        if(last && !display_pair.entity.is_player && !display_pair.entity.dead)
        {
            Vec2 new_pos {.x = (last->pos.x + display_pair.entity.pos.x)/2,
                          .y = (last->pos.y + display_pair.entity.pos.y)/2};
            display_pair.entity.pos = new_pos;
        }
        display_state_.push_back(display_pair);
    }
    last_state_ = new_state;
}

void AgarGameView::UpdateGameView(double dt)
{
    for(auto& pair : display_state_)
    {
        pair.entity.pos.x += pair.entity.vel.x * dt;
        pair.entity.pos.y += pair.entity.vel.y * dt;
    }
}

GameEntity* AgarGameView::GetEntityById(std::string id)
{
    for(int i = 0; i < last_state_.size(); i++)
    {
        if(last_state_[i].uid == id)
            return &last_state_[i].entity;
    }
    return nullptr;
}

GameEntity* AgarGameView::GetDisplayEntityById(std::string id)
{
    for(int i = 0; i < display_state_.size(); i++)
    {
        if(std::string(display_state_[i].uid, id.length()) == id)
            return &display_state_[i].entity;
    }
    return nullptr;
}

AgarGameView::GameState* AgarGameView::GetGameView()
{
  return &display_state_;
}
