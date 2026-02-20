#pragma once
#include "pch.h"
class TraitConfig
{
    std::string ConfigName;

public:
    TraitConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool Enable = true;
    bool Name = true;
    bool Distance = true;
    ImVec4 TraitColor = ImVec4(0.0f, 1.0f, 0.866666f, 1.0f);
    int FontSize = 13;
    int ToggleKey = 0;

    bool EnableBeastface = true;
    int BeastfaceDistance = 750;

    bool EnableBloodless = true;
    int BloodlessDistance = 750;
    
    bool EnableBulletgrubber = true;
    int BulletgrubberDistance = 750;

    bool EnableConduit = true;
    int ConduitDistance = 750;

    bool EnableDetermination = true;
    int DeterminationDistance = 750;

    bool EnableDoctor = true;
    int DoctorDistance = 750;

    bool EnableFanning = true;
    int FanningDistance = 750;

    bool EnableFastfingers = true;
    int FastfingersDistance = 750;
    
    bool EnableGatorlegs = true;
    int GatorlegsDistance = 750;

    bool EnableGhoul = true;
    int GhoulDistance = 750;

    bool EnableGreyhound = true;
    int GreyhoundDistance = 750;

    bool EnableLevering = true;
    int LeveringDistance = 750;

    bool EnableLightfoot = true;
    int LightfootDistance = 750;

    bool EnableMagpie = true;
    int MagpieDistance = 750;

    bool EnablePackmule = true;
    int PackmuleDistance = 750;

    bool EnablePhysician = true;
    int PhysicianDistance = 750;

    bool EnablePitcher = true;
    int PitcherDistance = 750;

    bool EnableQuartermaster = true;
    int QuartermasterDistance = 750;

    bool EnableResilience = true;
    int ResilienceDistance = 750;

    bool EnableSalveskin = true;
    int SalveskinDistance = 750;

    bool EnableSerpent = true;
    int SerpentDistance = 750;

    bool EnableVigor = true;
    int VigorDistance = 750;

    bool EnableWhispersmith = true;
    int WhispersmithDistance = 750;

    bool EnableWitness = true;
    int WitnessDistance = 750;

    bool EnableBerserker = true;
    int BerserkerDistance = 750;

    bool EnableDeathcheat = true;
    int DeathcheatDistance = 1500;

    bool EnableNecromancer = true;
    int NecromancerDistance = 750;

    bool EnableRampage = true;
    int RampageDistance = 750;

    bool EnableRelentless = true;
    int RelentlessDistance = 750;

    bool EnableRemedy = true;
    int RemedyDistance = 750;

    bool EnableShadow = true;
    int ShadowDistance = 750;

    bool EnableShadowleap = true;
    int ShadowleapDistance = 750;

    bool EnableOther = false;
    int OtherDistance = 750;

    bool EnableBlademancer = true;
    int BlademancerDistance = 1500;

    bool EnableCorpseseer = true;
    int CorpseseerDistance = 1500;

