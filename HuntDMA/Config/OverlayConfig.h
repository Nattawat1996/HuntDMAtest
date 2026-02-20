#pragma once
#include "pch.h"
class OverlayConfig
{
    std::string ConfigName;

public:
    OverlayConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool DrawRadar = true;
    bool RadarDrawSelf = false;
    float RadarScale = 0.6493056f; // Default magic number
    float RadarX = -0.001171875f;  // Default horizontal offset factor
    float RadarY = 0.0090277f;     // Default vertical offset factor
    ImVec4 PlayerRadarColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 EnemyRadarColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    bool ShowFPS = true;
    int FpsFontSize = 17;
    ImVec4 FpsColor = ImVec4(0.564705f, 0.564705f, 0.564705f, 1.0f);
    bool ShowObjectCount = true;
    int ObjectCountFontSize = 15;
    ImVec4 ObjectCountColor = ImVec4(0.564705f, 0.564705f, 0.564705f, 1.0f);
    int CrosshairType = 1;
    int CrosshairSize = 2;
    ImVec4 CrosshairColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

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
        j[ConfigName][LIT("DrawRadar")] = DrawRadar;
        j[ConfigName][LIT("RadarDrawSelf")] = RadarDrawSelf;
        j[ConfigName][LIT("RadarScale")] = RadarScale;
        j[ConfigName][LIT("RadarX")] = RadarX;
        j[ConfigName][LIT("RadarY")] = RadarY;
        j[ConfigName][LIT("ShowFPS")] = ShowFPS;
        j[ConfigName][LIT("FpsFontSize")] = FpsFontSize;
        j[ConfigName][LIT("ShowObjectCount")] = ShowObjectCount;
        j[ConfigName][LIT("ObjectCountFontSize")] = ObjectCountFontSize;
        j[ConfigName][LIT("CrosshairType")] = CrosshairType;
        j[ConfigName][LIT("CrosshairSize")] = CrosshairSize;
        ToJsonColor(&j, LIT("PlayerRadarColor"), &PlayerRadarColor);
        ToJsonColor(&j, LIT("EnemyRadarColor"), &EnemyRadarColor);
        ToJsonColor(&j, LIT("CrosshairColor"), &CrosshairColor);
        ToJsonColor(&j, LIT("FpsColor"), &FpsColor);
        ToJsonColor(&j, LIT("ObjectCountColor"), &ObjectCountColor);

        return j;
    }

    void FromJson(const json& j)
    {
        if (!j.contains(ConfigName))
            return;
        if (j[ConfigName].contains(LIT("DrawRadar")))
            DrawRadar = j[ConfigName][LIT("DrawRadar")];
        if (j[ConfigName].contains(LIT("RadarDrawSelf")))
            RadarDrawSelf = j[ConfigName][LIT("RadarDrawSelf")];
        if (j[ConfigName].contains(LIT("RadarScale")))
            RadarScale = j[ConfigName][LIT("RadarScale")];
        if (j[ConfigName].contains(LIT("RadarX")))
            RadarX = j[ConfigName][LIT("RadarX")];
        if (j[ConfigName].contains(LIT("RadarY")))
            RadarY = j[ConfigName][LIT("RadarY")];
        if (j[ConfigName].contains(LIT("ShowFPS")))
            ShowFPS = j[ConfigName][LIT("ShowFPS")];
        if (j[ConfigName].contains(LIT("FpsFontSize")))
            FpsFontSize = j[ConfigName][LIT("FpsFontSize")];
        if (j[ConfigName].contains(LIT("ShowObjectCount")))
            ShowObjectCount = j[ConfigName][LIT("ShowObjectCount")];
        if (j[ConfigName].contains(LIT("ObjectCountFontSize")))
            ObjectCountFontSize = j[ConfigName][LIT("ObjectCountFontSize")];
        if (j[ConfigName].contains(LIT("CrosshairType")))
            CrosshairType = j[ConfigName][LIT("CrosshairType")];
        if (j[ConfigName].contains(LIT("CrosshairSize")))
            CrosshairSize = j[ConfigName][LIT("CrosshairSize")];
        FromJsonColor(j, LIT("PlayerRadarColor"), &PlayerRadarColor);
        FromJsonColor(j, LIT("EnemyRadarColor"), &EnemyRadarColor);
        FromJsonColor(j, LIT("CrosshairColor"), &CrosshairColor);
        FromJsonColor(j, LIT("FpsColor"), &FpsColor);
        FromJsonColor(j, LIT("ObjectCountColor"), &ObjectCountColor);
    }
};

