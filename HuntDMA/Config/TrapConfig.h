#pragma once
#include "pch.h"
class TrapConfig
{
    std::string ConfigName;

public:
    TrapConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool Enable = true;
    bool Name = true;
    bool Distance = true;
    int MaxDistance = 75;
    bool ShowBeartrap = true;
    bool ShowTripmines = true;
    bool ShowDarksightDynamite = true;
    bool ShowGunpowderBurrels = false;
    bool ShowOilBurrels = false;
    bool ShowBioBurrels = false;
    ImVec4 TrapColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 BarrelColor = ImVec4(0.960784f, 0.235294f, 0.0f, 1.0f);
    int FontSize = 13;
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
        j[ConfigName][LIT("MaxDistance")] = MaxDistance;
        j[ConfigName][LIT("FontSize")] = FontSize;
        j[ConfigName][LIT("ToggleKey")] = ToggleKey;
        j[ConfigName][LIT("ShowBeartrap")] = ShowBeartrap;
        j[ConfigName][LIT("ShowTripmines")] = ShowTripmines;
        j[ConfigName][LIT("ShowDarksightDynamite")] = ShowDarksightDynamite;
        j[ConfigName][LIT("ShowGunpowderBurrels")] = ShowGunpowderBurrels;
        j[ConfigName][LIT("ShowOilBurrels")] = ShowOilBurrels;
        j[ConfigName][LIT("ShowBioBurrels")] = ShowBioBurrels;
        ToJsonColor(&j, LIT("TrapColor"), &TrapColor);
        ToJsonColor(&j, LIT("BarrelColor"), &BarrelColor);

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
        if (j[ConfigName].contains(LIT("FontSize")))
            FontSize = j[ConfigName][LIT("FontSize")];
        if (j[ConfigName].contains(LIT("MaxDistance")))
            MaxDistance = j[ConfigName][LIT("MaxDistance")];
        if (j[ConfigName].contains(LIT("ToggleKey")))
            ToggleKey = j[ConfigName][LIT("ToggleKey")];
        if (j[ConfigName].contains(LIT("ShowBeartrap")))
            ShowBeartrap = j[ConfigName][LIT("ShowBeartrap")];
        if (j[ConfigName].contains(LIT("ShowTripmines")))
            ShowTripmines = j[ConfigName][LIT("ShowTripmines")];
        if (j[ConfigName].contains(LIT("ShowDarksightDynamite")))
            ShowDarksightDynamite = j[ConfigName][LIT("ShowDarksightDynamite")];
        if (j[ConfigName].contains(LIT("ShowGunpowderBurrels")))
            ShowGunpowderBurrels = j[ConfigName][LIT("ShowGunpowderBurrels")];
        if (j[ConfigName].contains(LIT("ShowOilBurrels")))
            ShowOilBurrels = j[ConfigName][LIT("ShowOilBurrels")];
        if (j[ConfigName].contains(LIT("ShowBioBurrels")))
            ShowBioBurrels = j[ConfigName][LIT("ShowBioBurrels")];
        FromJsonColor(j, LIT("TrapColor"), &TrapColor);
        FromJsonColor(j, LIT("BarrelColor"), &BarrelColor);
    }
};

