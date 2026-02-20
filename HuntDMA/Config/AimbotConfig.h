#pragma once
#include "pch.h"

class AimbotConfig
{
    std::string ConfigName;

public:
    AimbotConfig(const std::string& name) { ConfigName = name; }

    // ── General ───────────────────────────────────────────────────────────
    bool        Enable          = false;
    int         MaxDistance     = 250;
    bool        TargetPlayers   = false;
    int         Priority        = 0;       // 0=Distance, 1=Crosshair, 2=Both
    int         FOV             = 200;     // pixels radius
    int         Aimkey          = 5;       // VK code (5 = RMB)

    // ── KMBox ─────────────────────────────────────────────────────────────
    int         KmboxBaudRate   = 115200;
    int         KmboxDeviceType = 0;       // 0=AutoDetect, 1=Makcu, 2=Standard
    std::string KmboxPort       = "";
    std::string KmboxIP         = "192.168.2.188";
    std::string KmboxNetPort    = "8347";
    std::string KmboxUUID       = "";

    // ── Visual ────────────────────────────────────────────────────────────
    bool        DrawFOV         = false;
    ImVec4      FOVColor        = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    // ── Smoothing & Output ────────────────────────────────────────────────
    float       Smoothing       = 5.0f;   // 1=instant, higher=smoother
    int         UpdateRate      = 10;     // ms between kmbox::move() calls
    float       Stability       = 0.5f;   // sub-pixel deadzone scale

    // ── Axis-Unlock Anti-Detection ────────────────────────────────────────
    bool        AxisLockEnable      = true;
    float       AxisLockThreshold   = 0.15f; // ratio below which movement is axis-locked
    float       AxisNoiseChance     = 0.75f; // probability of injecting noise [0..1]
    float       AxisNoiseMin        = 0.3f;  // minimum perpendicular noise (px)
    float       AxisNoiseMax        = 1.2f;  // maximum perpendicular noise (px)

    // ── Ballistic Prediction ──────────────────────────────────────────────
    bool        Prediction          = false;
    int         WeaponPreset        = 0;     // 0=Custom, 1+ = index into weaponNames[]
    float       BulletSpeed         = 500.0f;// m/s — used only when WeaponPreset==0
    float       PredictionScale     = 1.0f;  // lead strength multiplier
    float       GravityScale        = 1.0f;  // multiplier on 9.81 m/s²

