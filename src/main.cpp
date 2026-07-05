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
#include <imgui-cocos.hpp>
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
struct RogueInput {
    bool player;
    bool press;
    uint64_t tick;
    PlayerButton button;
    bool remove;
};
struct RogueMacro {
    char name[128]="";
    double tps=240;
    int seed=0;
    std::vector<RogueInput> inputs;
};
struct Rogue {
    RogueMacro macro;
    int state=0;
    uint64_t lastPress;
    uint64_t lastRelease;
    bool p1j;
    bool p2j;
    void save() {}; // UNI
    void load() {}; // UNI
    void cleanInputs() {
        bool lp1=false;
        bool lp2=false;
        bool p1=false;
        bool p2=false;
        for (auto& input:macro.inputs) {
            lp1=p1;
            lp2=p2;
            if (!input.player) p1=input.press;
            else p2=input.press;
            if (lp1==p1&&lp2==p2) input.remove=true;
        };
    };
};
Rogue rogue;
class $modify(PlayerObject) {
    void update(float dt) {
        if (!isPlayer1()) {
            PlayerObject::update(dt);
            return;
        };
        if (rogue.state==2) {
            for (const auto& input:rogue.macro.inputs) {
                if (input.tick==GJBaseGameLayer::get()->m_gameState.m_currentProgress/2) {
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
            std::erase_if(rogue.macro.inputs, [](RogueInput input){
                if (input.tick>GJBaseGameLayer::get()->m_gameState.m_currentProgress/2) return true;
                if (!GJBaseGameLayer::get()->m_isPlatformer&&input.button!=PlayerButton::Jump) return true;
                return input.remove;
            });
            rogue.cleanInputs();
        };
        PlayerObject::update(dt);
    };
    bool pushButton(PlayerButton button) {
        if (PlayLayer::get()==nullptr) return PlayerObject::pushButton(button); // prevent menu player inputs wtf bro
        rogue.lastPress=GJBaseGameLayer::get()->m_gameState.m_currentProgress/2;
        if (rogue.state==1) {
            rogue.macro.inputs.push_back(RogueInput{
                .player=this->isPlayer2(),
                .press=true,
                .button=button,
                .tick=GJBaseGameLayer::get()->m_gameState.m_currentProgress/2
            });
        };
        return PlayerObject::pushButton(button);
    };
    bool releaseButton(PlayerButton button) {
        if (PlayLayer::get()==nullptr) return PlayerObject::releaseButton(button);
        rogue.lastRelease=GJBaseGameLayer::get()->m_gameState.m_currentProgress/2;
        if (rogue.state==1) {
            rogue.macro.inputs.push_back(RogueInput{
                .player=this->isPlayer2(),
                .press=false,
                .button=button,
                .tick=GJBaseGameLayer::get()->m_gameState.m_currentProgress/2
            });
        };
        return PlayerObject::releaseButton(button);
    };
};
class $modify(PlayLayer) {
    void loadFromCheckpoint(CheckpointObject* object) {
        PlayLayer::loadFromCheckpoint(object);
        if (rogue.state==1) rogue.cleanInputs();
    };
};
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
    float waveTrailSize;
    GJGameLevel* lastLevel=nullptr;
    char iconSaveName[64]="icons";
    std::string realPassword(geode::SeedValueRS password) {
        if (password.value()==0) return "Not Copyable";
        if (password.value()==1) return "Free Copy";
        return std::to_string(password.value()).substr(1);
    };
    bool iconKitExists(std::string fileName) {
        return std::filesystem::exists(iconSavePath(fileName));
    };
    std::filesystem::path iconSavePath(std::string fileName) {
        return getMod()->getSaveDir()/"iconKits"/fileName;
    };
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
    };
    std::vector<GameObject*> editorObjectVec(cocos2d::CCArray* source) {
        auto ext=CCArrayExt<GameObject*>(source);
        return std::vector<GameObject*>(ext.begin(), ext.end());
    };
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
                ImGui::InputFloat("##", &toolbox.speedhack);
                ImGui::SameLine();
                ImGui::Checkbox("Speedhack", &toolbox.speedhackEnabled);
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
                                if (ImGui::RadioButton("Cube", &toolbox.gamemodeRadio1, 0)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 0);
                                if (ImGui::RadioButton("Ship", &toolbox.gamemodeRadio1, 1)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 1);
                                if (ImGui::RadioButton("Ball", &toolbox.gamemodeRadio1, 2)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 2);
                                if (ImGui::RadioButton("UFO", &toolbox.gamemodeRadio1, 3)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 3);
                                if (ImGui::RadioButton("Wave", &toolbox.gamemodeRadio1, 4)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 4);
                                if (ImGui::RadioButton("Robot", &toolbox.gamemodeRadio1, 5)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 5);
                                if (ImGui::RadioButton("Spider", &toolbox.gamemodeRadio1, 6)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 6);
                                if (ImGui::RadioButton("Swing", &toolbox.gamemodeRadio1, 7)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player1, 7);
                                ImGui::TreePop();
                            };
                            if (ImGui::TreeNode("Player 2")) {
                                toolbox.gamemodeRadio2=toolbox.getGamemode(GJBaseGameLayer::get()->m_player2);
                                if (ImGui::RadioButton("Cube", &toolbox.gamemodeRadio2, 0)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 0);
                                if (ImGui::RadioButton("Ship", &toolbox.gamemodeRadio2, 1)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 1);
                                if (ImGui::RadioButton("Ball", &toolbox.gamemodeRadio2, 2)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 2);
                                if (ImGui::RadioButton("UFO", &toolbox.gamemodeRadio2, 3)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 3);
                                if (ImGui::RadioButton("Wave", &toolbox.gamemodeRadio2, 4)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 4);
                                if (ImGui::RadioButton("Robot", &toolbox.gamemodeRadio2, 5)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 5);
                                if (ImGui::RadioButton("Spider", &toolbox.gamemodeRadio2, 6)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 6);
                                if (ImGui::RadioButton("Swing", &toolbox.gamemodeRadio2, 7)) toolbox.switchGamemode(GJBaseGameLayer::get()->m_player2, 7);
                                ImGui::TreePop();
                            };
                            ImGui::TreePop();
                        };
                    };
                    ImGui::TreePop();
                };
                ImGui::Checkbox("Noclip", &toolbox.noclip);
                ImGui::SetItemTooltip("Prevents the player from dying.");
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
                    std::vector<GameObject*> selectedObjects=toolbox.editorObjectVec(EditorUI::get()->m_selectedObjects);
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
                if (PlayLayer::get()!=nullptr) ImGui::Text("tick %s", std::to_string(GJBaseGameLayer::get()->m_gameState.m_currentProgress/2).c_str());
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
class $modify(PlayerObject) {
	void update(float dt) {
		this->update(dt);
        if (toolbox.customWaveTrail) {
		    if (toolbox.noWaveTrail) this->m_waveTrail->reset();
		    this->m_waveTrail->m_waveSize=toolbox.waveTrailSize*this->m_vehicleSize;
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
    void update(float dt) {
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
            for (auto& obj:toolbox.editorObjectVec(EditorUI::get()->m_selectedObjects)) if (obj->canRotateFree()||!std::fmod(rotation, 90.f)) obj->addRotation(rotation);
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
