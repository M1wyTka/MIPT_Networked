#include <AgarGame.hpp>
#include <random>
#include <vector>
#include <cstring>

AgarGame::AgarGame(int width, int height) :
                        width_(width),
                        height_(height),
                        is_running_(false)
{

}

AgarGame::~AgarGame()
{

}

void AgarGame::StartGame()
{
    entities_.resize(100);
    for(int i = 0; i < max_entity_amt; i++)
    {
        GameEntity ent = CreateRandomEntity();
        std::string uid = GetUniqueID();

        entities_[i] = ent;
        uid_by_ent_.emplace(&entities_[i], uid);
        ent_by_uid_.emplace(uid, &entities_[i]);
    }
}

std::string AgarGame::GetUniqueID()
{
    const char* v = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string rand_str;
    for (int i = 0; i < 16; i++) {
        rand_str += v[GetRandomInt(0, 35)];
    }
    return rand_str;
}

void AgarGame::AddPlayerInputs(std::string player_u_id)
{
    ent_by_uid_[player_u_id]->vel.x = 1;
    ent_by_uid_[player_u_id]->vel.y = 1;
}

GameEntity AgarGame::CreateRandomEntity()
{
    GameEntity ent = {
            .target = GetRandomCoordinate(),
            .size = static_cast<uint32_t>(GetRandomInt(2, 5)),
            .is_player = false,
            .dead = false
    };
    MoveRandomPlace(ent);
    return ent;
}

std::string AgarGame::CreatePlayer()
{
    if(!is_running_)
    {
        is_running_ = true;
        StartGame();
    }

    GameEntity ent = {
            .pos = GetRandomCoordinate(),
            .target = {0, 0},
            .vel = {0, 0},
            .size = 4,
            .is_player = true,
            .dead = false
    };

    std::string uid = GetUniqueID();

    entities_.push_back(ent);
    uid_by_ent_.emplace(&entities_.back(), uid);
    ent_by_uid_.emplace(uid, &entities_.back());
    return uid;
}

void AgarGame::Step(float dt)
{
    if(!is_running_)
        return;

    FreeKilled();

    ApplyInputs();

    UpdatePositions(dt);

    CheckCollision();
}

void AgarGame::FreeKilled()
{
    auto it = entities_.begin();
    while (it != entities_.end()) {
        // Remove elements while iterating
        if (it->dead) {
            ent_by_uid_.erase(uid_by_ent_[&(*it)]);
            uid_by_ent_.erase(&(*it));

            it = entities_.erase(it);
        } else
            it++;
    }
}

void AgarGame::ApplyInputs()
{

}

void AgarGame::UpdatePositions(float dt)
{
    for(auto& entity : entities_)
    {
        entity.pos.x += entity.vel.x * dt;
        entity.pos.y += entity.vel.y * dt;
        //ClipPosition(entity.pos);

        Vec2 diff = {
                .x = entity.pos.x - entity.target.x,
                .y = entity.pos.y - entity.target.y
        };
        if(SqLen(diff) < 0.01)
        {
            entity.target = GetRandomCoordinate();
            entity.vel.x = (entity.target.x - entity.pos.x) / 10;
            entity.vel.y = (entity.target.y - entity.pos.y) / 10;
        }
    }
}

void AgarGame::ClipPosition(Vec2 &pos)
{
    if(pos.x < 0)
        pos.x = width_ - pos.x;
    if(pos.x >= width_)
        pos.x -= width_;

    if(pos.y < 0)
        pos.y = height_ - pos.y;
    if(pos.y >= height_)
        pos.y -= height_;
}

std::vector<AgarGame::EntityPair> AgarGame::GetGameState()
{
    std::vector<AgarGame::EntityPair> out;

    for(auto& [u_id, ent] : ent_by_uid_)
    {
        AgarGame::EntityPair pair = {.entity = *ent};
        strcpy( pair.uid, u_id.c_str());
        out.push_back(pair);
    }
    return out;
}

void AgarGame::CheckCollision()
{
    for(int i = 0; i < entities_.size(); i++)
    {
        for(int j = i+1; j < entities_.size(); j++)
        {
            Vec2 diff = {
                    .x =  entities_[i].pos.x - entities_[j].pos.x,
                    .y = entities_[i].pos.y - entities_[j].pos.y
            };
            if(SqLen(diff) < entities_[i].size + entities_[j].size)
                ResolveEatCondition(entities_[i], entities_[j]);
        }
    }
}

void AgarGame::ResolveEatCondition(GameEntity& a, GameEntity& b)
{
    if(a.size < b.size){
        Eat(b, a);

        if(!a.dead)
            MoveRandomPlace(a);
        return;
    }

    Eat(a, b);
    if(!b.dead)
        MoveRandomPlace(b);
}

void AgarGame::Eat(GameEntity& a, GameEntity& b)
{
    if(b.size == 1)
    {
        a.size += b.size;
        b.dead = true;
    }
    else
    {
        b.size /= 2;
        a.size += b.size;
    }
}

void AgarGame::MoveRandomPlace(GameEntity &a)
{
    a.pos = GetRandomCoordinate();
    if(a.is_player)
        return;

    a.vel.x = (a.target.x - a.pos.x) / 10;
    a.vel.y = (a.target.y - a.pos.y) / 10;
}

int AgarGame::GetRandomInt(int start, int end)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(start, end);
    return distrib(gen);
}

Vec2 AgarGame::GetRandomCoordinate()
{
    int line_pos = GetRandomInt(0, width_*height_);
    int x = line_pos % height_;
    int y = line_pos / height_;
    return { static_cast<float>(x), static_cast<float>(y) };
}