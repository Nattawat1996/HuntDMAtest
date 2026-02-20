#pragma once
#include "pch.h"
class SupplyConfig
{
    std::string ConfigName;

public:
    SupplyConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool Enable = true;
    bool Name = true;
    bool Distance = true;
    bool ShowAmmoSwapBox = false;
    bool ShowCompactAmmo = false;
    bool ShowMediumAmmo = false;
    bool ShowLongAmmo = false;
    bool ShowShortgunAmmo = false;
    bool ShowSpecialAmmo = false;
    bool ShowMedkit = false;
    bool ShowSupplyBox = false;
    int MaxDistance = 100;
    ImVec4 TextColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    int FontSize = 12;
    int ToggleKey = 0;

    void ToJsonColor(json* j, const std::string& name, ImVec4* color)
    {
        (*j)[ConfigName][name][LIT("r")] = color->x;
        (*j)[ConfigName][name][LIT("g")] = color->y;
        (*j)[ConfigName][name][LIT("b")] = color->z;
        (*j)[ConfigName][name][LIT("a")] = color->w;
    }

    void FromJsonColor(json j, const std::string& name, ImVec4* color)
    {
        if (j[ConfigName].contains(name))
        {
            color->x = j[ConfigName][name][LIT("r")];
            color->y = j[ConfigName][name][LIT("g")];
            color->z = j[ConfigName][name][LIT("b")];
            color->w = j[ConfigName][name][LIT("a")];
        }
    }

    json ToJson()
    {
        json j;
        j[ConfigName][LIT("Enable")] = Enable;
        j[ConfigName][LIT("Name")] = Name;
        j[ConfigName][LIT("Distance")] = Distance;
        j[ConfigName][LIT("ShowAmmoSwapBox")] = ShowAmmoSwapBox;
        j[ConfigName][LIT("ShowCompactAmmo")] = ShowCompactAmmo;
        j[ConfigName][LIT("ShowMediumAmmo")] = ShowMediumAmmo;
        j[ConfigName][LIT("ShowLongAmmo")] = ShowLongAmmo;
        j[ConfigName][LIT("ShowShortgunAmmo")] = ShowShortgunAmmo;
        j[ConfigName][LIT("ShowSpecialAmmo")] = ShowSpecialAmmo;
        j[ConfigName][LIT("ShowMedkit")] = ShowMedkit;
        j[ConfigName][LIT("ShowSupplyBox")] = ShowSupplyBox;
        j[ConfigName][LIT("MaxDistance")] = MaxDistance;
        j[ConfigName][LIT("FontSize")] = FontSize;
        j[ConfigName][LIT("ToggleKey")] = ToggleKey;
        ToJsonColor(&j, LIT("TextColor"), &TextColor);

        return j;
    }
    void FromJson(const json& j)
    {
        if (!j.contains(ConfigName))
            return;
        if (j[ConfigName].contains(LIT("Enable")))
            Enable = j[ConfigName][LIT("Enable")];
        if (j[ConfigName].contains(LIT("Name")))
            Name = j[ConfigName][LIT("Name")];
        if (j[ConfigName].contains(LIT("Distance")))
            Distance = j[ConfigName][LIT("Distance")];
        if (j[ConfigName].contains(LIT("ShowAmmoSwapBox")))
            ShowAmmoSwapBox = j[ConfigName][LIT("ShowAmmoSwapBox")];
        if (j[ConfigName].contains(LIT("ShowCompactAmmo")))
            ShowCompactAmmo = j[ConfigName][LIT("ShowCompactAmmo")];
        if (j[ConfigName].contains(LIT("ShowMediumAmmo")))
            ShowMediumAmmo = j[ConfigName][LIT("ShowMediumAmmo")];
        if (j[ConfigName].contains(LIT("ShowLongAmmo")))
            ShowLongAmmo = j[ConfigName][LIT("ShowLongAmmo")];
        if (j[ConfigName].contains(LIT("ShowShortgunAmmo")))
            ShowShortgunAmmo = j[ConfigName][LIT("ShowShortgunAmmo")];
        if (j[ConfigName].contains(LIT("ShowSpecialAmmo")))
            ShowSpecialAmmo = j[ConfigName][LIT("ShowSpecialAmmo")];
        if (j[ConfigName].contains(LIT("ShowMedkit")))
            ShowMedkit = j[ConfigName][LIT("ShowMedkit")];
        if (j[ConfigName].contains(LIT("ShowSupplyBox")))
            ShowSupplyBox = j[ConfigName][LIT("ShowSupplyBox")];
        if (j[ConfigName].contains(LIT("FontSize")))
            FontSize = j[ConfigName][LIT("FontSize")];
        if (j[ConfigName].contains(LIT("MaxDistance")))
            MaxDistance = j[ConfigName][LIT("MaxDistance")];
        if (j[ConfigName].contains(LIT("ToggleKey")))
            ToggleKey = j[ConfigName][LIT("ToggleKey")];
        FromJsonColor(j, LIT("TextColor"), &TextColor);
    }
};

