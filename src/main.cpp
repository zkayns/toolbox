#include <Geode/Geode.hpp>
#include <Geode/modify/GJScoreCell.hpp>
#include <Geode/modify/CurrencyRewardLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <imgui-cocos.hpp>
#include "SavedIconsEntry.cpp";
using namespace geode::prelude;
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
    bool customWaveTrail;
    bool incognito;
    bool noclip;
    bool noCollide;
    bool noObjectGlow;
    bool noRespawnFlash;
    bool noWaveTrail;
    bool iconHack;
    float fontScale;
    float waveTrailSize;
    char iconSaveName[64]="icons";
    void loadIcons() {
        geode::utils::file::readFromJson<SavedIconsEntry>(getMod()->getSaveDir()/"iconKits"/iconSaveName).ok()->load();
    };
    void saveIcons() {
        geode::utils::file::writeToJson<SavedIconsEntry>(getMod()->getSaveDir()/"iconKits"/iconSaveName, SavedIconsEntry::fromCurrent());
    };
    Mod* getMod() { return Mod::get(); };
    FMODAudioEngine* getAudioEngine() { return FMODAudioEngine::get(); };
    void load() {
        customWaveTrail=getMod()->getSavedValue<bool>("customWaveTrail", false);
        fontScale=getMod()->getSavedValue<float>("fontScale", 1.0f);
        iconHack=getMod()->getSavedValue<bool>("iconHack", false);
        incognito=getMod()->getSavedValue<bool>("incognito", false);
        noclip=getMod()->getSavedValue<bool>("noclip", false);
        noCollide=getMod()->getSavedValue<bool>("noCollide", false);
        noObjectGlow=getMod()->getSavedValue<bool>("noObjectGlow", false);
        noRespawnFlash=getMod()->getSavedValue<bool>("noRespawnFlash", false);
        noWaveTrail=getMod()->getSavedValue<bool>("noWaveTrail", false);
        waveTrailSize=getMod()->getSavedValue<float>("waveTrailSize", 1.0f);
    };
    void save() {
        getMod()->setSavedValue<bool>("customWaveTrail", customWaveTrail);
        getMod()->setSavedValue<float>("fontScale", fontScale);
        getMod()->setSavedValue<bool>("iconHack", iconHack);
        getMod()->setSavedValue<bool>("incognito", incognito);
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
                ImGui::Checkbox("Incognito", &toolbox.incognito);
                ImGui::SetItemTooltip("Hides information that could potentially be used to identify the current account.");
                ImGui::TreePop();
            };
            if (ImGui::TreeNode("Player")) {
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
                ImGui::Checkbox("No Collide", &toolbox.noCollide);
                ImGui::SetItemTooltip("Disables object collision entirely.");
                ImGui::Checkbox("No Object Glow", &toolbox.noObjectGlow);
                ImGui::SetItemTooltip("Disables object glow.");
                if (ImGui::Button("Clear Shaders")) GJBaseGameLayer::get()->m_shaderLayer->resetAllShaders();
                ImGui::SetItemTooltip("Clears all currently running shaders.");
                ImGui::TreePop();
            };
            ImGui::End();
        };
        toolbox.save();
    });
};
class $modify(GJScoreCell) {
    void update(float delta) {
        GJScoreCell::update(delta);
        if (toolbox.incognito) this->m_backgroundLayer->setVisible(false);
    };
};
class $modify(CurrencyRewardLayer) {
    void update(float dt) {
        CurrencyRewardLayer::update(dt);
        if (toolbox.incognito) this->m_mainNode->setVisible(false);
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
	virtual void update(float dt) {
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
