#pragma once

#include <vector>
#include <array>
#include <memory>
#include <string>
#include <optional>
#include <unordered_set>

#include <extlib/cereal/archives/binary.hpp>
#include <extlib/cereal/types/vector.hpp>
#include <extlib/cereal/types/memory.hpp>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Sounds.hpp"
#include "Core/Camera.hpp"

#include "Player/Cursor.hpp"

#include "BossEntity.hpp"
#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Entity/Boss/BossSandSerpent.hpp"
#include "Entity/Boss/BossGlacialBrute.hpp"

class Player;
class Game;
class ChunkManager;

class BossManager
{
public:
    BossManager() = default;
    BossManager(const BossManager& bossManager);
    BossManager& operator=(const BossManager& bossManager);

    bool createBoss(const std::string& name, pl::Vector2f playerPosition, Game& game);

    void update(Game& game, ProjectileManager& projectileManager, ChunkManager& chunkManager, std::vector<Player*>& players, float dt, float gameTime);

    void testHitRectCollision(const std::vector<HitRect>& hitRects, int worldSize);

    // void handleWorldWrap(pl::Vector2f positionDelta);

    void stopBossMusic();

    bool isPlayingMusicBossMusic();

    void clearBosses();

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch);

    void drawStatsAtCursor(pl::RenderTarget& window, const Camera& camera, pl::Vector2f mouseScreenPos, int worldSize);

    void getBossWorldObjects(std::vector<WorldObject*>& worldObjects);

    std::vector<std::unique_ptr<BossEntity>>& getBosses();

    int getBossCount() const;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(bosses);
    }

private:
    std::vector<std::unique_ptr<BossEntity>> bosses;
    std::unordered_set<std::string> bossAliveNames;

    static constexpr float STATS_DRAW_OFFSET_X = 24;
    static constexpr float STATS_DRAW_OFFSET_Y = 24;
    static constexpr int STATS_DRAW_SIZE = 24;
    static constexpr int STATS_DRAW_PADDING = 3;
    static constexpr int STATS_DRAW_OUTLINE_THICKNESS = 2;

};