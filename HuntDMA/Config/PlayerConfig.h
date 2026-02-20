#pragma once
#include "pch.h"
class PlayerConfig
{
	std::string ConfigName;

public:
	PlayerConfig(const std::string& name)
	{
		ConfigName = name;
	}
    bool Enable = true;
    bool Name = true;
	bool Distance = true;
	int MaxDistance = 1500;
    int DeadMaxDistance = 500;
    ImVec4 TextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	int FontSize = 13;
    bool Chams = false;
    int ChamMode = 5;
    bool HP = true;
    bool DrawFriendsHP = false;
    bool ShowDead = false;
    bool DrawFrames = true;
    bool DrawHeadInFrames = true;
    float HeadCircleSize = 2.5f;
    float HeadCircleOffsetX = 0.0f;
    float HeadCircleOffsetY = 0.0f;
    bool DrawHealthBars = true;
    bool Snaplines = false;
    ImVec4 FramesColor = ImVec4(0.988235f, 0.949019f, 0.019607f, 1.0f);
    ImVec4 FriendColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    bool ShowPlayerList = true;
    int PlayerListFontSize = 15;
    ImVec4 PlayerListColor = ImVec4(0.807843f, 0.807843f, 0.807843f, 1.0f);
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
        j[ConfigName][LIT("DeadMaxDistance")] = DeadMaxDistance;
        j[ConfigName][LIT("FontSize")] = FontSize;
        j[ConfigName][LIT("Chams")] = Chams;
        j[ConfigName][LIT("ChamMode")] = ChamMode;
        j[ConfigName][LIT("HP")] = HP;
        j[ConfigName][LIT("DrawFriends")] = DrawFriendsHP;
        j[ConfigName][LIT("ShowDead")] = ShowDead;
        j[ConfigName][LIT("DrawFrames")] = DrawFrames;
        j[ConfigName][LIT("DrawHeadInFrames")] = DrawHeadInFrames;
        j[ConfigName][LIT("HeadCircleSize")] = HeadCircleSize;
        j[ConfigName][LIT("HeadCircleOffsetX")] = HeadCircleOffsetX;
        j[ConfigName][LIT("HeadCircleOffsetY")] = HeadCircleOffsetY;
        j[ConfigName][LIT("DrawHealthBars")] = DrawHealthBars;
        j[ConfigName][LIT("Snaplines")] = Snaplines;
        j[ConfigName][LIT("ShowPlayerList")] = ShowPlayerList;
        j[ConfigName][LIT("PlayerListFontSize")] = PlayerListFontSize;
        j[ConfigName][LIT("ToggleKey")] = ToggleKey;
        ToJsonColor(&j, LIT("TextColor"), &TextColor);
        ToJsonColor(&j, LIT("FramesColor"), &FramesColor);
        ToJsonColor(&j, LIT("FriendColor"), &FriendColor);
        ToJsonColor(&j, LIT("PlayerListColor"), &PlayerListColor);

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
        if (j[ConfigName].contains(LIT("DeadMaxDistance")))
            DeadMaxDistance = j[ConfigName][LIT("DeadMaxDistance")];
        if (j[ConfigName].contains(LIT("DrawFrames")))
            DrawFrames = j[ConfigName][LIT("DrawFrames")];
        if (j[ConfigName].contains(LIT("Chams")))
            Chams = j[ConfigName][LIT("Chams")];
        if (j[ConfigName].contains(LIT("ChamMode")))
            ChamMode = j[ConfigName][LIT("ChamMode")];
        if (j[ConfigName].contains(LIT("HP")))
            HP = j[ConfigName][LIT("HP")];
        if (j[ConfigName].contains(LIT("DrawFriends")))
            DrawFriendsHP = j[ConfigName][LIT("DrawFriends")];
        if (j[ConfigName].contains(LIT("ShowDead")))
            ShowDead = j[ConfigName][LIT("ShowDead")];
        if (j[ConfigName].contains(LIT("DrawHeadInFrames")))
            DrawHeadInFrames = j[ConfigName][LIT("DrawHeadInFrames")];
        if (j[ConfigName].contains(LIT("HeadCircleSize")))
            HeadCircleSize = j[ConfigName][LIT("HeadCircleSize")];
        if (j[ConfigName].contains(LIT("HeadCircleOffsetX")))
            HeadCircleOffsetX = j[ConfigName][LIT("HeadCircleOffsetX")];
        if (j[ConfigName].contains(LIT("HeadCircleOffsetY")))
            HeadCircleOffsetY = j[ConfigName][LIT("HeadCircleOffsetY")];
        if (j[ConfigName].contains(LIT("DrawHealthBars")))
            DrawHealthBars = j[ConfigName][LIT("DrawHealthBars")];
        if (j[ConfigName].contains(LIT("Snaplines")))
            Snaplines = j[ConfigName][LIT("Snaplines")];
        if (j[ConfigName].contains(LIT("ShowPlayerList")))
            ShowPlayerList = j[ConfigName][LIT("ShowPlayerList")];
        if (j[ConfigName].contains(LIT("PlayerListFontSize")))
            PlayerListFontSize = j[ConfigName][LIT("PlayerListFontSize")];
        if (j[ConfigName].contains(LIT("ToggleKey")))
            ToggleKey = j[ConfigName][LIT("ToggleKey")];

        FromJsonColor(j, LIT("FriendColor"), &FriendColor);
        FromJsonColor(j, LIT("FramesColor"), &FramesColor);
        FromJsonColor(j, LIT("TextColor"), &TextColor);
        FromJsonColor(j, LIT("PlayerListColor"), &PlayerListColor);
    }
};

