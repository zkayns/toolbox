#include <Geode/Geode.hpp>
using namespace geode::prelude;
struct SavedIconsEntry {
    int cube;
    int ship;
    int jetpack;
    int ball;
    int ufo;
    int wave;
    int robot;
    int spider;
    int swing;
    int color1;
    int color2;
    int colorGlow;
    bool glowEnabled;
    int streak;
    int fire;
    int death;
    static SavedIconsEntry fromCurrent() {
        return SavedIconsEntry{
            .cube=GameManager::get()->m_playerFrame.value(),
            .ship=GameManager::get()->m_playerShip.value(),
            .jetpack=GameManager::get()->m_playerJetpack.value(),
            .ball=GameManager::get()->m_playerBall.value(),
            .ufo=GameManager::get()->m_playerBird.value(),
            .wave=GameManager::get()->m_playerDart.value(),
            .robot=GameManager::get()->m_playerRobot.value(),
            .spider=GameManager::get()->m_playerSpider.value(),
            .swing=GameManager::get()->m_playerSwing.value(),
            .color1=GameManager::get()->m_playerColor.value(),
            .color2=GameManager::get()->m_playerColor2.value(),
            .colorGlow=GameManager::get()->m_playerGlowColor.value(),
            .glowEnabled=GameManager::get()->m_playerGlow,
            .streak=GameManager::get()->m_playerStreak.value(),
            .fire=GameManager::get()->m_playerShipFire.value(),
            .death=GameManager::get()->m_playerDeathEffect.value(),
        };
    };
    void load() {
        GameManager::get()->setPlayerFrame(this->cube);
        GameManager::get()->setPlayerShip(this->ship);
        GameManager::get()->setPlayerJetpack(this->jetpack);
        GameManager::get()->setPlayerBall(this->ball);
        GameManager::get()->setPlayerBird(this->ufo);
        GameManager::get()->setPlayerDart(this->wave);
        GameManager::get()->setPlayerRobot(this->robot);
        GameManager::get()->setPlayerSpider(this->spider);
        GameManager::get()->setPlayerSwing(this->swing);
        GameManager::get()->setPlayerColor(this->color1);
        GameManager::get()->setPlayerColor2(this->color2);
        GameManager::get()->setPlayerColor3(this->colorGlow);
        GameManager::get()->setPlayerGlow(this->glowEnabled);
        GameManager::get()->setPlayerShipStreak(this->fire);
        GameManager::get()->setPlayerDeathEffect(this->death);
   };
};
template<>
struct matjson::Serialize<SavedIconsEntry> {
    static Result<SavedIconsEntry> fromJson(matjson::Value const& value) {
        GEODE_UNWRAP_INTO(int cube, value["cube"].asInt());
        GEODE_UNWRAP_INTO(int ship, value["ship"].asInt());
        GEODE_UNWRAP_INTO(int jetpack, value["jetpack"].asInt());
        GEODE_UNWRAP_INTO(int ball, value["ball"].asInt());
        GEODE_UNWRAP_INTO(int ufo, value["ufo"].asInt());
        GEODE_UNWRAP_INTO(int wave, value["wave"].asInt());
        GEODE_UNWRAP_INTO(int robot, value["robot"].asInt());
        GEODE_UNWRAP_INTO(int spider, value["spider"].asInt());
        GEODE_UNWRAP_INTO(int swing, value["swing"].asInt());
        GEODE_UNWRAP_INTO(int color1, value["color1"].asInt());
        GEODE_UNWRAP_INTO(int color2, value["color2"].asInt());
        GEODE_UNWRAP_INTO(int colorGlow, value["colorGlow"].asInt());
        GEODE_UNWRAP_INTO(bool glowEnabled, value["glowEnabled"].asBool());
        GEODE_UNWRAP_INTO(int fire, value["fire"].asInt());
        GEODE_UNWRAP_INTO(int death, value["death"].asInt());
        return Ok(SavedIconsEntry{
            cube, 
            ship, 
            jetpack, 
            ball, 
            ufo, 
            wave, 
            robot, 
            spider, 
            swing,
            color1,
            color2,
            colorGlow,
            glowEnabled,
            fire,
            death
        });
    };
    static matjson::Value toJson(SavedIconsEntry const& value) {
        auto obj=matjson::Value();
        obj["cube"]=value.cube;
        obj["ship"]=value.ship;
        obj["jetpack"]=value.jetpack;
        obj["ball"]=value.ball;
        obj["ufo"]=value.ufo;
        obj["wave"]=value.wave;
        obj["robot"]=value.robot;
        obj["spider"]=value.spider;
        obj["swing"]=value.swing;
        obj["color1"]=value.color1;
        obj["color2"]=value.color2;
        obj["colorGlow"]=value.colorGlow;
        obj["glowEnabled"]=value.glowEnabled;
        obj["fire"]=value.fire;
        obj["death"]=value.death;
        return obj;
    };
};