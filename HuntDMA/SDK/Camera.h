#pragma once
#include <mutex>
#include <chrono>

// Immutable per-frame snapshot of camera state.
// Written once by SnapForFrame(), then read by WorldToScreen() and aimbot
// without any locking — safe because only the render/aimbot thread reads it.
struct CameraSnapshot {
    Vector3    Position;
    ViewMatrix RenderMatrix;
    ViewMatrix ProjectionMatrix;
};

class Camera
{
private:
	uint64_t CameraBaseOffset        = 0x840;
	uint64_t ViewMatrixOffset        = 0x230;
	uint64_t CameraPosOffset         = 0x2F0;
	uint64_t ProjectionMatrixOffset  = 0x270;
	uint64_t CachedCameraBase        = 0x0;
	std::chrono::steady_clock::time_point LastCameraBaseRefresh =
		std::chrono::steady_clock::time_point::min();

	// ── Scatter intermediate buffers ──────────────────────────────────────
	// VMMDLL scatter writes directly into these. They are NOT read by
	// WorldToScreen or the aimbot — CommitUpdate() copies them under the
	// mutex to the live fields below.
	Vector3    position_buf;
	ViewMatrix rendermatrix_buf;
	ViewMatrix projmatrix_buf;

	// ── Live state (mutex-protected) ──────────────────────────────────────
	mutable std::mutex matrix_mutex;
	Vector3    Position;
	ViewMatrix RenderMatrix;
	ViewMatrix ProjectionMatrix;

	// ── Per-frame render snapshot (render-thread only, no lock needed) ────
	CameraSnapshot RenderSnap;

public:
	// Called inside UpdateCam scatter setup — registers read requests
	// into the intermediate buffers (position_buf / rendermatrix_buf / projmatrix_buf).
	void UpdateCamera(VMMDLL_SCATTER_HANDLE handle);

	// Call AFTER ExecuteReadScatter — atomically commits the buffers to the
	// live Position/RenderMatrix/ProjectionMatrix fields under the mutex.
	void CommitUpdate();

	// Call ONCE per frame, before any WorldToScreen / aimbot call.
	// Copies the live fields (under mutex) into RenderSnap so all reads
	// within a frame see a consistent camera state.
	void SnapForFrame();

	// Thread-safe getter for world-space camera position (used for distance checks).
	Vector3 GetPosition() {
		std::lock_guard<std::mutex> l(matrix_mutex);
		return Position;
	}

	// Reads exclusively from RenderSnap — consistent within a frame,
	// no race condition with UpdateCamera.
	Vector2 WorldToScreen(Vector3 pos, bool clamp = true);
};