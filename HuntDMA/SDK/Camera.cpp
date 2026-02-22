#include "Pch.h"
#include "Camera.h"
#include "Globals.h"
#include "ConfigUtilities.h"
#include "ESPRenderer.h"

void Camera::UpdateCamera(VMMDLL_SCATTER_HANDLE handle)
{
    const auto now = std::chrono::steady_clock::now();
    if (CachedCameraBase == 0 ||
        LastCameraBaseRefresh == std::chrono::steady_clock::time_point::min() ||
        (now - LastCameraBaseRefresh) >= std::chrono::seconds(2)) {
        CachedCameraBase = TargetProcess.Read<uint64_t>(
            EnvironmentInstance->GetpSystem() + CameraBaseOffset);
        LastCameraBaseRefresh = now;
    }

    if (!CachedCameraBase)
        return;

    // Write into intermediate buffers — NOT into live fields directly.
    // This prevents WorldToScreen from reading a half-written matrix.
    TargetProcess.AddScatterReadRequest(handle, CachedCameraBase + CameraPosOffset,
                                        &position_buf, sizeof(Vector3));
    TargetProcess.AddScatterReadRequest(handle, CachedCameraBase + ViewMatrixOffset,
                                        &rendermatrix_buf, sizeof(ViewMatrix));
    TargetProcess.AddScatterReadRequest(handle, CachedCameraBase + ProjectionMatrixOffset,
                                        &projmatrix_buf, sizeof(ViewMatrix));
}

void Camera::CommitUpdate()
{
    // Atomically promote scatter results into the live camera state.
    std::lock_guard<std::mutex> l(matrix_mutex);
    Position         = position_buf;
    RenderMatrix     = rendermatrix_buf;
    ProjectionMatrix = projmatrix_buf;
}

void Camera::SnapForFrame()
{
    // Called once per render frame before any WorldToScreen / aimbot call.
    // Takes a single consistent snapshot under the mutex.
    std::lock_guard<std::mutex> l(matrix_mutex);
    RenderSnap.Position         = Position;
    RenderSnap.RenderMatrix     = RenderMatrix;
    RenderSnap.ProjectionMatrix = ProjectionMatrix;
}

Vector2 Camera::WorldToScreen(Vector3 pos, bool clamp)
{
    // Read exclusively from the per-frame snapshot — no race with UpdateCamera.
    const CameraSnapshot& cam = RenderSnap;

    if (cam.Position.IsZero())
        return Vector2::Zero();

    // ── View-space transform ──────────────────────────────────────────────
    Vector4 transformed = {
        pos.x * cam.RenderMatrix.matrix[0][0] + pos.y * cam.RenderMatrix.matrix[1][0] +
            pos.z * cam.RenderMatrix.matrix[2][0] + cam.RenderMatrix.matrix[3][0],
        pos.x * cam.RenderMatrix.matrix[0][1] + pos.y * cam.RenderMatrix.matrix[1][1] +
            pos.z * cam.RenderMatrix.matrix[2][1] + cam.RenderMatrix.matrix[3][1],
        pos.x * cam.RenderMatrix.matrix[0][2] + pos.y * cam.RenderMatrix.matrix[1][2] +
            pos.z * cam.RenderMatrix.matrix[2][2] + cam.RenderMatrix.matrix[3][2],
        pos.x * cam.RenderMatrix.matrix[0][3] + pos.y * cam.RenderMatrix.matrix[1][3] +
            pos.z * cam.RenderMatrix.matrix[2][3] + cam.RenderMatrix.matrix[3][3]
    };

    // Behind camera — discard
    if (transformed.z >= 0.0f)
        return Vector2::Zero();

    // ── Projection ────────────────────────────────────────────────────────
    Vector4 projected = {
        transformed.x * cam.ProjectionMatrix.matrix[0][0] +
            transformed.y * cam.ProjectionMatrix.matrix[1][0] +
            transformed.z * cam.ProjectionMatrix.matrix[2][0] +
            transformed.w * cam.ProjectionMatrix.matrix[3][0],
        transformed.x * cam.ProjectionMatrix.matrix[0][1] +
            transformed.y * cam.ProjectionMatrix.matrix[1][1] +
            transformed.z * cam.ProjectionMatrix.matrix[2][1] +
            transformed.w * cam.ProjectionMatrix.matrix[3][1],
        transformed.x * cam.ProjectionMatrix.matrix[0][2] +
            transformed.y * cam.ProjectionMatrix.matrix[1][2] +
            transformed.z * cam.ProjectionMatrix.matrix[2][2] +
            transformed.w * cam.ProjectionMatrix.matrix[3][2],
        transformed.x * cam.ProjectionMatrix.matrix[0][3] +
            transformed.y * cam.ProjectionMatrix.matrix[1][3] +
            transformed.z * cam.ProjectionMatrix.matrix[2][3] +
            transformed.w * cam.ProjectionMatrix.matrix[3][3]
    };

    if (projected.w == 0.0f)
        return Vector2::Zero();

    projected.x /= projected.w;
    projected.y /= projected.w;

    const float limit = clamp ? 1.0f : 1.5f;
    if (std::abs(projected.x) > limit || std::abs(projected.y) > limit)
        return Vector2::Zero();

    float width  = ESPRenderer::GetScreenWidth();
    float height = ESPRenderer::GetScreenHeight();

    return Vector2{
        (1.0f + projected.x) * width  * 0.5f,
        (1.0f - projected.y) * height * 0.5f
    };
}
