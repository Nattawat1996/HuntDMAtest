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
    int KmboxBaudRate = 115200;
    int KmboxDeviceType = 0; // 0=AutoDetect, 1=Makcu, 2=Standard KMBox
    std::string KmboxPort = ""; // Empty = auto-detect, otherwise e.g. "COM3"
    std::string KmboxIP = "192.168.2.188";
    std::string KmboxNetPort = "8347";
    std::string KmboxUUID = "";
    bool DrawFOV = false;
    ImVec4 FOVColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Prediction settings
    bool Prediction = false;
    float BulletSpeed = 500.0f;      // m/s (Hunt rifles ~400-600)
    float Smoothing = 5.0f;          // Aim smoothing (1=instant, higher=smoother)
    int UpdateRate = 10;             // Update delay in ms (lower = smoother/faster)
    int AmmoType = 0;                // 0=Compact, 1=Medium, 2=Long
    float Stability = 0.5f;          // 0.0=No deadzone, 1.0=Large deadzone/damping
    float PredictionScale = 1.0f;    // Fine-tune prediction amount (0.0-2.0)
    float DropRange = 200.0f;        // Distance before bullet drops (m)
    float GravityScale = 1.0f;       // Gravity multiplier
    float DropPower = 2.5f;          // Drop curve exponent (user fitted)
    bool AutoDropPower = true;       // Auto-calculate power from Drop Range
    int WeaponPreset = 0;            // 0=Custom, 1+=weapon index from WeaponPresets[]
    
    
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
        j[ConfigName][LIT("KmboxBaudRate")] = KmboxBaudRate;
        j[ConfigName][LIT("KmboxDeviceType")] = KmboxDeviceType;
        j[ConfigName][LIT("KmboxPort")] = KmboxPort;
        j[ConfigName][LIT("KmboxIP")] = KmboxIP;
        j[ConfigName][LIT("KmboxNetPort")] = KmboxNetPort;
        j[ConfigName][LIT("KmboxUUID")] = KmboxUUID;
        j[ConfigName][LIT("DrawFOV")] = DrawFOV;
        ToJsonColor(&j, LIT("FOVColor"), &FOVColor);
        j[ConfigName][LIT("Prediction")] = Prediction;
        j[ConfigName][LIT("BulletSpeed")] = BulletSpeed;
        j[ConfigName][LIT("Smoothing")] = Smoothing;
        j[ConfigName][LIT("UpdateRate")] = UpdateRate;
        j[ConfigName][LIT("UpdateRate")] = UpdateRate;
        j[ConfigName][LIT("AmmoType")] = AmmoType;
        j[ConfigName][LIT("Stability")] = Stability;
        j[ConfigName][LIT("PredictionScale")] = PredictionScale;
        j[ConfigName][LIT("DropRange")] = DropRange;
        j[ConfigName][LIT("GravityScale")] = GravityScale;
        j[ConfigName][LIT("DropPower")] = DropPower;
        j[ConfigName][LIT("AutoDropPower")] = AutoDropPower;
        j[ConfigName][LIT("WeaponPreset")] = WeaponPreset;


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
        if (j[ConfigName].contains(LIT("KmboxBaudRate")))
            KmboxBaudRate = j[ConfigName][LIT("KmboxBaudRate")];
        if (j[ConfigName].contains(LIT("KmboxDeviceType")))
            KmboxDeviceType = j[ConfigName][LIT("KmboxDeviceType")];
        if (j[ConfigName].contains(LIT("KmboxPort")))
            KmboxPort = j[ConfigName][LIT("KmboxPort")].get<std::string>();
        if (j[ConfigName].contains(LIT("KmboxIP")))
            KmboxIP = j[ConfigName][LIT("KmboxIP")].get<std::string>();
        if (j[ConfigName].contains(LIT("KmboxNetPort")))
            KmboxNetPort = j[ConfigName][LIT("KmboxNetPort")].get<std::string>();
        if (j[ConfigName].contains(LIT("KmboxUUID")))
            KmboxUUID = j[ConfigName][LIT("KmboxUUID")].get<std::string>();
        if (j[ConfigName].contains(LIT("DrawFOV")))
            DrawFOV = j[ConfigName][LIT("DrawFOV")];
        FromJsonColor(j, LIT("FOVColor"), &FOVColor);
        if (j[ConfigName].contains(LIT("Prediction")))
            Prediction = j[ConfigName][LIT("Prediction")];
        if (j[ConfigName].contains(LIT("BulletSpeed")))
            BulletSpeed = j[ConfigName][LIT("BulletSpeed")];
        if (j[ConfigName].contains(LIT("Smoothing")))
            Smoothing = j[ConfigName][LIT("Smoothing")];
        if (j[ConfigName].contains(LIT("UpdateRate")))
            UpdateRate = j[ConfigName][LIT("UpdateRate")];
        if (j[ConfigName].contains(LIT("UpdateRate")))
            UpdateRate = j[ConfigName][LIT("UpdateRate")];
        if (j[ConfigName].contains(LIT("AmmoType")))
            AmmoType = j[ConfigName][LIT("AmmoType")];
        if (j[ConfigName].contains(LIT("Stability")))
            Stability = j[ConfigName][LIT("Stability")];
        if (j[ConfigName].contains(LIT("PredictionScale")))
            PredictionScale = j[ConfigName][LIT("PredictionScale")];
        if (j[ConfigName].contains(LIT("DropRange")))
            DropRange = j[ConfigName][LIT("DropRange")];
        if (j[ConfigName].contains(LIT("GravityScale")))
            GravityScale = j[ConfigName][LIT("GravityScale")];
        if (j[ConfigName].contains(LIT("DropPower")))
            DropPower = j[ConfigName][LIT("DropPower")];
        if (j[ConfigName].contains(LIT("AutoDropPower")))
            AutoDropPower = j[ConfigName][LIT("AutoDropPower")];
        if (j[ConfigName].contains(LIT("WeaponPreset")))
            WeaponPreset = j[ConfigName][LIT("WeaponPreset")];
    }
};

