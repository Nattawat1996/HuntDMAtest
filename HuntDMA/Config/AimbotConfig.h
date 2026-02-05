#pragma once
#include "pch.h"
class AimbotConfig
{
    std::string ConfigName;

public:
    AimbotConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool Enable = false;
    int MaxDistance = 250;
    bool TargetPlayers = false;
    int Priority = 0;
    int FOV = 200;
    int Aimkey = 5;
    bool DrawFOV = false;
    ImVec4 FOVColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    
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
        j[ConfigName][LIT("MaxDistance")] = MaxDistance;
        j[ConfigName][LIT("TargetPlayers")] = TargetPlayers;
        j[ConfigName][LIT("Priority")] = Priority;
        j[ConfigName][LIT("FOV")] = FOV;
        j[ConfigName][LIT("Aimkey")] = Aimkey;
        j[ConfigName][LIT("DrawFOV")] = DrawFOV;
        ToJsonColor(&j, LIT("FOVColor"), &FOVColor);



        return j;
    }
    void FromJson(const json& j)
    {
        if (!j.contains(ConfigName))
            return;
        if (j[ConfigName].contains(LIT("Enable")))
            Enable = j[ConfigName][LIT("Enable")];
        if (j[ConfigName].contains(LIT("MaxDistance")))
            MaxDistance = j[ConfigName][LIT("MaxDistance")];
        if (j[ConfigName].contains(LIT("TargetPlayers")))
            TargetPlayers = j[ConfigName][LIT("TargetPlayers")];
        if (j[ConfigName].contains(LIT("Priority")))
            Priority = j[ConfigName][LIT("Priority")];
        if (j[ConfigName].contains(LIT("FOV")))
            FOV = j[ConfigName][LIT("FOV")];
        if (j[ConfigName].contains(LIT("Aimkey")))
            Aimkey = j[ConfigName][LIT("Aimkey")];
        if (j[ConfigName].contains(LIT("DrawFOV")))
            DrawFOV = j[ConfigName][LIT("DrawFOV")];
        FromJsonColor(j, LIT("FOVColor"), &FOVColor);
    }
};

