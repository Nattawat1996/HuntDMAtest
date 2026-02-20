#pragma once
#include "pch.h"
class POIConfig
{
    std::string ConfigName;

public:
    POIConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool Enable = true;
    bool Name = true;
    bool Distance = true;
    int MaxDistance = 900;
    bool ShowResupplyStation = true;
    bool ShowExtraction = true;
    bool ShowCashRegisters = false;
    bool ShowPouches = false;
    bool ShowPosters = false;
    bool ShowBlueprints = false;
    bool ShowGunOil = false;
    bool ShowClues = false;
    bool ShowSeasonalDestructibles = false;
    bool ShowBoons = true;
    ImVec4 TextColor = ImVec4(0.0f, 1.0f, 0.866666f, 1.0f);
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
        j[ConfigName][LIT("MaxDistance")] = MaxDistance;
        j[ConfigName][LIT("FontSize")] = FontSize;
        j[ConfigName][LIT("ToggleKey")] = ToggleKey;
        j[ConfigName][LIT("ShowResupplyStation")] = ShowResupplyStation;
        j[ConfigName][LIT("ShowExtraction")] = ShowExtraction;
        j[ConfigName][LIT("ShowCashRegisters")] = ShowCashRegisters;
        j[ConfigName][LIT("ShowPouches")] = ShowPouches;
        j[ConfigName][LIT("ShowPosters")] = ShowPosters;
        j[ConfigName][LIT("ShowBlueprints")] = ShowBlueprints;
        j[ConfigName][LIT("ShowGunOil")] = ShowGunOil;
        j[ConfigName][LIT("ShowClues")] = ShowClues;
        j[ConfigName][LIT("ShowSeasonalDestructibles")] = ShowSeasonalDestructibles;
        j[ConfigName][LIT("ShowBoons")] = ShowBoons;
        ToJsonColor(&j, LIT("TextColor"), &TextColor);
        ToJsonColor(&j, LIT("BoonColor"), &BoonColor);

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
        if (j[ConfigName].contains(LIT("ShowResupplyStation")))
            ShowResupplyStation = j[ConfigName][LIT("ShowResupplyStation")];
        if (j[ConfigName].contains(LIT("ShowExtraction")))
            ShowExtraction = j[ConfigName][LIT("ShowExtraction")];
        if (j[ConfigName].contains(LIT("ShowCashRegisters")))
            ShowCashRegisters = j[ConfigName][LIT("ShowCashRegisters")];
        if (j[ConfigName].contains(LIT("ShowPouches")))
            ShowPouches = j[ConfigName][LIT("ShowPouches")];
        if (j[ConfigName].contains(LIT("ShowPosters")))
            ShowPosters = j[ConfigName][LIT("ShowPosters")];
        if (j[ConfigName].contains(LIT("ShowBlueprints")))
            ShowBlueprints = j[ConfigName][LIT("ShowBlueprints")];
        if (j[ConfigName].contains(LIT("ShowGunOil")))
            ShowGunOil = j[ConfigName][LIT("ShowGunOil")];
        if (j[ConfigName].contains(LIT("ShowClues")))
            ShowClues = j[ConfigName][LIT("ShowClues")];
        if (j[ConfigName].contains(LIT("ShowSeasonalDestructibles")))
            ShowSeasonalDestructibles = j[ConfigName][LIT("ShowSeasonalDestructibles")];
        if (j[ConfigName].contains(LIT("ShowBoons")))
            ShowBoons = j[ConfigName][LIT("ShowBoons")];
        FromJsonColor(j, LIT("TextColor"), &TextColor);
        FromJsonColor(j, LIT("BoonColor"), &BoonColor);
    }
    
    ImVec4 BoonColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f); // Default Magenta
};

