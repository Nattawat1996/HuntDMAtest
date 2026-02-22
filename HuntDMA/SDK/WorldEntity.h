#pragma once
#include "Memory.h"
#include <chrono>
struct RenderNode {
	char pad_01[0x10]; // 0x00(0x10)
	unsigned int rnd_flags; // 0x10(0x04) — was uint64, changed to uint32 to avoid corrupting +0x14
	char pad_02[0x11C]; // 0x14(0x11C)
	unsigned int silhouettes_param; // 0x130(0x04) — moved from 0x2C
};
struct HealthBar {
	char pad_01[0x18]; // 0x00(0x18)
	unsigned int current_hp; // 0x18(0x04)
	unsigned int regenerable_max_hp; // 0x1C(0x04)
	unsigned int current_max_hp; // 0x20(0x04)
};
struct EntityNameStruct {
	char name[100];
};
enum class EntityType : int
{
	Unknown,

	LocalPlayer,
	EnemyPlayer,
	FriendlyPlayer,
	DeadPlayer,

	Butcher,
	Spider,
	Assassin,
	Scrapbeak,
	Rotjaw,
	Hellborn,

	CashRegister,
	GoldCashRegister,

	Pouch,
	Poster,
	WorkbenchUpgrade,
	WorkbenchOil,
	Trait,
	SealedTrait,

	ResupplyStation,
	ExtractionPoint,
	Clue,

	BearTrap,
	TripMine,
	DarksightDynamite,

	OilBarrel,
	GunpowderBarrel,
	BioBarrel,

	AmmoSwapBox,
	SpitzerBullets,
	PoisonBullets,
	HighVelocityBullets,
	IncendiaryBullets,
	DumDumBullets,
	ExplosiveBullets,
	FMJBullets,
	PoisonBolts,
	ExplosiveBolts,
	IncendiaryShells,
	PennyShotShells,
	FlechetteShells,
	SlugShells,

	SupplyBox,
	AmmoCrate,
	SpecialAmmoCrate,
	CompactAmmoPackage,
	MediumAmmoPackage,
	LongAmmoPackage,
	ShotgunAmmoPackage,
	Medkit,

	Event,
	EventBoon,
};
struct Matrix34 {
	// CryEngine Matrix34: 3 rows × 4 cols (row-major)
	// row0 = [m00 m01 m02 m03]  (X-axis + Tx)
	// row1 = [m10 m11 m12 m13]  (Y-axis + Ty)
	// row2 = [m20 m21 m22 m23]  (Z-axis + Tz)
	float m[3][4];

	Matrix34() { memset(m, 0, sizeof(m)); }

	// Transform local bone offset → world space
	Vector3 TransformPoint(const Vector3& p) const {
		return Vector3(
			m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3],
			m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3],
			m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3]
		);
	}
};

// Keep Matrix4x4 alias for compatibility
using Matrix4x4 = Matrix34;

class WorldEntity
{
private:
	uint64_t Class = 0x0;
	uint64_t ClassAddress = 0x0;
	const uint64_t PosOffset = 0x134;
	const uint64_t StringBufferOffset = 0x10;
	const uint64_t ClassPointerOffset = 0x18;
	const uint64_t SlotsPointerOffset = 0xA8;
	const uint64_t SlotOffset = 0x0;
	const uint64_t RenderNodePointerOffset = 0xA0;
	RenderNode Node;
	EntityNameStruct EntityName;
	EntityNameStruct ClassName;

	uint64_t EntityNamePointer = 0x0;
	uint64_t ClassNamePointer = 0x0;
	uint64_t ClassPointer = 0x0;
	uint64_t SlotsPointer = 0x0;
	uint64_t Slot = 0x0;
	uint64_t RenderNodePointer = 0x0;
	Vector3 Position;
	EntityType Type;

	const uint64_t TypeNameOffset1 = 0x20;
	const uint64_t TypeNameOffset2 = 0x8;
	uint64_t TypeNamePointer1 = 0x0;
	uint64_t TypeNamePointer2 = 0x0;
	EntityNameStruct TypeName;

	const uint64_t HpOffset1 = 0x198;
	const uint64_t HpOffset2 = 0x20;
	const uint64_t HpOffset3 = 0xC8;
	const uint64_t HpOffset4 = 0x78;
	const uint64_t HpOffset5 = 0x58;
	uint64_t HpPointer1 = 0x0;
	uint64_t HpPointer2 = 0x0;
	uint64_t HpPointer3 = 0x0;
	uint64_t HpPointer4 = 0x0;
	uint64_t HpPointer5 = 0x0;
	HealthBar Health;

