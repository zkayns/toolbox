#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <imgui-cocos.hpp>
using namespace geode::prelude;
struct Toolbox {
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
    bool noclip;
    bool noCollide;
    bool noObjectGlow;
    bool noRespawnFlash;
    bool noWaveTrail;
    bool iconHack;
    float fontScale;
    float waveTrailSize;
    Mod* getMod() { return Mod::get(); };
    FMODAudioEngine* getAudioEngine() { return FMODAudioEngine::get(); };
    void load() {
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
            ImGui::Begin("Toolbox", &toolbox.uiOpen);
            ImGui::SliderFloat("Font Scale", &toolbox.fontScale, 0.0f, 10.0f, "%.2f");
            ImGui::SetItemTooltip("Scale of Toolbox's text elements.");
            ImGui::SliderFloat("Music Volume", &toolbox.getAudioEngine()->m_musicVolume, 0.0f, 1.0f, "%.2f");
            ImGui::SetItemTooltip("Equivalent to GD's music volume slider.");
            ImGui::SliderFloat("SFX Volume", &toolbox.getAudioEngine()->m_sfxVolume, 0.0f, 1.0f, "%.2f");
            ImGui::SetItemTooltip("Equivalent to GD's SFX volume slider.");
            ImGui::Checkbox("Icon Hack", &toolbox.iconHack);
            ImGui::SetItemTooltip("Bypasses icon & color unlock requirements.");
            ImGui::Checkbox("Noclip", &toolbox.noclip);
            ImGui::SetItemTooltip("Prevents the player from dying.");
            ImGui::Checkbox("No Collide", &toolbox.noCollide);
            ImGui::SetItemTooltip("Disables object collision entirely.");
            ImGui::Checkbox("No Object Glow", &toolbox.noObjectGlow);
            ImGui::SetItemTooltip("Disables object glow.");
            ImGui::Checkbox("No Respawn Flash", &toolbox.noRespawnFlash);
            ImGui::SetItemTooltip("Disables the flash effect upon respawn.");
            ImGui::Checkbox("No Wave Trail", &toolbox.noWaveTrail);
            ImGui::SetItemTooltip("Disables the wave trail.");
            ImGui::SliderFloat("Wave Trail Size", &toolbox.waveTrailSize, 0.0f, 10.0f, "%.2f");
            ImGui::SetItemTooltip("Multiplier for the wave trail's size.");
            if (ImGui::Button("Clear Shaders")) GJBaseGameLayer::get()->m_shaderLayer->resetAllShaders();
            ImGui::SetItemTooltip("Clears all currently running shaders.");
            ImGui::Text(std::format("Toolbox {}", toolbox.getMod()->getVersion().toVString(true)).c_str());
            ImGui::SetItemTooltip(toolbox.currentFunny.c_str());
            ImGui::End();
        };
        toolbox.save();
    });
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
		if (toolbox.noWaveTrail) this->m_waveTrail->reset();
		this->m_waveTrail->m_waveSize=toolbox.waveTrailSize*this->m_vehicleSize;
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
	listenForKeybindSettingPresses("kb-toggle", [](Keybind const& keybind, bool down, bool repeat, double dt) {
		if (down&&!repeat) {
            toolbox.currentFunny=toolbox.funnies[rand()%size(toolbox.funnies)];
            toolbox.uiOpen=!toolbox.uiOpen;
        };
	});
};
