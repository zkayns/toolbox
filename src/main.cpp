// .RGB = RoGue Bot
// .TBG = ToolBox Garage
#include <Geode/Geode.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/GJGameLevel.hpp>
#include <imgui-cocos.hpp>
#include "../include/slc/slc.hpp"
using namespace geode::prelude;
struct Shared {
    static uint64_t getTick() {
        if (GJBaseGameLayer::get()==nullptr) return 0;
        return GJBaseGameLayer::get()->m_gameState.m_currentProgress/2;
    };
};
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
            .cube=GameManager::get()->getPlayerFrame(),
            .ship=GameManager::get()->getPlayerShip(),
            .jetpack=GameManager::get()->getPlayerJetpack(),
            .ball=GameManager::get()->getPlayerBall(),
            .ufo=GameManager::get()->getPlayerBird(),
            .wave=GameManager::get()->getPlayerDart(),
            .robot=GameManager::get()->getPlayerRobot(),
            .spider=GameManager::get()->getPlayerSpider(),
            .swing=GameManager::get()->getPlayerSwing(),
            .color1=GameManager::get()->getPlayerColor(),
            .color2=GameManager::get()->getPlayerColor2(),
            .colorGlow=GameManager::get()->getPlayerGlowColor(),
            .glowEnabled=GameManager::get()->getPlayerGlow(),
            .streak=GameManager::get()->getPlayerStreak(),
            .fire=GameManager::get()->getPlayerShipFire(),
            .death=GameManager::get()->getPlayerDeathEffect(),
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
// BEGIN ROGUE
struct RogueInput {
    bool player;
    bool press;
    PlayerButton button;
    uint64_t tick;
    bool remove;
    static RogueInput fromString(std::string str) {
        if (str.size()<3) return RogueInput{};
        RogueInput ret;
        if (str[0]=='0') ret.button=PlayerButton::Jump;
        else if (str[0]=='1') ret.button=PlayerButton::Left;
        else if (str[0]=='2') ret.button=PlayerButton::Right;
        ret.press=(str[1]=='1');
        ret.player=(str[2]=='1');
        ret.tick=std::stoull(str.substr(3));
        return ret;
    };
    std::string toString() {
        std::string ret;
        if (button==PlayerButton::Jump) ret+="0";
        else if (button==PlayerButton::Left) ret+="1";
        else if (button==PlayerButton::Right) ret+="2";
        ret+=std::to_string(press);
        ret+=std::to_string(player);
        ret+=std::to_string(tick);
        return ret;
    };
};
struct RogueMacro {
    char name[128]="";
    double tps=240;
    int seed=0;
    std::vector<RogueInput> inputs;
    void cleanInputs() {
        std::erase_if(inputs, [](RogueInput input){
            if (input.tick>Shared::getTick()) return true;
            if (input.tick<1) return true;
            if (!GJBaseGameLayer::get()->m_isPlatformer&&input.button!=PlayerButton::Jump) return true;
            return input.remove;
        });
    };
    std::string toString() {
        std::string ret=std::to_string(tps)+"|"+std::to_string(seed);
        for (RogueInput input:inputs) ret+="|"+input.toString();
        return ret;
    };
};
struct Rogue {
    RogueMacro macro;
    int state;
    uint64_t lastPress;
    uint64_t lastRelease;
    int format;
    std::filesystem::path macroSavePath(std::string fileName, std::string format) { return Mod::get()->getSaveDir()/"rogue"/"macros"/(fileName+"."+format); };
    void replayFrame() { for (const auto& input:macro.inputs) if (input.tick==Shared::getTick()&&(!input.player||GJBaseGameLayer::get()->m_level->m_twoPlayerMode)) GJBaseGameLayer::get()->handleButton(input.press, static_cast<int>(input.button), !input.player); };
    void save() { 
        switch (format) {
            case 0: { // RGB
                geode::utils::file::writeString(macroSavePath(std::string(macro.name), "rgb"), macro.toString()); 
                break;
            };
            case 1: { // SLC
                slc::ActionAtom actions;
                for (const auto& input:macro.inputs) actions.addAction(input.tick-1, (input.button==PlayerButton::Jump)?slc::Action::ActionType::Jump:((input.button==PlayerButton::Left)?slc::Action::ActionType::Left:slc::Action::ActionType::Right), input.press, input.player);
                slc::Replay<> replay;
                replay.m_meta.m_tps=macro.tps;
                replay.m_meta.m_seed=macro.seed;
                replay.m_atoms.add(std::move(actions));
                std::ofstream file(macroSavePath(std::string(macro.name), "slc"), std::ios::binary);
                replay.write(file);
                break;
            };
        };
    };
    void load() {
        switch (format) {
            case 0: { // RGB
                char macroName[128];
                strcpy(macroName, macro.name);
                auto result=geode::utils::file::readString(macroSavePath(std::string(macroName), "rgb")).ok();
                macro=RogueMacro{};
                strcpy(macro.name, macroName);
                if (!result.has_value()) break;
                auto split=geode::utils::string::splitView(result.value(), "|");
                if (split.size()<3) break;
                macro.tps=std::stod(std::string(split[0]));
                macro.seed=std::stoi(std::string(split[1]));
                std::vector<std::string_view> inputStrings(split.begin()+2, split.end());
                for (const auto& s:inputStrings) macro.inputs.push_back(RogueInput::fromString(std::string(s)));
                break;
            };
            case 1: { // SLC
                std::ifstream file(macroSavePath(std::string(macro.name), "slc"), std::ios::binary);
                auto sr=slc::v3::Replay<>::read(file);
                if (sr.has_value()) {
                    macro.inputs.clear();
                    auto& atoms=sr.value().m_atoms.m_atoms;
                    auto actions=std::get<slc::ActionAtom>(*(std::find_if(atoms.begin(), atoms.end(), [](auto& v) { return std::visit([](auto& at) { return at.id == slc::v3::AtomId::Action; }, v); }))).m_actions;
                    for (int i=0; i<actions.size(); ++i) macro.inputs.push_back(RogueInput{
                        .player=actions.at(i).m_player2,
                        .press=actions.at(i).m_holding,
                        .button=(actions.at(i).m_type==slc::Action::ActionType::Jump)?PlayerButton::Jump:((actions.at(i).m_type==slc::Action::ActionType::Left)?PlayerButton::Left:PlayerButton::Right),
                        .tick=actions.at(i).m_frame+1
                    });
                    macro.seed=sr.value().m_meta.m_seed;
                    macro.tps=sr.value().m_meta.m_tps;
                };
                break;
            };
        };
        std::erase_if(macro.inputs, [](RogueInput input){
            return input.tick<1;
        });
    };
};
Rogue rogue;
class $modify(GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (rogue.state==1) rogue.macro.inputs.push_back(RogueInput{
            .player=!isPlayer1,
            .press=down,
            .button=static_cast<PlayerButton>(button),
            .tick=Shared::getTick()
        });
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    };
};
class $modify(PlayerObject) {
    void loadFromCheckpoint(PlayerCheckpoint* object) {
        PlayerObject::loadFromCheckpoint(object);
        PlayerObject::releaseAllButtons();
        if (rogue.state==1) rogue.macro.cleanInputs();
    };
    void update(float dt) {
        if (!isPlayer1()) {
            PlayerObject::update(dt);
            return;
        };
        if (rogue.state==2) rogue.replayFrame();
        else if (rogue.state==1) {
            rogue.macro.seed=GJBaseGameLayer::get()->m_randomSeed;
            rogue.macro.cleanInputs();
        };
        PlayerObject::update(dt);
    };
};
class $modify(PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        if (rogue.state==1) rogue.macro.cleanInputs();      
    };
};
// END ROGUE
struct Toolbox {
    bool styleEditor=false;
    bool uiOpen=false;
    int gamemodeRadio1;
    int gamemodeRadio2;
    const std::string funnies[12]={
        "Approved by SEA1997 and RobTop",
        "Watch Schaf Central",
        "I hope that kid starts lowballing on Marketplace",
        "Sign my petition to save GD",
        "All about the Mets, baby",
        "Send me cringe one more time, kid",
        "It's Yeezy over everything",
        "Bro shut up",
        "El Gordo owns you kid",
        "You are a leg",
        "I'm for the two way streets",
        "We sell relatable gold here sir"
    };
    static inline const std::unordered_set<uint16_t> layoutObjs={
        0,
        1,
        2,
        3,
        4,
        6,
        7,
        8,
        9,
        10,
        11,
        12,
        13,
        34,
        35,
        36,
        38,
        39,
        40,
        42,
        43,
        44,
        45,
        46,
        47,
        61,
        62,
        63,
        64,
        65,
        66,
        67,
        68,
        69,
        70,
        71,
        72,
        74,
        75,
        76,
        77,
        78,
        79,
        81,
        82,
        83,
        84,
        88,
        89,
        90,
        91,
        92,
        93,
        94,
        95,
        96,
        98,
        99,
        101,
        103,
        111,
        116,
        117,
        118,
        119,
        121,
        122,
        135,
        140,
        141,
        142, 
        143,
        144,
        145,
        146,
        147,
        160,
        161,
        162,
        163,
        165,
        166,
        167,
        168,
        169,
        170,
        171,
        172,
        173,
        174,
        175,
        176,
        177,
        178,
        179,
        183,
        184,
        185,
        186,
        187,
        188,
        192,
        194,
        195,
        196,
        197,
        200,
        201,
        202,
        203,
        204,
        205,
        206,
        207,
        208,
        209,
        210,
        212,
        213,
        215,
        216,
        217,
        218,
        219,
        220,
        243,
        244,
        247,
        248,
        249,
        250,
        252,
        253,
        254,
        255,
        256,
        257,
        258,
        260,
        261,
        263,
        264,
        265,
        267,
        268,
        269,
        270,
        271,
        272,
        274,
        275,
        286,
        287,
        289,
        291,
        294,
        295,
        299,
        301,
        305,
        307,
        309,
        311,
        315,
        317,
        321,
        323,
        326,
        327,
        328,
        329,
        331,
        333,
        337,
        339,
        343,
        345,
        349,
        351,
        353,
        355,
        363,
        364,
        365,
        366,
        367,
        368,
        369,
        370,
        371,
        372,
        392,
        397,
        398,
        399,
        421,
        422,
        446,
        447,
        458,
        459,
        467,
        468,
        469,
        470,
        471,
        475,
        483,
        484,
        492,
        493,
        651,
        652,
        660,
        661,
        662,
        663,
        664,
        665,
        666,
        667,
        673,
        674,
        675,
        676,
        677,
        678,
        679,
        680,
        709,
        710,
        711,
        712,
        726,
        727,
        728,
        729,
        740,
        741,
        742,
        745,
        747,
        749,
        886,
        887,
        918,
        919,
        989,
        1022,
        1155,
        1156,
        1157,
        1202,
        1203,
        1204,
        1208,
        1209,
        1210,
        1220,
        1221,
        1222,
        1226,
        1227,
        1260,
        1262,
        1264,
        1275,
        1327,
        1328,
        1329,
        1330,
        1331,
        1332,
        1333,
        1334,
        1338,
        1339,
        1340,
        1341,
        1342,
        1343,
        1344,
        1345,
        1561,
        1562,
        1563,
        1564,
        1565,
        1566,
        1567,
        1568,
        1569,
        1582,
        1583,
        1584,
        1587,
        1589,
        1594,
        1598,
        1614,
        1619,
        1620,
        1701,
        1702,
        1703,
        1704,
        1705,
        1706,
        1707,
        1708,
        1709,
        1710,
        1711,
        1712,
        1713,
        1714,
        1715,
        1716,
        1717,
        1718,
        1719,
        1720,
        1721,
        1722,
        1723,
        1724,
        1725,
        1726,
        1727,
        1728,
        1729,
        1730,
        1731,
        1732,
        1733,
        1734,
        1735,
        1736,
        1743,
        1744,
        1745,
        1746,
        1747,
        1748,
        1749,
        1750,
        1751,
        1816,
        1903,
        1904,
        1905,
        1906,
        1907,
        1910,
        1911,
        1933,
        2012,
        2064,
        2902,
        2926,
        3004,
        3005,
        3027,
        3034,
        3035,
        3036,
        3037,
        3601,
        3610,
        3611,
        4401,
        4402,
        4403,
        4404,
        4405,
        4406,
        4407,
        4408,
        4409,
        4410,
        4411,
        4412,
        4413,
        4414,
        4415,
        4416,
        4417,
        4418,
        4419,
        4420,
        4421,
        4422,
        4423,
        4424,
        4425,
        4426,
        4427,
        4428,
        4429,
        4430,
        4431,
        4432,
        4433,
        4434,
        4435,
        4436,
        4437,
        4438,
        4439,
        4440,
        4441,
        4442,
        4443,
        4444,
        4445,
        4446,
        4447,
        4448,
        4449,
        4450,
        4451,
        4452,
        4453,
        4454,
        4455,
        4456,
        4457,
        4458,
        4459,
        4460,
        4461,
        4462,
        4463,
        4464,
        4465,
        4466,
        4467,
        4468,
        4469,
        4470,
        4471,
        4472,
        4473,
        4474,
        4475,
        4476,
        4477,
        4478,
        4479,
        4480,
        4481,
        4482,
        4483,
        4484,
        4485,
        4486,
        4487,
        4488,
        4489,
        4490,
        4491,
        4492,
        4493,
        4494,
        4495,
        4496,
        4497,
        4498,
        4499,
        4500,
        4501,
        4502,
        4503,
        4504,
        4505,
        4506,
        4507,
        4508,
        4509,
        4510,
        4511,
        4512,
        4513,
        4514,
        4515,
        4516,
        4517,
        4518,
        4519,
        4520,
        4521,
        4522,
        4523,
        4524,
        4525,
        4526,
        4527,
        4528,
        4529,
        4530,
        4531,
        4532,
        4533,
        4534,
        4535,
        4536,
        4537,
        4538,
        4539
    };
    static inline const std::unordered_set<uint16_t> cosmeticObjs={
        16, 
        17, 
        29,
        30,
        32,
        33,
        104,
        105,
        221,
        717, 
        718,
        743, 
        744, 
        899, 
        900, 
        915, 
        1006,
        1007,
        1520,
        1612,
        1613,
        1818,
        1819,
        2903,
        2999,
        3009,
        3010,
        3014,
        3015,
        3020,
        3021,
        3029,
        3030,
        3031,
        3606,
        3608,
        3612
    };
    std::string currentFunny;
    bool individualRotate;
    float speedhack;
    bool speedhackEnabled;
    bool autoKill;
    float autoKillPercentage;
    bool customWaveTrail;
    bool noclip;
    bool noCollide;
    bool noObjectGlow;
    bool noRespawnFlash;
    bool noWaveTrail;
    bool iconHack;
    float fontScale;
    bool noCheckpointDelay;
    float waveTrailSize;
    cocos2d::ccColor3B waveTrailColor;
    float waveTrailColorPicker[3];
    bool useWaveTrailColor;
    bool oneTimeCheatThisAttempt;
    double realTpsBypass;
    double tpsBypass;
    bool tpsBypassEnabled;
    bool maintainGravity;
    bool showLayout;
    cocos2d::ccColor3B layoutG1Color;
    float layoutG1ColorPicker[3];
    cocos2d::ccColor3B layoutG2Color;
    float layoutG2ColorPicker[3];
    cocos2d::ccColor3B layoutLineColor;
    float layoutLineColorPicker[3];
    cocos2d::ccColor3B layoutMGColor;
    float layoutMGColorPicker[3];
    cocos2d::ccColor3B layoutMG2Color;
    float layoutMG2ColorPicker[3];
    cocos2d::ccColor3B layoutBGColor;
    float layoutBGColorPicker[3];
    bool verifyHack;
    bool isCheating() {
        return (
            noclip||
            noCollide||
            speedhackEnabled||
            rogue.state==2||
            oneTimeCheatThisAttempt||
            maintainGravity||
            showLayout
        );
    };
    GJGameLevel* lastLevel=nullptr;
    char iconSaveName[64]="icons";
    static std::string realPassword(geode::SeedValueRS password) {
        if (password.value()==0) return "Not Copyable";
        if (password.value()==1) return "Free Copy";
        return std::to_string(password.value()).substr(1);
    };
    void switchGamemodeFromMenu(PlayerObject* player, int mode) {
        oneTimeCheatThisAttempt=true;
        Toolbox::switchGamemode(player, mode);
    };
    bool iconKitExists(const std::string fileName) {
        return std::filesystem::exists(iconSavePath(fileName));
    };
    std::filesystem::path iconSavePath(const std::string fileName) { return Toolbox::getMod()->getSaveDir()/"iconKits"/(fileName+".tbg"); };
    static int getGamemode(const PlayerObject* player) {
        if (player->m_isShip) return 1;
        if (player->m_isBall) return 2;
        if (player->m_isBird) return 3;
        if (player->m_isDart) return 4;
        if (player->m_isRobot) return 5;
        if (player->m_isSpider) return 6;
        if (player->m_isSwing) return 7;
        return 0;
    };
    static void switchGamemode(PlayerObject* player, int mode) {
        EffectGameObject* eff=EffectGameObject::create("");
        eff->m_cameraIsFreeMode=true;
        player->toggleFlyMode(mode==1, true);
        player->toggleRollMode(mode==2, true);
        player->toggleBirdMode(mode==3, true);
        player->toggleDartMode(mode==4, true);
        player->toggleRobotMode(mode==5, true);
        player->toggleSpiderMode(mode==6, true);
        player->toggleSwingMode(mode==7, true);
        GJBaseGameLayer::get()->updateCameraMode(eff, true);
    };
    void loadIcons() {
        if (!iconKitExists(iconSaveName)) return;
        geode::utils::file::readFromJson<SavedIconsEntry>(iconSavePath(iconSaveName)).ok()->load();
    };
    void saveIcons() {
        geode::utils::file::writeToJson<SavedIconsEntry>(iconSavePath(iconSaveName), SavedIconsEntry::fromCurrent());
    };
    static GLubyte toGLubyte(float f) { return static_cast<GLubyte>(std::clamp<float>(std::round(f*255.f), 0.f, 255.f)); };
    static Mod* getMod() { return Mod::get(); };
    static FMODAudioEngine* getAudioEngine() { return FMODAudioEngine::get(); };
    void load() {
        autoKill=Toolbox::getMod()->getSavedValue<bool>("autoKill", false);
        autoKillPercentage=Toolbox::getMod()->getSavedValue<float>("autoKillPercentage", 50.f);
        customWaveTrail=Toolbox::getMod()->getSavedValue<bool>("customWaveTrail", false);
        fontScale=Toolbox::getMod()->getSavedValue<float>("fontScale", 1.f);
        iconHack=Toolbox::getMod()->getSavedValue<bool>("iconHack", false);
        noclip=Toolbox::getMod()->getSavedValue<bool>("noclip", false);
        noCollide=Toolbox::getMod()->getSavedValue<bool>("noCollide", false);
        noObjectGlow=Toolbox::getMod()->getSavedValue<bool>("noObjectGlow", false);
        noRespawnFlash=Toolbox::getMod()->getSavedValue<bool>("noRespawnFlash", false);
        noWaveTrail=Toolbox::getMod()->getSavedValue<bool>("noWaveTrail", false);
        waveTrailSize=Toolbox::getMod()->getSavedValue<float>("waveTrailSize", 1.f);
        speedhack=Toolbox::getMod()->getSavedValue<float>("speedhack", 1.f);
        speedhackEnabled=Toolbox::getMod()->getSavedValue<bool>("speedhackEnabled", false);
        individualRotate=Toolbox::getMod()->getSavedValue<bool>("individualRotate", false);
        waveTrailColor=Toolbox::getMod()->getSavedValue<cocos2d::ccColor3B>("waveTrailColor", cocos2d::ccColor3B{255, 0, 0});
        Toolbox::loadColorPicker(&waveTrailColor, waveTrailColorPicker);
        useWaveTrailColor=Toolbox::getMod()->getSavedValue<bool>("useWaveTrailColor", false);
        noCheckpointDelay=Toolbox::getMod()->getSavedValue<bool>("noCheckpointDelay", false);
        tpsBypass=Toolbox::getMod()->getSavedValue<double>("tpsBypass", 240.0);
        tpsBypassEnabled=Toolbox::getMod()->getSavedValue<bool>("tpsBypassEnabled", false);
        maintainGravity=Toolbox::getMod()->getSavedValue<bool>("maintainGravity", false);
        showLayout=Toolbox::getMod()->getSavedValue<bool>("showLayout", false);
        layoutBGColor=Toolbox::getMod()->getSavedValue<cocos2d::ccColor3B>("layoutBGColor", cocos2d::ccColor3B{40, 125, 255});
        Toolbox::loadColorPicker(&layoutBGColor, layoutBGColorPicker);
        layoutG1Color=Toolbox::getMod()->getSavedValue<cocos2d::ccColor3B>("layoutG1Color", cocos2d::ccColor3B{0, 102, 255});
        Toolbox::loadColorPicker(&layoutG1Color, layoutG1ColorPicker);
        layoutG2Color=Toolbox::getMod()->getSavedValue<cocos2d::ccColor3B>("layoutG2Color", cocos2d::ccColor3B{0, 102, 255});
        Toolbox::loadColorPicker(&layoutG2Color, layoutG2ColorPicker);
        layoutLineColor=Toolbox::getMod()->getSavedValue<cocos2d::ccColor3B>("layoutLineColor", cocos2d::ccColor3B{255, 255, 255});
        Toolbox::loadColorPicker(&layoutLineColor, layoutLineColorPicker);
        layoutMGColor=Toolbox::getMod()->getSavedValue<cocos2d::ccColor3B>("layoutMGColor", cocos2d::ccColor3B{40, 125, 255});
        Toolbox::loadColorPicker(&layoutMGColor, layoutMGColorPicker);
        layoutMG2Color=Toolbox::getMod()->getSavedValue<cocos2d::ccColor3B>("layoutMG2Color", cocos2d::ccColor3B{40, 125, 255});
        Toolbox::loadColorPicker(&layoutMG2Color, layoutMG2ColorPicker);
        verifyHack=Toolbox::getMod()->getSavedValue<bool>("verifyHack", false);
    };
    void save() {
        Toolbox::getMod()->setSavedValue<bool>("autoKill", autoKill);
        Toolbox::getMod()->setSavedValue<float>("autoKillPercentage", autoKillPercentage);
        Toolbox::getMod()->setSavedValue<bool>("customWaveTrail", customWaveTrail);
        Toolbox::getMod()->setSavedValue<float>("fontScale", fontScale);
        Toolbox::getMod()->setSavedValue<bool>("iconHack", iconHack);
        Toolbox::getMod()->setSavedValue<bool>("noclip", noclip);
        Toolbox::getMod()->setSavedValue<bool>("noCollide", noCollide);
        Toolbox::getMod()->setSavedValue<bool>("noObjectGlow", noObjectGlow);
        Toolbox::getMod()->setSavedValue<bool>("noRespawnFlash", noRespawnFlash);
        Toolbox::getMod()->setSavedValue<bool>("noWaveTrail", noWaveTrail);
        Toolbox::getMod()->setSavedValue<float>("waveTrailSize", waveTrailSize);
        Toolbox::getMod()->setSavedValue<float>("speedhack", speedhack);
        Toolbox::getMod()->setSavedValue<float>("speedhackEnabled", speedhackEnabled);
        Toolbox::getMod()->setSavedValue<bool>("individualRotate", individualRotate);
        Toolbox::getMod()->setSavedValue<cocos2d::ccColor3B>("waveTrailColor", waveTrailColor);
        Toolbox::getMod()->setSavedValue<bool>("useWaveTrailColor", useWaveTrailColor);
        Toolbox::getMod()->setSavedValue<bool>("noCheckpointDelay", noCheckpointDelay);
        Toolbox::getMod()->setSavedValue<double>("tpsBypass", tpsBypass);
        Toolbox::getMod()->setSavedValue<bool>("tpsBypassEnabled", tpsBypassEnabled);
        Toolbox::getMod()->setSavedValue<bool>("maintainGravity", maintainGravity);
        Toolbox::getMod()->setSavedValue<bool>("showLayout", showLayout);
        Toolbox::getMod()->setSavedValue<cocos2d::ccColor3B>("layoutBGColor", layoutBGColor);
        Toolbox::getMod()->setSavedValue<cocos2d::ccColor3B>("layoutG1Color", layoutG1Color);
        Toolbox::getMod()->setSavedValue<cocos2d::ccColor3B>("layoutG2Color", layoutG2Color);
        Toolbox::getMod()->setSavedValue<cocos2d::ccColor3B>("layoutLineColor", layoutLineColor);
        Toolbox::getMod()->setSavedValue<cocos2d::ccColor3B>("layoutMGColor", layoutMGColor);
        Toolbox::getMod()->setSavedValue<cocos2d::ccColor3B>("layoutMG2Color", layoutMG2Color);
        Toolbox::getMod()->setSavedValue<bool>("verifyHack", verifyHack);
    };
    static std::vector<GameObject*> objectVec(cocos2d::CCArray* source) {
        auto ext=CCArrayExt<GameObject*>(source);
        return std::vector<GameObject*>(ext.begin(), ext.end());
    };
    static bool isDecoration(const GameObject* obj) { return obj->m_isNoTouch||!Toolbox::layoutObjs.contains(obj->m_objectID); };
    static bool isCosmetic(const GameObject* obj) { return Toolbox::cosmeticObjs.contains(obj->m_objectID); };
    static void updateColorPicker(float* source, cocos2d::ccColor3B* destination) {
        destination->r=Toolbox::toGLubyte(source[0]);
        destination->g=Toolbox::toGLubyte(source[1]);
        destination->b=Toolbox::toGLubyte(source[2]);
    };
    static void loadColorPicker(cocos2d::ccColor3B* source, float* destination) {
        destination[0]=static_cast<float>(source->r)/255.f;
        destination[1]=static_cast<float>(source->g)/255.f;
        destination[2]=static_cast<float>(source->b)/255.f;
    };
    void colorPickerChange() { 
        Toolbox::updateColorPicker(waveTrailColorPicker, &waveTrailColor);
        Toolbox::updateColorPicker(layoutBGColorPicker, &layoutBGColor);
        Toolbox::updateColorPicker(layoutG1ColorPicker, &layoutG1Color);
        Toolbox::updateColorPicker(layoutG2ColorPicker, &layoutG2Color);
        Toolbox::updateColorPicker(layoutMGColorPicker, &layoutMGColor);
        Toolbox::updateColorPicker(layoutMG2ColorPicker, &layoutMG2Color);
        Toolbox::updateColorPicker(layoutLineColorPicker, &layoutLineColor);
    };
};
Toolbox toolbox;
$on_mod(Loaded) {
    geode::utils::file::createDirectory(Mod::get()->getSaveDir()/"iconKits");
    geode::utils::file::createDirectory(Mod::get()->getSaveDir()/"rogue");
    geode::utils::file::createDirectory(Mod::get()->getSaveDir()/"rogue"/"macros");
	listenForKeybindSettingPresses("kb-toggle", [](Keybind const& keybind, bool down, bool repeat, double dt) {
		if (down&&!repeat) {
            toolbox.currentFunny=toolbox.funnies[rand()%size(toolbox.funnies)];
            toolbox.uiOpen=!toolbox.uiOpen;
        };
	});
    toolbox.load();
    ImGuiCocos::get().setup();
    ImGuiCocos::get().draw([]{
        if (toolbox.uiOpen) {
            ImGui::GetIO().FontGlobalScale=toolbox.fontScale;
            if (toolbox.styleEditor) ImGui::ShowStyleEditor();
            ImGui::Begin("Toolbox", &toolbox.uiOpen);
            if (ImGui::TreeNode("Toolbox Settings")) {
                ImGui::PushItemWidth(200.f); 
                ImGui::InputFloat("Font Scale", &toolbox.fontScale);
                ImGui::SetItemTooltip("Scale of Toolbox's text elements.");
                ImGui::PopItemWidth();
                ImGui::Checkbox("Style Editor", &toolbox.styleEditor);
                ImGui::SetItemTooltip("Opens the Style Editor.");
                ImGui::Text("%s", std::format("Toolbox {}", Toolbox::getMod()->getVersion().toVString(true)).c_str());
                ImGui::SetItemTooltip("%s", toolbox.currentFunny.c_str());
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Universal")) {
                ImGui::PushItemWidth(200.f); 
                ImGui::InputFloat("Music Volume", &Toolbox::getAudioEngine()->m_musicVolume);
                ImGui::SetItemTooltip("Equivalent to GD's music volume slider.");
                ImGui::InputFloat("SFX Volume", &Toolbox::getAudioEngine()->m_sfxVolume);
                ImGui::SetItemTooltip("Equivalent to GD's SFX volume slider.");
                ImGui::PopItemWidth();
                ImGui::InputFloat("##speedhack", &toolbox.speedhack);
                ImGui::SameLine();
                ImGui::Checkbox("Speedhack", &toolbox.speedhackEnabled);
                ImGui::InputDouble("##tpsBypass", &toolbox.tpsBypass);
                ImGui::SameLine();
                ImGui::Checkbox("TPS Bypass", &toolbox.tpsBypassEnabled);
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Player")) {
                if (ImGui::TreeNode("Auto Kill")) {
                    ImGui::Checkbox("Enabled", &toolbox.autoKill);
                    ImGui::SetItemTooltip("Enables Auto Kill.");
                    ImGui::InputFloat("Percentage", &toolbox.autoKillPercentage);
                    ImGui::SetItemTooltip("Percentage at which to automatically kill the player.");
                    ImGui::TreePop();
                };
                if (ImGui::TreeNode("Custom Wave Trail")) {
                    ImGui::Checkbox("Enabled", &toolbox.customWaveTrail);
                    ImGui::SetItemTooltip("Enables Custom Wave Trail.");
                    ImGui::Checkbox("No Wave Trail", &toolbox.noWaveTrail);
                    ImGui::SetItemTooltip("Disables the wave trail.");
                    ImGui::PushItemWidth(200.f); 
                    if (ImGui::ColorEdit3("##waveTrailColorPicker", toolbox.waveTrailColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.colorPickerChange();
                    ImGui::SameLine();
                    ImGui::Checkbox("Wave Trail Color", &toolbox.useWaveTrailColor);
                    ImGui::InputFloat("Wave Trail Size", &toolbox.waveTrailSize);
                    ImGui::SetItemTooltip("Multiplier for the wave trail's size.");
                    ImGui::PopItemWidth();
                    ImGui::TreePop();
                };
                if (ImGui::TreeNode("Garage")) {
                    ImGui::InputText("Kit Name", toolbox.iconSaveName, 64);
                    if (ImGui::Button("Save Kit")) toolbox.saveIcons();
                    ImGui::SameLine();
                    if (ImGui::Button("Load Kit")) toolbox.loadIcons();
                    ImGui::TreePop();
                };
                ImGui::Checkbox("Maintain Gravity", &toolbox.maintainGravity);
                if (ImGui::TreeNode("Manager")) {
                    if (PlayLayer::get()==nullptr) ImGui::Text("No level open");
                    else {
                        if (ImGui::TreeNode("Position")) {
                            ImGui::PushItemWidth(200.f); 
                            ImGui::InputFloat("##p1x", &GJBaseGameLayer::get()->m_player1->m_position.x);
                            ImGui::SameLine();
                            ImGui::InputFloat("##p1y", &GJBaseGameLayer::get()->m_player1->m_position.y);
                            ImGui::SameLine();
                            ImGui::Text("Player 1");
                            ImGui::InputFloat("##p2x", &GJBaseGameLayer::get()->m_player2->m_position.x);
                            ImGui::SameLine();
                            ImGui::InputFloat("##p2y", &GJBaseGameLayer::get()->m_player2->m_position.y);
                            ImGui::SameLine();
                            ImGui::Text("Player 2");
                            ImGui::PopItemWidth();
                            ImGui::TreePop();
                        };
                        if (ImGui::TreeNode("Gamemode")) {
                            if (ImGui::TreeNode("Player 1")) {
                                toolbox.gamemodeRadio1=Toolbox::getGamemode(GJBaseGameLayer::get()->m_player1);
                                if (ImGui::RadioButton("Cube", &toolbox.gamemodeRadio1, 0)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 0);
                                if (ImGui::RadioButton("Ship", &toolbox.gamemodeRadio1, 1)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 1);
                                if (ImGui::RadioButton("Ball", &toolbox.gamemodeRadio1, 2)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 2);
                                if (ImGui::RadioButton("UFO", &toolbox.gamemodeRadio1, 3)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 3);
                                if (ImGui::RadioButton("Wave", &toolbox.gamemodeRadio1, 4)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 4);
                                if (ImGui::RadioButton("Robot", &toolbox.gamemodeRadio1, 5)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 5);
                                if (ImGui::RadioButton("Spider", &toolbox.gamemodeRadio1, 6)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 6);
                                if (ImGui::RadioButton("Swing", &toolbox.gamemodeRadio1, 7)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player1, 7);
                                ImGui::TreePop();
                            };
                            if (ImGui::TreeNode("Player 2")) {
                                toolbox.gamemodeRadio2=Toolbox::getGamemode(GJBaseGameLayer::get()->m_player2);
                                if (ImGui::RadioButton("Cube", &toolbox.gamemodeRadio2, 0)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 0);
                                if (ImGui::RadioButton("Ship", &toolbox.gamemodeRadio2, 1)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 1);
                                if (ImGui::RadioButton("Ball", &toolbox.gamemodeRadio2, 2)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 2);
                                if (ImGui::RadioButton("UFO", &toolbox.gamemodeRadio2, 3)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 3);
                                if (ImGui::RadioButton("Wave", &toolbox.gamemodeRadio2, 4)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 4);
                                if (ImGui::RadioButton("Robot", &toolbox.gamemodeRadio2, 5)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 5);
                                if (ImGui::RadioButton("Spider", &toolbox.gamemodeRadio2, 6)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 6);
                                if (ImGui::RadioButton("Swing", &toolbox.gamemodeRadio2, 7)) toolbox.switchGamemodeFromMenu(GJBaseGameLayer::get()->m_player2, 7);
                                ImGui::TreePop();
                            };
                            ImGui::TreePop();
                        };
                    };
                    ImGui::TreePop();
                };
                ImGui::Checkbox("Noclip", &toolbox.noclip);
                ImGui::SetItemTooltip("Prevents the player from dying.");
                ImGui::Checkbox("No Checkpoint Delay", &toolbox.noCheckpointDelay);
                ImGui::SetItemTooltip("Removes the frame of delay before checkpoint placement.");
                ImGui::Checkbox("No Respawn Flash", &toolbox.noRespawnFlash);
                ImGui::SetItemTooltip("Disables the flash effect upon respawn.");
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Bypass")) {
                ImGui::Checkbox("Icon Hack", &toolbox.iconHack);
                ImGui::SetItemTooltip("Bypasses icon & color unlock requirements.");
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Level")) {
                if (ImGui::TreeNode("Level Info")) {
                    if (toolbox.lastLevel==nullptr) ImGui::Text("No level loaded");
                    else {
                        ImGui::Text("ID: %s", std::to_string((*toolbox.lastLevel).m_levelID.value()).c_str());
                        ImGui::Text("Name: %s", (*toolbox.lastLevel).m_levelName.c_str());
                        ImGui::Text("Creator: %s", (*toolbox.lastLevel).m_creatorName.c_str());
                        ImGui::Text("Objects: %s", std::to_string((*toolbox.lastLevel).m_objectCount.value()).c_str());
                        ImGui::Text("Version: %s", std::to_string((*toolbox.lastLevel).m_levelVersion).c_str());
                        ImGui::Text("Password: %s", Toolbox::realPassword((*toolbox.lastLevel).m_password).c_str());
                        ImGui::Text("Attempts: %s", std::to_string((*toolbox.lastLevel).m_attempts.value()).c_str());
                        ImGui::Text("Jumps: %s", std::to_string((*toolbox.lastLevel).m_jumps.value()).c_str());
                    };
                    ImGui::TreePop();
                };
                ImGui::Checkbox("No Collide", &toolbox.noCollide);
                ImGui::SetItemTooltip("Disables object collision entirely.");
                ImGui::Checkbox("No Object Glow", &toolbox.noObjectGlow);
                ImGui::SetItemTooltip("Disables object glow.");
                if (ImGui::TreeNode("Show Layout")) {
                    ImGui::Checkbox("Enabled", &toolbox.showLayout);
                    if (ImGui::ColorEdit3("BG Color##layoutBGColorPicker", toolbox.layoutBGColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.colorPickerChange();
                    if (ImGui::ColorEdit3("G1 Color##layoutG1ColorPicker", toolbox.layoutG1ColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.colorPickerChange();
                    if (ImGui::ColorEdit3("G2 Color##layoutG2ColorPicker", toolbox.layoutG2ColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.colorPickerChange();
                    if (ImGui::ColorEdit3("Line Color##layoutLineColorPicker", toolbox.layoutLineColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.colorPickerChange();
                    if (ImGui::ColorEdit3("MG Color##layoutMGColorPicker", toolbox.layoutMGColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.colorPickerChange();
                    if (ImGui::ColorEdit3("MG2 Color##layoutMG2ColorPicker", toolbox.layoutMG2ColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.colorPickerChange();
                    ImGui::TreePop();
                };
                if (ImGui::Button("Clear Shaders")) GJBaseGameLayer::get()->m_shaderLayer->resetAllShaders();
                ImGui::SetItemTooltip("Clears all currently running shaders.");
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Creator")) {
                ImGui::Checkbox("Individual Rotate", &toolbox.individualRotate);
                ImGui::SetItemTooltip("Rotates objects individually when attempting to rotate a selection.");
                ImGui::Checkbox("Verify Hack", &toolbox.verifyHack);
                ImGui::SetItemTooltip("Allows unverified levels to be uploaded.");
                if (ImGui::Button("Shuffle Object Positions")&&EditorUI::get()!=nullptr&&EditorUI::get()->m_selectedObjects->count()>1) {
                    int count=EditorUI::get()->m_selectedObjects->count();
                    std::vector<cocos2d::CCPoint> positions;
                    std::vector<GameObject*> selectedObjects=Toolbox::objectVec(EditorUI::get()->m_selectedObjects);
                    for (int i=0; i<count; ++i) positions.push_back(selectedObjects[i]->getUnmodifiedPosition());
                    std::ranges::shuffle(positions, std::default_random_engine{});
                    for (int i=0; i<count; ++i) selectedObjects[i]->setPosition(positions[i]);
                };
                ImGui::SetItemTooltip("Shuffles the positions of all selected objects.");
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Rogue")) {
                ImGui::InputText("Macro Name", rogue.macro.name, IM_ARRAYSIZE(rogue.macro.name));
                ImGui::InputDouble("Macro TPS", &rogue.macro.tps);
                ImGui::Text("Bot State");
                ImGui::SameLine();
                ImGui::RadioButton("Off", &rogue.state, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Record", &rogue.state, 1);
                ImGui::SameLine();
                ImGui::RadioButton("Play", &rogue.state, 2);
                if (ImGui::Button("Save")) rogue.save();
                ImGui::SameLine();
                if (ImGui::Button("Load")) rogue.load();
                ImGui::SameLine();
                if (ImGui::Button("Clear")) rogue.macro.inputs.clear();
                if (ImGui::TreeNode("Replay Format")) {
                    ImGui::RadioButton(".rgb (Rogue)", &rogue.format, 0);
                    ImGui::RadioButton(".slc (Silicate v3)", &rogue.format, 1);
                    ImGui::TreePop();
                };
                if (PlayLayer::get()!=nullptr) ImGui::Text("tick %s", std::to_string(Shared::getTick()).c_str());
                else ImGui::Text("tick 0");
                ImGui::Text("inputs %s", std::to_string(rogue.macro.inputs.size()).c_str());
                ImGui::Text("last press %s", std::to_string(rogue.lastPress).c_str());
                ImGui::Text("last release %s", std::to_string(rogue.lastRelease).c_str());
                ImGui::TreePop();
            };
            ImGui::End();
        };
        toolbox.save();
    });
};
class $modify(UILayer) {
    void onCheck(CCObject* sender) {
        UILayer::onCheck(sender);
        if ((rogue.state==1||toolbox.noCheckpointDelay)&&PlayLayer::get()!=nullptr) {
            PlayLayer::get()->m_tryPlaceCheckpoint=false;
            PlayLayer::get()->markCheckpoint();
        };
    };
};
class $modify(LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        toolbox.lastLevel=level;
        return LevelInfoLayer::init(level, challenge);
    };
};
class $modify(GameObject) {
    void addGlow(gd::string frame) {
        if (!toolbox.noObjectGlow) GameObject::addGlow(frame);
        if (toolbox.showLayout&&PlayLayer::get()!=nullptr) m_isHide=(Toolbox::isDecoration(this));
    };
    void setOpacity(unsigned char opacity) {
        if (toolbox.showLayout&&PlayLayer::get()!=nullptr) {
            GameObject::setOpacity((Toolbox::isDecoration(this))?0:255);
            return;
        };
        GameObject::setOpacity(opacity);
    };
    void setVisible(bool visible) {
        if (toolbox.showLayout&&PlayLayer::get()!=nullptr) {
            GameObject::setVisible(!Toolbox::isDecoration(this));
            return;
        };
        GameObject::setVisible(visible);
    };
    CCRect getObjectRect(float width, float height) {
        CCRect ret=GameObject::getObjectRect(width, height);
        if (toolbox.noCollide) ret.size.setSize(0, 0);
        return ret;
    };
};
class $modify(ToolboxPlayer, PlayerObject) {
    struct Fields {
        bool m_isMGInput;
        bool m_jumpIsDown;
    };
    bool pushButton(PlayerButton button) {
        if (Shared::getTick()&&!m_fields->m_isMGInput&&toolbox.maintainGravity&&button==PlayerButton::Jump&&m_isUpsideDown) {
            m_fields->m_isMGInput=true;
            bool ret=!PlayerObject::releaseButton(button);
            m_fields->m_isMGInput=false;
            return ret;
        };
        m_fields->m_jumpIsDown=true;
        return PlayerObject::pushButton(button);
    };
    bool releaseButton(PlayerButton button) {
        if (Shared::getTick()&&!m_fields->m_isMGInput&&toolbox.maintainGravity&&button==PlayerButton::Jump&&m_isUpsideDown) {
            m_fields->m_isMGInput=true;
            bool ret=!PlayerObject::pushButton(button);
            m_fields->m_isMGInput=false;
            return ret;
        };
        m_fields->m_jumpIsDown=false;
        return PlayerObject::releaseButton(button);
    };
    void flipGravity(bool flip, bool noEffects) {
        if (toolbox.maintainGravity) {
            if (m_fields->m_jumpIsDown) {
                m_fields->m_isMGInput=true;
                PlayerObject::releaseButton(PlayerButton::Jump);
                m_fields->m_isMGInput=false;
            } else {
                m_fields->m_isMGInput=true;
                PlayerObject::pushButton(PlayerButton::Jump);
                m_fields->m_isMGInput=false;
            };
        };
        PlayerObject::flipGravity(flip, noEffects);
    };
	void update(float dt) {
		PlayerObject::update(dt);
        if (toolbox.customWaveTrail) {
		    if (toolbox.noWaveTrail) m_waveTrail->reset();
            if (toolbox.useWaveTrailColor) m_waveTrail->setColor(toolbox.waveTrailColor);
		    m_waveTrail->m_waveSize=toolbox.waveTrailSize*m_vehicleSize;
        };
	};
	void playSpawnEffect() {
        if (toolbox.noRespawnFlash) return;
        PlayerObject::playSpawnEffect();
	};
};
class $modify(GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        if (toolbox.iconHack) return true;
        return GameManager::isIconUnlocked(id, type);
    };
    bool isColorUnlocked(int id, UnlockType type) {
        if (toolbox.iconHack) return true;
        return GameManager::isColorUnlocked(id, type);
    };
};
class $modify(GJBaseGameLayer) {
    void updateColor(ccColor3B& color, float fadeTime, int colorID, bool blending, float opacity, ccHSVValue& copyHSV, int colorIDToCopy, bool copyOpacity, EffectGameObject* callerObject, int unk1, int unk2) {
        if (toolbox.showLayout) {
            switch (colorID) {
                case 1000: {
                    GJBaseGameLayer::updateColor(toolbox.layoutBGColor, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
                    return;
                };
                case 1001: {
                    GJBaseGameLayer::updateColor(toolbox.layoutG1Color, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
                    return;
                };
                case 1002: {
                    GJBaseGameLayer::updateColor(toolbox.layoutLineColor, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
                    return;
                };
                case 1009: {
                    GJBaseGameLayer::updateColor(toolbox.layoutG2Color, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
                    return;
                };
                case 1013: {
                    GJBaseGameLayer::updateColor(toolbox.layoutMGColor, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
                    return;
                };
                case 1014: {
                    GJBaseGameLayer::updateColor(toolbox.layoutMG2Color, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
                    return;
                };
            };
            
        }
        GJBaseGameLayer::updateColor(color, fadeTime, colorID, blending, opacity, copyHSV, colorIDToCopy, copyOpacity, callerObject, unk1, unk2);
    }
    void resetPlayer() {
        toolbox.oneTimeCheatThisAttempt=false;
        GJBaseGameLayer::resetPlayer();
    };
    void update(float dt) {
        if (Shared::getTick()<1) toolbox.oneTimeCheatThisAttempt=false;
        toolbox.lastLevel=this->m_level;
        if (toolbox.autoKill&&!this->m_level->isPlatformer()&&PlayLayer::get()->getCurrentPercent()>=toolbox.autoKillPercentage) PlayLayer::get()->resetLevelFromStart();
        GJBaseGameLayer::update(dt);
    };
};
class $modify(cocos2d::CCScheduler) {
    void update(float dt) {
        if (toolbox.speedhackEnabled) {
            CCScheduler::update(dt*toolbox.speedhack);
            return;
        };
        CCScheduler::update(dt);
    };
};
class $modify(EditorUI) {
    void rotateObjects(CCArray* objects, float rotation, CCPoint pivotPoint) {
        if (toolbox.individualRotate) {
            for (auto& obj:Toolbox::objectVec(EditorUI::get()->m_selectedObjects)) if (obj->canRotateFree()||!std::fmod(rotation, 90.f)) obj->addRotation(rotation);
            return;
        };
        EditorUI::rotateObjects(objects, rotation, pivotPoint);
    };
};
class $modify(PlayLayer) {
    void addObject(GameObject* object) {
        if (toolbox.showLayout) {
            if (Toolbox::isCosmetic(object)) return;
            object->m_activeMainColorID=-1;
            object->m_activeDetailColorID=-1;
            object->m_shouldBlendBase=false;
            object->m_shouldBlendDetail=false;
            object->m_baseUsesHSV=false;
            object->m_detailUsesHSV=false;
            if (!Toolbox::isDecoration(object)&&!object->isTrigger()) {
                object->m_isHide=false;
                object->m_hasNoGlow=true;
                object->setVisible(true);
                object->setOpacity(255);
            } else object->m_isHide=true;
        };
        PlayLayer::addObject(object);
    };
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (toolbox.noclip) return;
        PlayLayer::destroyPlayer(player, object);
    };
};
class $modify(GJGameLevel) {
    bool init() {
        bool ret=GJGameLevel::init();
        if (toolbox.verifyHack) m_isVerifiedRaw=true;
        return ret;
    };
};