	const uint64_t CharacterInstanceOffset = 0x88;   // Slot -> CCharInstance*
	const uint64_t SkeletonPoseOffset       = 0xC80;  // CCharInstance -> CSkeletonPose (inline)
	const uint64_t BoneArrayBaseOffset      = 0x20;   // CSkeletonPose -> QuatT array base ptr
	const uint64_t BoneArrayAlignOffset     = 0x38;   // CSkeletonPose -> QuatT array align ptr
	const uint64_t DefaultSkeletonOffset    = 0x1C0;  // CCharInstance -> IDefaultSkeleton ptr
	const uint64_t ModelJointsOffset        = 0x8;    // IDefaultSkeleton -> joint name array
	const uint64_t BoneArraySizeOffset      = 0xA0;   // IDefaultSkeleton -> joint count
	const uint64_t BoneStructSize           = 0x1C;   // sizeof(QuatT)
	const uint64_t BonePosOffset            = 0x10;   // QuatT -> Vec3 position
	const uint64_t WorldMatrixOffset        = 0x160;
	static const int MAX_BONES = 15;
	uint32_t BoneCount = 0;
	int BoneIndex[MAX_BONES] = { 0 };
	Vector3 HeadPosition;
	Vector3 BonePositions[MAX_BONES];  // world-space positions, updated every frame
	Vector3 LocalBonePositions[MAX_BONES];  // raw local-space from scatter, temp buffer
	uint64_t HeadBonePtr = 0;  // cached ptr to head QuatT — valid after UpdateBones()
	std::string DebugBoneNames;  // first few bone names from skeleton — for offset debugging
	bool BonesMapped = false;    // true once UpdateBones() has built the bone index map
	Matrix4x4 WorldMatrix;       // entity world transform — for bone local->world transform

	const uint64_t InternalFlagsOffset = 0x8;
	uint32_t InternalFlags = 0x0;
	bool Hidden = false;


	bool Valid = true;

