#include "main.hpp"

#include "UnityEngine/RectTransform_Axis.hpp"
#include "HMUI/Touchable.hpp"
#include "HMUI/ViewController_AnimationType.hpp"
#include "HMUI/ViewController_AnimationDirection.hpp"
#include "questui/shared/BeatSaberUI.hpp"

DEFINE_TYPE(Qubes, GlobalSettings);
DEFINE_TYPE(Qubes, CreationSettings);
DEFINE_TYPE(Qubes, ButtonSettings);
DEFINE_TYPE(Qubes, ModSettings);
DEFINE_TYPE(Qubes, CreditsView);

using namespace QuestUI;

// button and controller names, need to be synchronized with buttons in main.cpp
const std::vector<std::string> buttonNames = { "Side Trigger", "A/X Button", "B/Y Button", "None" };
const std::vector<std::string> controllerNames = { "Left", "Right", "Both" };

#pragma region flowCoordinator
void Qubes::ModSettings::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    // set active in case show in menu is disabled
    for(auto cube : cubeArr) {
        cube->setActive(true);
    }

    if(!firstActivation)
        return;
    
    if(!globalSettings)
        globalSettings = BeatSaberUI::CreateViewController<Qubes::GlobalSettings*>();
    if(!creationSettings)
        creationSettings = BeatSaberUI::CreateViewController<Qubes::CreationSettings*>();
    if(!buttonSettings)
        buttonSettings = BeatSaberUI::CreateViewController<Qubes::ButtonSettings*>();
    if(!credits)
        credits = BeatSaberUI::CreateViewController<Qubes::CreditsView*>();
    showBackButton = true;
    static ConstString title("Qube Settings");
    SetTitle(title, HMUI::ViewController::AnimationType::In);

    ProvideInitialViewControllers(globalSettings, creationSettings, buttonSettings, credits, nullptr);
}

void Qubes::ModSettings::BackButtonWasPressed(HMUI::ViewController* topViewController) {
    parentFlowCoordinator->DismissFlowCoordinator(this, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, false);
    
    // set back to show in menu value
    for(auto cube : cubeArr) {
        cube->setActive(getModConfig().ShowInMenu.GetValue());
    }
}
#pragma endregion

#pragma region globalSettings
void Qubes::GlobalSettings::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(!firstActivation)
        return;
    
    get_gameObject()->AddComponent<HMUI::Touchable*>();
    auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(get_transform());
    // prevent the layout from expanding to the full height of its container
    vertical->set_childControlHeight(false);
    vertical->set_childForceExpandHeight(false);
    vertical->set_spacing(1);
    auto verticalTransform = vertical->get_transform();
    
    BeatSaberUI::CreateText(verticalTransform, "Global Settings")->set_alignment(514);

    AddConfigValueToggle(verticalTransform, getModConfig().ShowInMenu);
    AddConfigValueToggle(verticalTransform, getModConfig().ShowInLevel);
    AddConfigValueToggle(verticalTransform, getModConfig().ReqDirection);
    AddConfigValueToggle(verticalTransform, getModConfig().Debris);
    AddConfigValueIncrementFloat(verticalTransform, getModConfig().RespawnTime, 1, 0.5, 0, 5);
    AddConfigValueIncrementFloat(verticalTransform, getModConfig().Vibration, 1, 0.1, 0, 2);
}
#pragma endregion

#pragma region creationSettings
void Qubes::CreationSettings::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
    defaultCube->setActive(false);
}

void Qubes::CreationSettings::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    defaultCube->setActive(true);
    defaultCube->setMenuActive(true);

    if(!firstActivation)
        return;
    
    get_gameObject()->AddComponent<HMUI::Touchable*>();
    auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(get_transform());
    // prevent the layout from expanding to the full height of its container
    vertical->set_childControlHeight(false);
    vertical->set_childForceExpandHeight(false);
    vertical->set_spacing(1);
    auto verticalTransform = vertical->get_transform();

    BeatSaberUI::CreateText(verticalTransform, "Creation Settings")->set_alignment(514);

    AddConfigValueIncrementFloat(verticalTransform, getModConfig().CreateDist, 1, 0.5, 0, 3);
    AddConfigValueToggle(verticalTransform, getModConfig().CreateRot);

    BeatSaberUI::CreateText(verticalTransform, "Default Cube")->set_alignment(514);
}
#pragma endregion