    bool EnableGunrunner = true;
    int GunrunnerDistance = 1500;

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
        j[ConfigName][LIT("FontSize")] = FontSize;
        j[ConfigName][LIT("ToggleKey")] = ToggleKey;
        j[ConfigName][LIT("EnableBeastface")] = EnableBeastface;
        j[ConfigName][LIT("BeastfaceDistance")] = BeastfaceDistance;
        j[ConfigName][LIT("EnableBloodless")] = EnableBloodless;
        j[ConfigName][LIT("BloodlessDistance")] = BloodlessDistance;
        j[ConfigName][LIT("EnableBulletgrubber")] = EnableBulletgrubber;
        j[ConfigName][LIT("BulletgrubberDistance")] = BulletgrubberDistance;
        j[ConfigName][LIT("EnableConduit")] = EnableConduit;
        j[ConfigName][LIT("ConduitDistance")] = ConduitDistance;
        j[ConfigName][LIT("EnableDetermination")] = EnableDetermination;
        j[ConfigName][LIT("DeterminationDistance")] = DeterminationDistance;
        j[ConfigName][LIT("EnableDoctor")] = EnableDoctor;
        j[ConfigName][LIT("DoctorDistance")] = DoctorDistance;
        j[ConfigName][LIT("EnableFanning")] = EnableFanning;
        j[ConfigName][LIT("FanningDistance")] = FanningDistance;
        j[ConfigName][LIT("EnableFastfingers")] = EnableFastfingers;
        j[ConfigName][LIT("FastfingersDistance")] = FastfingersDistance;
        j[ConfigName][LIT("EnableGatorlegs")] = EnableGatorlegs;
        j[ConfigName][LIT("GatorlegsDistance")] = GatorlegsDistance;
        j[ConfigName][LIT("EnableGhoul")] = EnableGhoul;
        j[ConfigName][LIT("GhoulDistance")] = GhoulDistance;
        j[ConfigName][LIT("EnableGreyhound")] = EnableGreyhound;
        j[ConfigName][LIT("GreyhoundDistance")] = GreyhoundDistance;
        j[ConfigName][LIT("EnableLevering")] = EnableLevering;
        j[ConfigName][LIT("LeveringDistance")] = LeveringDistance;
        j[ConfigName][LIT("EnableLightfoot")] = EnableLightfoot;
        j[ConfigName][LIT("LightfootDistance")] = LightfootDistance;
        j[ConfigName][LIT("EnableMagpie")] = EnableMagpie;
        j[ConfigName][LIT("MagpieDistance")] = MagpieDistance;
        j[ConfigName][LIT("EnablePackmule")] = EnablePackmule;
        j[ConfigName][LIT("PackmuleDistance")] = PackmuleDistance;
        j[ConfigName][LIT("EnablePhysician")] = EnablePhysician;
        j[ConfigName][LIT("PhysicianDistance")] = PhysicianDistance;
        j[ConfigName][LIT("EnablePitcher")] = EnablePitcher;
        j[ConfigName][LIT("PitcherDistance")] = PitcherDistance;
        j[ConfigName][LIT("EnableQuartermaster")] = EnableQuartermaster;
        j[ConfigName][LIT("QuartermasterDistance")] = QuartermasterDistance;
        j[ConfigName][LIT("EnableResilience")] = EnableResilience;
        j[ConfigName][LIT("ResilienceDistance")] = ResilienceDistance;
        j[ConfigName][LIT("EnableSalveskin")] = EnableSalveskin;
        j[ConfigName][LIT("SalveskinDistance")] = SalveskinDistance;
        j[ConfigName][LIT("EnableSerpent")] = EnableSerpent;
        j[ConfigName][LIT("SerpentDistance")] = SerpentDistance;
        j[ConfigName][LIT("EnableVigor")] = EnableVigor;
        j[ConfigName][LIT("VigorDistance")] = VigorDistance;
        j[ConfigName][LIT("EnableWhispersmith")] = EnableWhispersmith;
        j[ConfigName][LIT("WhispersmithDistance")] = WhispersmithDistance;
        j[ConfigName][LIT("EnableWitness")] = EnableWitness;
        j[ConfigName][LIT("WitnessDistance")] = WitnessDistance;
        j[ConfigName][LIT("EnableBerserker")] = EnableBerserker;
        j[ConfigName][LIT("BerserkerDistance")] = BerserkerDistance;
        j[ConfigName][LIT("EnableDeathcheat")] = EnableDeathcheat;
        j[ConfigName][LIT("DeathcheatDistance")] = DeathcheatDistance;
        j[ConfigName][LIT("EnableNecromancer")] = EnableNecromancer;
        j[ConfigName][LIT("NecromancerDistance")] = NecromancerDistance;
        j[ConfigName][LIT("EnableRampage")] = EnableRampage;
        j[ConfigName][LIT("RampageDistance")] = RampageDistance;
        j[ConfigName][LIT("EnableRelentless")] = EnableRelentless;
        j[ConfigName][LIT("RelentlessDistance")] = RelentlessDistance;
        j[ConfigName][LIT("EnableRemedy")] = EnableRemedy;
        j[ConfigName][LIT("RemedyDistance")] = RemedyDistance;
        j[ConfigName][LIT("EnableShadow")] = EnableShadow;
        j[ConfigName][LIT("ShadowDistance")] = ShadowDistance;
        j[ConfigName][LIT("EnableShadowleap")] = EnableShadowleap;
        j[ConfigName][LIT("ShadowleapDistance")] = ShadowleapDistance;
        j[ConfigName][LIT("EnableBlademancer")] = EnableBlademancer;
        j[ConfigName][LIT("BlademancerDistance")] = BlademancerDistance;
        j[ConfigName][LIT("EnableCorpseseer")] = EnableCorpseseer;
        j[ConfigName][LIT("CorpseseerDistance")] = CorpseseerDistance;
        j[ConfigName][LIT("EnableGunrunner")] = EnableGunrunner;
        j[ConfigName][LIT("GunrunnerDistance")] = GunrunnerDistance;
        j[ConfigName][LIT("EnableOther")] = EnableOther;
        j[ConfigName][LIT("OtherDistance")] = OtherDistance;
        ToJsonColor(&j, LIT("TraitColor"), &TraitColor);

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
        if (j[ConfigName].contains(LIT("ToggleKey")))
            ToggleKey = j[ConfigName][LIT("ToggleKey")];
        if (j[ConfigName].contains(LIT("EnableBeastface")))
            EnableBeastface = j[ConfigName][LIT("EnableBeastface")];
        if (j[ConfigName].contains(LIT("BeastfaceDistance")))
            BeastfaceDistance = j[ConfigName][LIT("BeastfaceDistance")];
        if (j[ConfigName].contains(LIT("EnableBloodless")))
            EnableBloodless = j[ConfigName][LIT("EnableBloodless")];
        if (j[ConfigName].contains(LIT("BloodlessDistance")))
            BloodlessDistance = j[ConfigName][LIT("BloodlessDistance")];
        if (j[ConfigName].contains(LIT("EnableBulletgrubber")))
            EnableBulletgrubber = j[ConfigName][LIT("EnableBulletgrubber")];
        if (j[ConfigName].contains(LIT("BulletgrubberDistance")))
            BulletgrubberDistance = j[ConfigName][LIT("BulletgrubberDistance")];
        if (j[ConfigName].contains(LIT("EnableConduit")))
            EnableConduit = j[ConfigName][LIT("EnableConduit")];
        if (j[ConfigName].contains(LIT("ConduitDistance")))
            ConduitDistance = j[ConfigName][LIT("ConduitDistance")];
        if (j[ConfigName].contains(LIT("EnableDetermination")))
            EnableDetermination = j[ConfigName][LIT("EnableDetermination")];
        if (j[ConfigName].contains(LIT("DeterminationDistance")))
            DeterminationDistance = j[ConfigName][LIT("DeterminationDistance")];
        if (j[ConfigName].contains(LIT("EnableDoctor")))
            EnableDoctor = j[ConfigName][LIT("EnableDoctor")];
        if (j[ConfigName].contains(LIT("DoctorDistance")))
            DoctorDistance = j[ConfigName][LIT("DoctorDistance")];
        if (j[ConfigName].contains(LIT("EnableFanning")))
            EnableFanning = j[ConfigName][LIT("EnableFanning")];
        if (j[ConfigName].contains(LIT("FanningDistance")))
            FanningDistance = j[ConfigName][LIT("FanningDistance")];
        if (j[ConfigName].contains(LIT("EnableFastfingers")))
            EnableFastfingers = j[ConfigName][LIT("EnableFastfingers")];
        if (j[ConfigName].contains(LIT("FastfingersDistance")))
            FastfingersDistance = j[ConfigName][LIT("FastfingersDistance")];
        if (j[ConfigName].contains(LIT("EnableGatorlegs")))
            EnableGatorlegs = j[ConfigName][LIT("EnableGatorlegs")];
        if (j[ConfigName].contains(LIT("GatorlegsDistance")))
            GatorlegsDistance = j[ConfigName][LIT("GatorlegsDistance")];
        if (j[ConfigName].contains(LIT("EnableGhoul")))
            EnableGhoul = j[ConfigName][LIT("EnableGhoul")];
        if (j[ConfigName].contains(LIT("GhoulDistance")))
            GhoulDistance = j[ConfigName][LIT("GhoulDistance")];
        if (j[ConfigName].contains(LIT("EnableGreyhound")))
            EnableGreyhound = j[ConfigName][LIT("EnableGreyhound")];
        if (j[ConfigName].contains(LIT("GreyhoundDistance")))
            GreyhoundDistance = j[ConfigName][LIT("GreyhoundDistance")];
        if (j[ConfigName].contains(LIT("EnableLevering")))
            EnableLevering = j[ConfigName][LIT("EnableLevering")];
        if (j[ConfigName].contains(LIT("LeveringDistance")))
            LeveringDistance = j[ConfigName][LIT("LeveringDistance")];
        if (j[ConfigName].contains(LIT("EnableLightfoot")))
            EnableLightfoot = j[ConfigName][LIT("EnableLightfoot")];
        if (j[ConfigName].contains(LIT("LightfootDistance")))
            LightfootDistance = j[ConfigName][LIT("LightfootDistance")];
        if (j[ConfigName].contains(LIT("EnableMagpie")))
            EnableMagpie = j[ConfigName][LIT("EnableMagpie")];
        if (j[ConfigName].contains(LIT("MagpieDistance")))
            MagpieDistance = j[ConfigName][LIT("MagpieDistance")];
        if (j[ConfigName].contains(LIT("EnablePackmule")))
            EnablePackmule = j[ConfigName][LIT("EnablePackmule")];
        if (j[ConfigName].contains(LIT("PackmuleDistance")))
            PackmuleDistance = j[ConfigName][LIT("PackmuleDistance")];
        if (j[ConfigName].contains(LIT("EnablePhysician")))
            EnablePhysician = j[ConfigName][LIT("EnablePhysician")];
        if (j[ConfigName].contains(LIT("PhysicianDistance")))
            PhysicianDistance = j[ConfigName][LIT("PhysicianDistance")];
        if (j[ConfigName].contains(LIT("EnablePitcher")))
            EnablePitcher = j[ConfigName][LIT("EnablePitcher")];
        if (j[ConfigName].contains(LIT("PitcherDistance")))
            PitcherDistance = j[ConfigName][LIT("PitcherDistance")];
        if (j[ConfigName].contains(LIT("EnableQuartermaster")))
            EnableQuartermaster = j[ConfigName][LIT("EnableQuartermaster")];
        if (j[ConfigName].contains(LIT("QuartermasterDistance")))
            QuartermasterDistance = j[ConfigName][LIT("QuartermasterDistance")];
        if (j[ConfigName].contains(LIT("EnableResilience")))
            EnableResilience = j[ConfigName][LIT("EnableResilience")];
        if (j[ConfigName].contains(LIT("ResilienceDistance")))
            ResilienceDistance = j[ConfigName][LIT("ResilienceDistance")];
        if (j[ConfigName].contains(LIT("EnableSalveskin")))
            EnableSalveskin = j[ConfigName][LIT("EnableSalveskin")];
        if (j[ConfigName].contains(LIT("SalveskinDistance")))
            SalveskinDistance = j[ConfigName][LIT("SalveskinDistance")];
        if (j[ConfigName].contains(LIT("EnableSerpent")))
            EnableSerpent = j[ConfigName][LIT("EnableSerpent")];
        if (j[ConfigName].contains(LIT("SerpentDistance")))
            SerpentDistance = j[ConfigName][LIT("SerpentDistance")];
        if (j[ConfigName].contains(LIT("EnableVigor")))
            EnableVigor = j[ConfigName][LIT("EnableVigor")];
        if (j[ConfigName].contains(LIT("VigorDistance")))
            VigorDistance = j[ConfigName][LIT("VigorDistance")];
        if (j[ConfigName].contains(LIT("EnableWhispersmith")))
            EnableWhispersmith = j[ConfigName][LIT("EnableWhispersmith")];
        if (j[ConfigName].contains(LIT("WhispersmithDistance")))
            WhispersmithDistance = j[ConfigName][LIT("WhispersmithDistance")];
        if (j[ConfigName].contains(LIT("EnableWitness")))
            EnableWitness = j[ConfigName][LIT("EnableWitness")];
        if (j[ConfigName].contains(LIT("WitnessDistance")))
            WitnessDistance = j[ConfigName][LIT("WitnessDistance")];
        if (j[ConfigName].contains(LIT("EnableBerserker")))
            EnableBerserker = j[ConfigName][LIT("EnableBerserker")];
        if (j[ConfigName].contains(LIT("BerserkerDistance")))
            BerserkerDistance = j[ConfigName][LIT("BerserkerDistance")];
        if (j[ConfigName].contains(LIT("EnableDeathcheat")))
            EnableDeathcheat = j[ConfigName][LIT("EnableDeathcheat")];
        if (j[ConfigName].contains(LIT("DeathcheatDistance")))
            DeathcheatDistance = j[ConfigName][LIT("DeathcheatDistance")];
        if (j[ConfigName].contains(LIT("EnableNecromancer")))
            EnableNecromancer = j[ConfigName][LIT("EnableNecromancer")];
        if (j[ConfigName].contains(LIT("NecromancerDistance")))
            NecromancerDistance = j[ConfigName][LIT("NecromancerDistance")];
        if (j[ConfigName].contains(LIT("EnableRampage")))
            EnableRampage = j[ConfigName][LIT("EnableRampage")];
        if (j[ConfigName].contains(LIT("RampageDistance")))
            RampageDistance = j[ConfigName][LIT("RampageDistance")];
        if (j[ConfigName].contains(LIT("EnableRelentless")))
            EnableRelentless = j[ConfigName][LIT("EnableRelentless")];
        if (j[ConfigName].contains(LIT("RelentlessDistance")))
            RelentlessDistance = j[ConfigName][LIT("RelentlessDistance")];
        if (j[ConfigName].contains(LIT("EnableRemedy")))
            EnableRemedy = j[ConfigName][LIT("EnableRemedy")];
        if (j[ConfigName].contains(LIT("RemedyDistance")))
            RemedyDistance = j[ConfigName][LIT("RemedyDistance")];
        if (j[ConfigName].contains(LIT("EnableShadow")))
            EnableShadow = j[ConfigName][LIT("EnableShadow")];
        if (j[ConfigName].contains(LIT("ShadowDistance")))
            ShadowDistance = j[ConfigName][LIT("ShadowDistance")];
        if (j[ConfigName].contains(LIT("EnableShadowleap")))
            EnableShadowleap = j[ConfigName][LIT("EnableShadowleap")];
        if (j[ConfigName].contains(LIT("ShadowleapDistance")))
            ShadowleapDistance = j[ConfigName][LIT("ShadowleapDistance")];
        if (j[ConfigName].contains(LIT("EnableBlademancer")))
            EnableBlademancer = j[ConfigName][LIT("EnableBlademancer")];
        if (j[ConfigName].contains(LIT("BlademancerDistance")))
            BlademancerDistance = j[ConfigName][LIT("BlademancerDistance")];
        if (j[ConfigName].contains(LIT("EnableCorpseseer")))
            EnableCorpseseer = j[ConfigName][LIT("EnableCorpseseer")];
        if (j[ConfigName].contains(LIT("CorpseseerDistance")))
            CorpseseerDistance = j[ConfigName][LIT("CorpseseerDistance")];
        if (j[ConfigName].contains(LIT("EnableGunrunner")))
            EnableGunrunner = j[ConfigName][LIT("EnableGunrunner")];
        if (j[ConfigName].contains(LIT("GunrunnerDistance")))
            GunrunnerDistance = j[ConfigName][LIT("GunrunnerDistance")];
        if (j[ConfigName].contains(LIT("EnableOther")))
            EnableOther = j[ConfigName][LIT("EnableOther")];
        if (j[ConfigName].contains(LIT("OtherDistance")))
            OtherDistance = j[ConfigName][LIT("OtherDistance")];
        FromJsonColor(j, LIT("TraitColor"), &TraitColor);
    }
};