	std::unordered_map<EntityType, std::string> Names = {
		{EntityType::Unknown, "Unknown"},

		{EntityType::LocalPlayer, "LocalPlayer"},
		{EntityType::EnemyPlayer, "EnemyPlayer"},
		{EntityType::FriendlyPlayer, "FriendlyPlayer"},
		{EntityType::DeadPlayer, "DeadPlayer"},

		{EntityType::Butcher, "Butcher"},
		{EntityType::Assassin, "Assassin"},
		{EntityType::Scrapbeak, "Scrapbeak"},
		{EntityType::Spider, "Spider"},
		{EntityType::Rotjaw, "Rotjaw"},
		{EntityType::Hellborn, "Hellborn"},

		{EntityType::CashRegister, "CashRegister"},
		{EntityType::GoldCashRegister, "GoldCashRegister"},

		{EntityType::Pouch, "Pouch"},
		{EntityType::Poster, "Poster"},
		{EntityType::WorkbenchUpgrade, "WorkbenchUpgrade"},
		{EntityType::WorkbenchOil, "WorkbenchOil"},
		{EntityType::Trait, "Trait"},
		{EntityType::SealedTrait, "Sealed Trait"},

		{EntityType::ResupplyStation, "ResupplyStation"},
		{EntityType::ExtractionPoint, "ExtractionPoint"},
		{EntityType::Clue, "Clue"},

		{EntityType::BearTrap, "BearTrap"},
		{EntityType::TripMine, "TripMine"},
		{EntityType::DarksightDynamite, "DarksightDynamite"},

		{EntityType::OilBarrel, "OilBarrel"},
		{EntityType::GunpowderBarrel, "GunpowderBarrel"},
		{EntityType::BioBarrel, "BioBarrel"},

		{EntityType::AmmoSwapBox, "AmmoSwapBox"},
		{EntityType::SpitzerBullets, "SpitzerBullets"},
		{EntityType::PoisonBullets, "PoisonBullets"},
		{EntityType::HighVelocityBullets, "HighVelocityBullets"},
		{EntityType::IncendiaryBullets, "IncendiaryBullets"},
		{EntityType::DumDumBullets, "DumDumBullets"},
		{EntityType::ExplosiveBullets, "ExplosiveBullets"},
		{EntityType::FMJBullets, "FMJBullets"},
		{EntityType::PoisonBolts, "PoisonBolts"},
		{EntityType::ExplosiveBolts, "ExplosiveBolts"},
		{EntityType::IncendiaryShells, "IncendiaryShells"},
		{EntityType::PennyShotShells, "PennyShotShells"},
		{EntityType::FlechetteShells, "FlechetteShells"},
		{EntityType::SlugShells, "SlugShells"},

		{EntityType::SupplyBox, "SupplyBox"},
		{EntityType::AmmoCrate, "AmmoCrate"},
		{EntityType::SpecialAmmoCrate, "SpecialAmmoCrate"},
		{EntityType::CompactAmmoPackage, "CompactAmmoPackage"},
		{EntityType::MediumAmmoPackage, "MediumAmmoPackage"},
		{EntityType::LongAmmoPackage, "LongAmmoPackage"},
		{EntityType::ShotgunAmmoPackage, "ShotgunAmmoPackage"},
		{EntityType::Medkit, "Medkit"},

		{EntityType::Event, "Event"},
		{EntityType::EventBoon, "Event Boon"},
	};
public:
	WorldEntity(uint64_t classptr);
	void SetUp(VMMDLL_SCATTER_HANDLE handle);
	void SetUp1(VMMDLL_SCATTER_HANDLE handle);
	void SetUp2(VMMDLL_SCATTER_HANDLE handle);
	void SetUp3(VMMDLL_SCATTER_HANDLE handle);
	void SetUp4(VMMDLL_SCATTER_HANDLE handle);
	EntityNameStruct GetEntityName() { return EntityName; }
	EntityNameStruct GetEntityClassName() { return ClassName; }
	EntityNameStruct GetTypeName() { return TypeName; }
	std::string CompactTypeName = "";
	Vector3 GetPosition() { return Position; }
	RenderNode GetRenderNode() { return Node; }
	HealthBar GetHealth() { return Health; }
	uint32_t GetInternalFlags() { return InternalFlags; }
	void SetType(EntityType type) { Type = type; }
	EntityType GetType() { return Type; }
	void WriteNode(VMMDLL_SCATTER_HANDLE handle, int colour, bool show);
	void UpdatePosition(VMMDLL_SCATTER_HANDLE handle);
	void UpdateNode(VMMDLL_SCATTER_HANDLE handle);
	void UpdateHealth(VMMDLL_SCATTER_HANDLE handle);
	void UpdateBones();
	void UpdateHeadPosition(VMMDLL_SCATTER_HANDLE handle); // fast scatter read of all bone positions
	Vector3 GetBonePosition(int idx) const { return (idx >= 0 && idx < MAX_BONES) ? BonePositions[idx] : Vector3::Zero(); }
	void UpdateVelocity(); // Compute velocity from position delta

	// Velocity tracking for prediction (public for aimbot access)
	Vector3 Velocity;
	Vector3 PreviousPosition;
	std::chrono::steady_clock::time_point LastPositionTime;
	std::chrono::steady_clock::time_point LastMoveTime;
	bool HasPreviousPosition = false;

	void UpdateExtraction(VMMDLL_SCATTER_HANDLE handle);
	void UpdateClass(VMMDLL_SCATTER_HANDLE handle);
	uint64_t GetClass() { return ClassAddress; }
	bool IsHidden() { return Hidden; }
	void SetHidden(bool isHidden) { Hidden = isHidden; }
	bool GetValid() { return Valid; }
	void SetValid(bool valid) { Valid = valid; }
	std::string GetTypeAsString() { return Names[Type]; };
	bool IsLocalPlayer();
	Vector3 GetHeadPosition() const { return HeadBonePtr != 0 ? BonePositions[0] : HeadPosition; }
	uint32_t GetBoneCount() const { return BoneCount; }
	const std::string& GetDebugBoneNames() const { return DebugBoneNames; }
	const Matrix4x4& GetWorldMatrix() const { return WorldMatrix; }
	void ApplyBoneWorldTransform();  // call after ExecuteReadScatter to convert local→world

	uint64_t SpecCountOffset1 = 0x198;
	uint64_t SpecCountOffset2 = 0x20;
	uint64_t SpecCountOffset3 = 0xD0;
	uint64_t SpecCountOffset4 = 0xE8;
	uint64_t SpecCountOffset5 = 0x330;
	uint64_t SpecCountPointer1 = 0x0;
	uint64_t SpecCountPointer2 = 0x0;
	uint64_t SpecCountPointer3 = 0x0;
	uint64_t SpecCountPointer4 = 0x0;
	int SpecCount = 0;

	static const uint32_t HIDDEN_FLAG = 0x8;
public:
	bool IsValid() const { return Valid; }
};