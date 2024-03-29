#pragma once
#include <GameEntity.hpp>
#include <unordered_map>
#include <vector>
#include <deque>

static constexpr int ui_len = 15;
class AgarGame
{
public:
    struct EntityPair
            {
                char uid[ui_len+1];
                GameEntity entity;
            };

    AgarGame(int width, int height);
    ~AgarGame();

    void StartGame();

    void AddPlayerInputs(std::string player_u_id);

    std::string CreatePlayer();

    void Step(float dt);
    std::vector<EntityPair> GetGameState();

private:
    GameEntity CreateRandomEntity();

    void FreeKilled();

    void ApplyInputs();
    void UpdatePositions(float dt);
    void ClipPosition(Vec2& pos);

    void CheckCollision();
    void ResolveEatCondition(GameEntity& a, GameEntity& b);
    void Eat(GameEntity& a, GameEntity& b);
    void MoveRandomPlace(GameEntity& a);

    int GetRandomInt(int start, int end);
    Vec2 GetRandomCoordinate();

    std::string GetUniqueID();

    bool is_running_;
    int width_;
    int height_;

    static constexpr int max_entity_amt { 10 };
    std::unordered_map<GameEntity*, std::string> uid_by_ent_{};
    std::unordered_map<std::string, GameEntity*> ent_by_uid_{};
    std::deque<GameEntity> entities_{};
};