    // ─────────────────────────────────────────────────────────────────────
    //  JSON helpers
    // ─────────────────────────────────────────────────────────────────────
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
        // General
        j[ConfigName][LIT("Enable")]           = Enable;
        j[ConfigName][LIT("MaxDistance")]       = MaxDistance;
        j[ConfigName][LIT("TargetPlayers")]     = TargetPlayers;
        j[ConfigName][LIT("Priority")]          = Priority;
        j[ConfigName][LIT("FOV")]               = FOV;
        j[ConfigName][LIT("Aimkey")]            = Aimkey;
        // KMBox
        j[ConfigName][LIT("KmboxBaudRate")]     = KmboxBaudRate;
        j[ConfigName][LIT("KmboxDeviceType")]   = KmboxDeviceType;
        j[ConfigName][LIT("KmboxPort")]         = KmboxPort;
        j[ConfigName][LIT("KmboxIP")]           = KmboxIP;
        j[ConfigName][LIT("KmboxNetPort")]      = KmboxNetPort;
        j[ConfigName][LIT("KmboxUUID")]         = KmboxUUID;
        // Visual
        j[ConfigName][LIT("DrawFOV")]           = DrawFOV;
        ToJsonColor(&j, LIT("FOVColor"), &FOVColor);
        // Smoothing
        j[ConfigName][LIT("Smoothing")]         = Smoothing;
        j[ConfigName][LIT("UpdateRate")]        = UpdateRate;
        j[ConfigName][LIT("Stability")]         = Stability;
        // Axis-Unlock
        j[ConfigName][LIT("AxisLockEnable")]    = AxisLockEnable;
        j[ConfigName][LIT("AxisLockThreshold")] = AxisLockThreshold;
        j[ConfigName][LIT("AxisNoiseChance")]   = AxisNoiseChance;
        j[ConfigName][LIT("AxisNoiseMin")]      = AxisNoiseMin;
        j[ConfigName][LIT("AxisNoiseMax")]      = AxisNoiseMax;
        // Ballistic
        j[ConfigName][LIT("Prediction")]        = Prediction;
        j[ConfigName][LIT("WeaponPreset")]      = WeaponPreset;
        j[ConfigName][LIT("BulletSpeed")]       = BulletSpeed;
        j[ConfigName][LIT("PredictionScale")]   = PredictionScale;
        j[ConfigName][LIT("GravityScale")]      = GravityScale;
        return j;
    }

    void FromJson(const json& j)
    {
        if (!j.contains(ConfigName)) return;
        auto& c = j[ConfigName];
        // General
        if (c.contains(LIT("Enable")))         Enable         = c[LIT("Enable")];
        if (c.contains(LIT("MaxDistance")))     MaxDistance    = c[LIT("MaxDistance")];
        if (c.contains(LIT("TargetPlayers")))   TargetPlayers  = c[LIT("TargetPlayers")];
        if (c.contains(LIT("Priority")))        Priority       = c[LIT("Priority")];
        if (c.contains(LIT("FOV")))             FOV            = c[LIT("FOV")];
        if (c.contains(LIT("Aimkey")))          Aimkey         = c[LIT("Aimkey")];
        // KMBox
        if (c.contains(LIT("KmboxBaudRate")))   KmboxBaudRate  = c[LIT("KmboxBaudRate")];
        if (c.contains(LIT("KmboxDeviceType"))) KmboxDeviceType= c[LIT("KmboxDeviceType")];
        if (c.contains(LIT("KmboxPort")))       KmboxPort      = c[LIT("KmboxPort")].get<std::string>();
        if (c.contains(LIT("KmboxIP")))         KmboxIP        = c[LIT("KmboxIP")].get<std::string>();
        if (c.contains(LIT("KmboxNetPort")))    KmboxNetPort   = c[LIT("KmboxNetPort")].get<std::string>();
        if (c.contains(LIT("KmboxUUID")))       KmboxUUID      = c[LIT("KmboxUUID")].get<std::string>();
        // Visual
        if (c.contains(LIT("DrawFOV")))         DrawFOV        = c[LIT("DrawFOV")];
        FromJsonColor(j, LIT("FOVColor"), &FOVColor);
        // Smoothing
        if (c.contains(LIT("Smoothing")))       Smoothing      = c[LIT("Smoothing")];
        if (c.contains(LIT("UpdateRate")))      UpdateRate     = c[LIT("UpdateRate")];
        if (c.contains(LIT("Stability")))       Stability      = c[LIT("Stability")];
        // Axis-Unlock
        if (c.contains(LIT("AxisLockEnable")))    AxisLockEnable    = c[LIT("AxisLockEnable")];
        if (c.contains(LIT("AxisLockThreshold"))) AxisLockThreshold = c[LIT("AxisLockThreshold")];
        if (c.contains(LIT("AxisNoiseChance")))   AxisNoiseChance   = c[LIT("AxisNoiseChance")];
        if (c.contains(LIT("AxisNoiseMin")))      AxisNoiseMin      = c[LIT("AxisNoiseMin")];
        if (c.contains(LIT("AxisNoiseMax")))      AxisNoiseMax      = c[LIT("AxisNoiseMax")];
        // Ballistic
        if (c.contains(LIT("Prediction")))      Prediction     = c[LIT("Prediction")];
        if (c.contains(LIT("WeaponPreset")))    WeaponPreset   = c[LIT("WeaponPreset")];
        if (c.contains(LIT("BulletSpeed")))     BulletSpeed    = c[LIT("BulletSpeed")];
        if (c.contains(LIT("PredictionScale"))) PredictionScale= c[LIT("PredictionScale")];
        if (c.contains(LIT("GravityScale")))    GravityScale   = c[LIT("GravityScale")];
    }
};
