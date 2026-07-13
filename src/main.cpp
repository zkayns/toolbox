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
        if (rogue.state==2) {
            for (const auto& input:rogue.macro.inputs) {
                if (input.tick==Shared::getTick()) {
                    if (input.player) {
                        if (input.press) GJBaseGameLayer::get()->m_player2->pushButton(input.button);
                        else GJBaseGameLayer::get()->m_player2->releaseButton(input.button);
                    } else {
                        if (input.press) GJBaseGameLayer::get()->m_player1->pushButton(input.button);
                        else GJBaseGameLayer::get()->m_player1->releaseButton(input.button);
                    };
                };
            };
        } else if (rogue.state==1) {
            rogue.macro.seed=GJBaseGameLayer::get()->m_randomSeed;
            rogue.macro.cleanInputs();
        };
        PlayerObject::update(dt);
    };
    bool pushButton(PlayerButton button) {
        if (PlayLayer::get()==nullptr) return PlayerObject::pushButton(button); // prevent menu player inputs wtf bro
        rogue.lastPress=Shared::getTick();
        if (rogue.state==1) {
            rogue.macro.inputs.push_back(RogueInput{
                .player=this->isPlayer2(),
                .press=true,
                .button=button,
                .tick=Shared::getTick()
            });
        };
        return PlayerObject::pushButton(button);
    };
    bool releaseButton(PlayerButton button) {
        if (PlayLayer::get()==nullptr) return PlayerObject::releaseButton(button);
        rogue.lastRelease=Shared::getTick();
        if (rogue.state==1) {
            rogue.macro.inputs.push_back(RogueInput{
                .player=this->isPlayer2(),
                .press=false,
                .button=button,
                .tick=Shared::getTick()
            });
        };
        return PlayerObject::releaseButton(button);
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
    bool isCheating() {
        return (
            noclip||
            noCollide||
            speedhackEnabled||
            rogue.state==2||
            oneTimeCheatThisAttempt
        );
    };
    GJGameLevel* lastLevel=nullptr;
    char iconSaveName[64]="icons";
    std::string realPassword(geode::SeedValueRS password) {
        if (password.value()==0) return "Not Copyable";
        if (password.value()==1) return "Free Copy";
        return std::to_string(password.value()).substr(1);
    };
    void switchGamemodeFromMenu(PlayerObject* player, int mode) {
        oneTimeCheatThisAttempt=true;
        switchGamemode(player, mode);
    };
    bool iconKitExists(std::string fileName) {
        return std::filesystem::exists(iconSavePath(fileName));
    };
    std::filesystem::path iconSavePath(std::string fileName) { return getMod()->getSaveDir()/"iconKits"/(fileName+".tbg"); };
    int getGamemode(PlayerObject* player) {
        if (player->m_isShip) return 1;
        if (player->m_isBall) return 2;
        if (player->m_isBird) return 3;
        if (player->m_isDart) return 4;
        if (player->m_isRobot) return 5;
        if (player->m_isSpider) return 6;
        if (player->m_isSwing) return 7;
        return 0;
    };
    void switchGamemode(PlayerObject* player, int mode) {
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
    GLubyte toGLubyte(float f) { return static_cast<GLubyte>(std::clamp<float>(std::round(f*255.f), 0.f, 255.f)); };
    Mod* getMod() { return Mod::get(); };
    FMODAudioEngine* getAudioEngine() { return FMODAudioEngine::get(); };
    void load() {
        autoKill=getMod()->getSavedValue<bool>("autoKill", false);
        autoKillPercentage=getMod()->getSavedValue<float>("autoKillPercentage", 50.f);
        customWaveTrail=getMod()->getSavedValue<bool>("customWaveTrail", false);
        fontScale=getMod()->getSavedValue<float>("fontScale", 1.f);
        iconHack=getMod()->getSavedValue<bool>("iconHack", false);
        noclip=getMod()->getSavedValue<bool>("noclip", false);
        noCollide=getMod()->getSavedValue<bool>("noCollide", false);
        noObjectGlow=getMod()->getSavedValue<bool>("noObjectGlow", false);
        noRespawnFlash=getMod()->getSavedValue<bool>("noRespawnFlash", false);
        noWaveTrail=getMod()->getSavedValue<bool>("noWaveTrail", false);
        waveTrailSize=getMod()->getSavedValue<float>("waveTrailSize", 1.f);
        speedhack=getMod()->getSavedValue<float>("speedhack", 1.f);
        speedhackEnabled=getMod()->getSavedValue<bool>("speedhackEnabled", false);
        individualRotate=getMod()->getSavedValue<bool>("individualRotate", false);
        waveTrailColor=getMod()->getSavedValue<cocos2d::ccColor3B>("waveTrailColor", cocos2d::ccColor3B{255, 0, 0});
        waveTrailColorPicker[0]=static_cast<float>(waveTrailColor.r)/255.f;
        waveTrailColorPicker[1]=static_cast<float>(waveTrailColor.g)/255.f;
        waveTrailColorPicker[2]=static_cast<float>(waveTrailColor.b)/255.f;
        useWaveTrailColor=getMod()->getSavedValue<bool>("useWaveTrailColor", false);
        noCheckpointDelay=getMod()->getSavedValue<bool>("noCheckpointDelay", false);
        tpsBypass=getMod()->getSavedValue<double>("tpsBypass", 240.0);
        tpsBypassEnabled=getMod()->getSavedValue<bool>("tpsBypassEnabled", false);
        maintainGravity=getMod()->getSavedValue<bool>("maintainGravity", false);
    };
    void save() {
        getMod()->setSavedValue<bool>("autoKill", autoKill);
        getMod()->setSavedValue<float>("autoKillPercentage", autoKillPercentage);
        getMod()->setSavedValue<bool>("customWaveTrail", customWaveTrail);
        getMod()->setSavedValue<float>("fontScale", fontScale);
        getMod()->setSavedValue<bool>("iconHack", iconHack);
        getMod()->setSavedValue<bool>("noclip", noclip);
        getMod()->setSavedValue<bool>("noCollide", noCollide);
        getMod()->setSavedValue<bool>("noObjectGlow", noObjectGlow);
        getMod()->setSavedValue<bool>("noRespawnFlash", noRespawnFlash);
        getMod()->setSavedValue<bool>("noWaveTrail", noWaveTrail);
        getMod()->setSavedValue<float>("waveTrailSize", waveTrailSize);
        getMod()->setSavedValue<float>("speedhack", speedhack);
        getMod()->setSavedValue<float>("speedhackEnabled", speedhackEnabled);
        getMod()->setSavedValue<bool>("individualRotate", individualRotate);
        getMod()->setSavedValue<cocos2d::ccColor3B>("waveTrailColor", waveTrailColor);
        getMod()->setSavedValue<bool>("useWaveTrailColor", useWaveTrailColor);
        getMod()->setSavedValue<bool>("noCheckpointDelay", noCheckpointDelay);
        getMod()->setSavedValue<double>("tpsBypass", tpsBypass);
        getMod()->setSavedValue<bool>("tpsBypassEnabled", tpsBypassEnabled);
        getMod()->setSavedValue<bool>("maintainGravity", maintainGravity);
    };
    std::vector<GameObject*> objectVec(cocos2d::CCArray* source) {
        auto ext=CCArrayExt<GameObject*>(source);
        return std::vector<GameObject*>(ext.begin(), ext.end());
    };
    void updateWaveTrailColor() { waveTrailColor=cocos2d::ccColor3B{toGLubyte(waveTrailColorPicker[0]), toGLubyte(waveTrailColorPicker[1]), toGLubyte(waveTrailColorPicker[2])}; };
};
Toolbox toolbox;
$on_mod(Loaded) {
    toolbox.load();
    ImGuiCocos::get().setup();
    ImGuiCocos::get().draw([]{
        if (toolbox.uiOpen) {
            ImGui::GetIO().FontGlobalScale=toolbox.fontScale;
            if (toolbox.styleEditor) ImGui::ShowStyleEditor();
            ImGui::Begin("Toolbox", &toolbox.uiOpen);
            if (ImGui::TreeNode("Toolbox Settings")) {
                ImGui::PushItemWidth(200.f); 
                ImGui::SliderFloat("Font Scale", &toolbox.fontScale, 0.f, 10.f, "%.2f");
                ImGui::SetItemTooltip("Scale of Toolbox's text elements.");
                ImGui::PopItemWidth();
                ImGui::Checkbox("Style Editor", &toolbox.styleEditor);
                ImGui::SetItemTooltip("Opens the Style Editor.");
                ImGui::Text("%s", std::format("Toolbox {}", toolbox.getMod()->getVersion().toVString(true)).c_str());
                ImGui::SetItemTooltip("%s", toolbox.currentFunny.c_str());
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Universal")) {
                ImGui::PushItemWidth(200.f); 
                ImGui::SliderFloat("Music Volume", &toolbox.getAudioEngine()->m_musicVolume, 0.f, 1.f, "%.2f");
                ImGui::SetItemTooltip("Equivalent to GD's music volume slider.");
                ImGui::SliderFloat("SFX Volume", &toolbox.getAudioEngine()->m_sfxVolume, 0.f, 1.f, "%.2f");
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
                    ImGui::SliderFloat("Percentage", &toolbox.autoKillPercentage, 0.f, 100.f);
                    ImGui::SetItemTooltip("Percentage at which to automatically kill the player.");
                    ImGui::TreePop();
                };
                if (ImGui::TreeNode("Custom Wave Trail")) {
                    ImGui::Checkbox("Enabled", &toolbox.customWaveTrail);
                    ImGui::SetItemTooltip("Enables Custom Wave Trail.");
                    ImGui::Checkbox("No Wave Trail", &toolbox.noWaveTrail);
                    ImGui::SetItemTooltip("Disables the wave trail.");
                    ImGui::PushItemWidth(200.f); 
                    if (ImGui::ColorEdit3("##waveTrailColorPicker", toolbox.waveTrailColorPicker, ImGuiColorEditFlags_Uint8)) toolbox.updateWaveTrailColor();
                    ImGui::SameLine();
                    ImGui::Checkbox("Wave Trail Color", &toolbox.useWaveTrailColor);
                    ImGui::SliderFloat("Wave Trail Size", &toolbox.waveTrailSize, 0.f, 10.f, "%.2f");
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
                                toolbox.gamemodeRadio1=toolbox.getGamemode(GJBaseGameLayer::get()->m_player1);
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
                                toolbox.gamemodeRadio2=toolbox.getGamemode(GJBaseGameLayer::get()->m_player2);
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
                        ImGui::Text("Password: %s", toolbox.realPassword((*toolbox.lastLevel).m_password).c_str());
                        ImGui::Text("Attempts: %s", std::to_string((*toolbox.lastLevel).m_attempts.value()).c_str());
                        ImGui::Text("Jumps: %s", std::to_string((*toolbox.lastLevel).m_jumps.value()).c_str());
                    };
                    ImGui::TreePop();
                };
                ImGui::Checkbox("No Collide", &toolbox.noCollide);
                ImGui::SetItemTooltip("Disables object collision entirely.");
                ImGui::Checkbox("No Object Glow", &toolbox.noObjectGlow);
                ImGui::SetItemTooltip("Disables object glow.");
                if (ImGui::Button("Clear Shaders")) GJBaseGameLayer::get()->m_shaderLayer->resetAllShaders();
                ImGui::SetItemTooltip("Clears all currently running shaders.");
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Creator")) {
                ImGui::Checkbox("Individual Rotate", &toolbox.individualRotate);
                if (ImGui::Button("Shuffle Object Positions")&&EditorUI::get()!=nullptr&&EditorUI::get()->m_selectedObjects->count()>1) {
                    int count=EditorUI::get()->m_selectedObjects->count();
                    std::vector<cocos2d::CCPoint> positions;
                    std::vector<GameObject*> selectedObjects=toolbox.objectVec(EditorUI::get()->m_selectedObjects);
                    for (int i=0; i<count; ++i) positions.push_back(selectedObjects[i]->getUnmodifiedPosition());
                    std::ranges::shuffle(positions, std::default_random_engine{});
                    for (int i=0; i<count; ++i) selectedObjects[i]->setPosition(positions[i]);
                };
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
        if (toolbox.noObjectGlow) return;
        GameObject::addGlow(frame);
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
            for (auto& obj:toolbox.objectVec(EditorUI::get()->m_selectedObjects)) if (obj->canRotateFree()||!std::fmod(rotation, 90.f)) obj->addRotation(rotation);
            return;
        };
        EditorUI::rotateObjects(objects, rotation, pivotPoint);
    };
};
class $modify(PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (toolbox.noclip) return;
        PlayLayer::destroyPlayer(player, object);
    };
};
$execute {
    geode::utils::file::createDirectory(Mod::get()->getSaveDir()/"iconKits");
    geode::utils::file::createDirectory(Mod::get()->getSaveDir()/"rogue");
    geode::utils::file::createDirectory(Mod::get()->getSaveDir()/"rogue"/"macros");
	listenForKeybindSettingPresses("kb-toggle", [](Keybind const& keybind, bool down, bool repeat, double dt) {
		if (down&&!repeat) {
            toolbox.currentFunny=toolbox.funnies[rand()%size(toolbox.funnies)];
            toolbox.uiOpen=!toolbox.uiOpen;
        };
	});
};
