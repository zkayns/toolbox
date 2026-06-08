#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <imgui-cocos.hpp>
using namespace geode::prelude;
class Toolbox {
    public:
        bool uiOpen=false;
        bool noclip=false;
        float fontScale=1.0f;
};
Toolbox toolbox;
$on_mod(Loaded) {
    ImGuiCocos::get().setup();
    ImGuiCocos::get().draw([]{
        if (toolbox.uiOpen) {
            ImGui::GetIO().FontGlobalScale=toolbox.fontScale;
            ImGui::Begin("Toolbox", &toolbox.uiOpen);
            ImGui::SliderFloat("Font Scale", &toolbox.fontScale, 0.0f, 10.0f, "%.2f");
            ImGui::SliderFloat("Music Volume", &FMODAudioEngine::get()->m_musicVolume, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("SFX Volume", &FMODAudioEngine::get()->m_sfxVolume, 0.0f, 1.0f, "%.2f");
            ImGui::Checkbox("Noclip", &toolbox.noclip);
            ImGui::Text(std::format("Toolbox {}", Mod::get()->getVersion().toVString(true)).c_str());
            ImGui::End();
        };
    });
};
class $modify(PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (!toolbox.noclip) PlayLayer::destroyPlayer(player, object);
    }
};
$execute {
	listenForKeybindSettingPresses("kb-toggle", [](Keybind const& keybind, bool down, bool repeat, double dt) {
		if (down&&!repeat) toolbox.uiOpen=!toolbox.uiOpen;
	});
};