#pragma region buttonSettings
// makes the paired dropdown menus for controller bindings
void makeDropdowns(UnityEngine::Transform* parent, ConfigUtils::ConfigValue<int>& buttonSetting, ConfigUtils::ConfigValue<int>& controllerSetting) {
    std::vector<StringW> buttonNamesList(buttonNames.begin(), buttonNames.end());
    auto layout = BeatSaberUI::CreateHorizontalLayoutGroup(parent)->get_transform();
    auto d = BeatSaberUI::CreateDropdown(layout, buttonSetting.GetName(), buttonNamesList[buttonSetting.GetValue()], buttonNamesList, [&buttonSetting](StringW value){
        for(int i = 0; i < buttonNames.size(); i++) {
            if(value == buttonNames[i]) {
                buttonSetting.SetValue(i);
                break;
            }
        }
    });
    ((UnityEngine::RectTransform*) d->get_transform())->SetSizeWithCurrentAnchors(UnityEngine::RectTransform::Axis::Horizontal, 27);
    auto p = d->get_transform()->get_parent();
    static ConstString labelName("Label");
    ((UnityEngine::RectTransform*) p->Find(labelName))->set_anchorMax({2, 1});
    p->get_gameObject()->GetComponent<UnityEngine::UI::LayoutElement*>()->set_preferredWidth(48);

    std::vector<StringW> controllerNamesList(controllerNames.begin(), controllerNames.end());
    d = BeatSaberUI::CreateDropdown(layout, controllerSetting.GetName(), controllerNamesList[controllerSetting.GetValue()], controllerNamesList, [&controllerSetting](StringW value){
        for(int i = 0; i < controllerNames.size(); i++) {
            if(controllerNames[i] == value) {
                controllerSetting.SetValue(i);
                break;
            }
        }
    });
    ((UnityEngine::RectTransform*) d->get_transform())->SetSizeWithCurrentAnchors(UnityEngine::RectTransform::Axis::Horizontal, 22);
    p = d->get_transform()->get_parent();
    ((UnityEngine::RectTransform*) p->Find(labelName))->set_anchorMax({2, 1});
    p->get_gameObject()->GetComponent<UnityEngine::UI::LayoutElement*>()->set_preferredWidth(42);
}

void Qubes::ButtonSettings::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(!firstActivation)
        return;

    get_gameObject()->AddComponent<HMUI::Touchable*>();
    auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(get_transform());
    // prevent the layout from expanding to the full height of its container
    vertical->set_childControlHeight(false);
    vertical->set_childForceExpandHeight(false);
    vertical->set_spacing(1);
    auto verticalTransform = vertical->get_transform();

    BeatSaberUI::CreateText(verticalTransform, "Controller Settings")->set_alignment(514);

    makeDropdowns(verticalTransform, getModConfig().BtnMake, getModConfig().CtrlMake);
    makeDropdowns(verticalTransform, getModConfig().BtnEdit, getModConfig().CtrlEdit);
    makeDropdowns(verticalTransform, getModConfig().BtnDel, getModConfig().CtrlDel);
    
    AddConfigValueIncrementFloat(verticalTransform, getModConfig().MoveSpeed, 1, 0.1, 0, 5);
    AddConfigValueIncrementFloat(verticalTransform, getModConfig().RotSpeed, 1, 0.1, 0, 5);
    AddConfigValueToggle(verticalTransform, getModConfig().LeftThumbMove);
}
#pragma endregion

#pragma region CreditsView
void Qubes::CreditsView::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(!firstActivation)
        return;
    TMPro::TextMeshProUGUI * text;
    TMPro::TextMeshProUGUI * text1;
    TMPro::TextMeshProUGUI * text2;
    TMPro::TextMeshProUGUI * text3;
    TMPro::TextMeshProUGUI * text4;
    TMPro::TextMeshProUGUI * text5;
    TMPro::TextMeshProUGUI * text6;
    TMPro::TextMeshProUGUI * space1;
    TMPro::TextMeshProUGUI * space2;
    TMPro::TextMeshProUGUI * space3;

    UnityEngine::UI::VerticalLayoutGroup* layout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(get_transform());
    space1 = QuestUI::BeatSaberUI::CreateText(layout -> get_transform(), "");
    space2 = QuestUI::BeatSaberUI::CreateText(layout -> get_transform(), "");
    space3 = QuestUI::BeatSaberUI::CreateText(layout -> get_transform(), "");
    text4 = QuestUI::BeatSaberUI::CreateText(layout-> get_transform(), "Message speecil#5350 on discord if there are any issues");
    text4 -> set_alignment(TMPro::TextAlignmentOptions::Center);
    text4 -> set_fontStyle(TMPro::FontStyles::Normal);
    text5 = QuestUI::BeatSaberUI::CreateText(layout -> get_transform(), "Qubes v1.3.1");
    text5 -> set_alignment(TMPro::TextAlignmentOptions::Center);
    text5 -> set_fontSize(5.0);
    text5 -> set_fontStyle(TMPro::FontStyles::Normal);
    text5 -> set_color(UnityEngine::Color::get_green());
    text6 = QuestUI::BeatSaberUI::CreateText(layout -> get_transform(), "First ported by Metalit");
    text6 -> set_alignment(TMPro::TextAlignmentOptions::Center);
    text6 -> set_fontSize(4.5);
    text6 -> set_fontStyle(TMPro::FontStyles::Normal);
}
#pragma endregion