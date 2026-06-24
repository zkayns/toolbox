#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CheckpointObject.hpp>
#include <imgui-cocos.hpp>
#include "SavedIconsEntry.cpp"
using namespace geode::prelude;
struct RogueInput {
    bool player;
    bool press;
    PlayerButton button;
    uint64_t tick;
};
struct RogueMacro {
    char name[128]="";
    double tps=240;
    int seed=0;
    bool platformer=false;
    bool ldm=false;
    std::vector<RogueInput> inputs;
};
struct Rogue {
    RogueMacro macro;
    int state=0;
    uint64_t tick;
};
Rogue rogue;
class $modify(PlayerObject) {
    bool pushButton(PlayerButton button) {
        if (rogue.state==1) {
            rogue.macro.inputs.push_back(RogueInput{
                .player=this->isPlayer2(),
                .press=true,
                .button=button,
                .tick=rogue.tick
            });
        };
        return PlayerObject::pushButton(button);
    };
    bool releaseButton(PlayerButton button) {
        if (rogue.state==1) {
            rogue.macro.inputs.push_back(RogueInput{
                .player=this->isPlayer2(),
                .press=false,
                .button=button,
                .tick=rogue.tick
            });
        };
        return PlayerObject::releaseButton(button);
    };
    void update(float dt) {
        if (isPlayer1()) {
            if (rogue.state==2) {
                for (const auto& input:rogue.macro.inputs) {
                    if (input.tick==rogue.tick) {
                        if (input.player) {
                            if (input.press) GJBaseGameLayer::get()->m_player2->pushButton(input.button);
                            else GJBaseGameLayer::get()->m_player2->releaseButton(input.button);
                        } else {
                            if (input.press) GJBaseGameLayer::get()->m_player1->pushButton(input.button);
                            else GJBaseGameLayer::get()->m_player1->releaseButton(input.button);
                        };
                    };
                };
            } else if (rogue.state==1) std::erase_if(rogue.macro.inputs, [](RogueInput input){
                return input.tick>rogue.tick;
            });
            if (rogue.state==1) rogue.macro.seed=GJBaseGameLayer::get()->m_randomSeed;
        };
        PlayerObject::update(dt);
        if (isPlayer1()) rogue.tick++;
    };
};
class $modify(RogueCheckpoint, CheckpointObject) {
    struct Fields {
        uint64_t m_tick=0;
    };
    bool init() {
        if (rogue.tick==0&&rogue.state==1) return false; // geek ass fix
        bool r=CheckpointObject::init();
        m_fields->m_tick=rogue.tick-1;
        return r;
    };
};
class $modify(PlayLayer) {
    void startGame() {
        rogue.tick=0;  
        PlayLayer::startGame();
    };
    void resetLevel() {
        rogue.tick=0;
        PlayLayer::resetLevel();
    };
    void loadFromCheckpoint(CheckpointObject* object) {
        rogue.tick=static_cast<RogueCheckpoint*>(object)->m_fields->m_tick;  
        PlayLayer::loadFromCheckpoint(object);
    };
};
struct Toolbox {
    bool styleEditor=false;
    bool uiOpen=false;
    std::string funnies[12]={
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
        autoKillPercentage=getMod()->getSavedValue<float>("autoKillPercentage", 50.0f);
        customWaveTrail=getMod()->getSavedValue<bool>("customWaveTrail", false);
        fontScale=getMod()->getSavedValue<float>("fontScale", 1.0f);
        iconHack=getMod()->getSavedValue<bool>("iconHack", false);
        noclip=getMod()->getSavedValue<bool>("noclip", false);
        noCollide=getMod()->getSavedValue<bool>("noCollide", false);
        noObjectGlow=getMod()->getSavedValue<bool>("noObjectGlow", false);
        noRespawnFlash=getMod()->getSavedValue<bool>("noRespawnFlash", false);
        noWaveTrail=getMod()->getSavedValue<bool>("noWaveTrail", false);
        waveTrailSize=getMod()->getSavedValue<float>("waveTrailSize", 1.0f);
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
                ImGui::SliderFloat("Font Scale", &toolbox.fontScale, 0.0f, 10.0f, "%.2f");
                ImGui::SetItemTooltip("Scale of Toolbox's text elements.");
                ImGui::Checkbox("Style Editor", &toolbox.styleEditor);
                ImGui::SetItemTooltip("Opens the Style Editor.");
                ImGui::Text("%s", std::format("Toolbox {}", toolbox.getMod()->getVersion().toVString(true)).c_str());
                ImGui::SetItemTooltip("%s", toolbox.currentFunny.c_str());
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Universal")) {
                ImGui::SliderFloat("Music Volume", &toolbox.getAudioEngine()->m_musicVolume, 0.0f, 1.0f, "%.2f");
                ImGui::SetItemTooltip("Equivalent to GD's music volume slider.");
                ImGui::SliderFloat("SFX Volume", &toolbox.getAudioEngine()->m_sfxVolume, 0.0f, 1.0f, "%.2f");
                ImGui::SetItemTooltip("Equivalent to GD's SFX volume slider.");
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Player")) {
                if (ImGui::TreeNode("Auto Kill")) {
                    ImGui::Checkbox("Enabled", &toolbox.autoKill);
                    ImGui::SetItemTooltip("Enables Auto Kill.");
                    ImGui::SliderFloat("Percentage", &toolbox.autoKillPercentage, 0.0f, 100.0f);
                    ImGui::SetItemTooltip("Percentage at which to automatically kill the player.");
                    ImGui::TreePop();
                };
                if (ImGui::TreeNode("Custom Wave Trail")) {
                    ImGui::Checkbox("Enabled", &toolbox.customWaveTrail);
                    ImGui::SetItemTooltip("Enables Custom Wave Trail.");
                    ImGui::Checkbox("No Wave Trail", &toolbox.noWaveTrail);
                    ImGui::SetItemTooltip("Disables the wave trail.");
                    ImGui::SliderFloat("Wave Trail Size", &toolbox.waveTrailSize, 0.0f, 10.0f, "%.2f");
                    ImGui::SetItemTooltip("Multiplier for the wave trail's size.");
                    ImGui::TreePop();
                };
                if (ImGui::TreeNode("Garage")) {
                    ImGui::InputText("Kit Name", toolbox.iconSaveName, 64);
                    if (ImGui::Button("Save Kit")) toolbox.saveIcons();
                    ImGui::SameLine();
                    if (ImGui::Button("Load Kit")) toolbox.loadIcons();
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
                if (ImGui::Button("Clear")) rogue.macro.inputs.clear();
                ImGui::Text("tick %s", std::to_string(rogue.tick).c_str());
                ImGui::Text("inputs %s", std::to_string(rogue.macro.inputs.size()).c_str());
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
        if (!toolbox.noObjectGlow) GameObject::addGlow(frame);
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
        if (!toolbox.noRespawnFlash) PlayerObject::playSpawnEffect();
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
class $modify(PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (!toolbox.noclip) PlayLayer::destroyPlayer(player, object);
    };
};
$execute {
    geode::utils::file::createDirectory(Mod::get()->getSaveDir()/"iconKits");
	listenForKeybindSettingPresses("kb-toggle", [](Keybind const& keybind, bool down, bool repeat, double dt) {
		if (down&&!repeat) {
            toolbox.currentFunny=toolbox.funnies[rand()%size(toolbox.funnies)];
            toolbox.uiOpen=!toolbox.uiOpen;
        };
	});
};
