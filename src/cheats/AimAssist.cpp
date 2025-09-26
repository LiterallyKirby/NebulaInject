#include "AimAssist.h"

#include <EntityPlayerSP.h>
#include <World.h>
#include <imgui.h>

#include <cmath>
#include <numbers>

#include "../utils/XUtils.h"
#include "AxisAlignedBB.h"
#include "ItemStack.h"
#include "MovingObjectPosition.h"
#include "World.h"

AimAssist::AimAssist(Phantom* phantom)
    : Cheat("AimAssist", "Aims for u, but smoooothly."), phantom(phantom) {
    range = 3.5f;
    fov = 90.0f;
    hSpeed = 10.0f;
    vSpeed = 0.0f;
    onlyOnClick = true;
    center = true;
    dead = false;
    teams = false;
}

static float distance(float x1, float y1, float z1, float x2, float y2,
                      float z2) {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) +
                     (z2 - z1) * (z2 - z1));
}

static float wrapAngleTo180(float angle) {
    while (angle >= 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

static float getAngleDiff(float a, float b) { return wrapAngleTo180(a - b); }

static std::pair<float, float> getRotations(Player* from, Player* to) {
    auto fromPos = from->GetPos();  // Vec3 or equivalent
    auto toPos = to->GetPos();

    float dx = toPos.x - fromPos.x;
    float dy = toPos.y - fromPos.y;
    float dz = toPos.z - fromPos.z;

    float distXZ = std::sqrt(dx * dx + dz * dz);

    float yaw = std::atan2(dz, dx) * 180.0f / std::numbers::pi_v<float> - 90.0f;
    float pitch = -std::atan2(dy, distXZ) * 180.0f / std::numbers::pi_v<float>;

    return {yaw, pitch};
}

static int getDirection(float currentYaw, float targetYaw) {
    float diff = getAngleDiff(targetYaw, currentYaw);
    return (diff > 0) ? 1 : -1;
}

void AimAssist::run(Minecraft* mc) {
    if (XUtils::mouseDeviceID == 0) return;

    Display* dpy = XOpenDisplay(nullptr);
    XUtils::DeviceState mouseState =
        XUtils::getDeviceState(dpy, XUtils::mouseDeviceID);
    XCloseDisplay(dpy);

    if (mouseState.numButtons == 0) {
        XUtils::isDeviceShit = true;
        return;
    } else {
        XUtils::isDeviceShit = false;
    }

    if (!onlyOnClick || mouseState.buttonStates[1]) {
        float closestDistance = range;

        auto* thePlayer = (Player*)Minecraft::GetThePlayer(phantom->getEnv());
        if (!thePlayer) return;

        auto* worldObj = Minecraft::GetTheWorld(phantom->getEnv());
        if (!worldObj) return;

        World world(phantom->getEnv(), worldObj);
        if (!world.isValid()) return;

        auto players = world.GetPlayerEntities(phantom->getEnv());
        if (players.empty()) return;

        Player* closest = nullptr;

        for (auto* pl : players) {
            if (!pl || pl->GetName() == thePlayer->GetName()) continue;

            for (auto* pl : players) {
                if (!pl || pl->GetName() == thePlayer->GetName()) continue;

                if (isInFOV(pl, thePlayer, fov)) {
                    Vec3D plPos = pl->GetPos();
                    Vec3D myPos = thePlayer->GetPos();

                    double dist = plPos.distance(myPos);

                    if (dist < closestDistance) {
                        closestDistance = dist;
                        closest = pl;
                    }
                }
            }

            if (closest) {
                auto rotations = getRotations(thePlayer, closest);

                float targetYaw = rotations.first;
                float targetPitch = rotations.second;

                float currentYaw = wrapAngleTo180(thePlayer->GetRotationYaw());
                float currentPitch =
                    wrapAngleTo180(thePlayer->GetRotationPitch());

                int direction = getDirection(currentYaw, targetYaw);

                thePlayer->SetRotationYaw(
                    thePlayer->GetRotationYaw() +
                    std::min(hSpeed / ImGui::GetIO().Framerate,
                             std::abs(targetYaw - currentYaw)) *
                        (float)direction);

                if (targetPitch > currentPitch) {
                    thePlayer->SetRotationPitch(
                        thePlayer->GetRotationPitch() +
                        (vSpeed / ImGui::GetIO().Framerate));
                    thePlayer->SetRotationPitch(
                        std::min(thePlayer->GetRotationPitch(), targetPitch));
                } else if (targetPitch < currentPitch) {
                    thePlayer->SetRotationPitch(
                        thePlayer->GetRotationPitch() -
                        (vSpeed / ImGui::GetIO().Framerate));
                    thePlayer->SetRotationPitch(
                        std::max(thePlayer->GetRotationPitch(), targetPitch));
                }
            }
        }
    }
}
void AimAssist::renderSettings() {
    ImGui::SliderFloat("range", &range, 0, 6, "%.2f");
    ImGui::SliderFloat("FOV", &fov, 0, 180, "%.1f");
    ImGui::SliderFloat("hSpeed", &hSpeed, 0, 100, "%.2f");
    ImGui::SliderFloat("vSpeed", &vSpeed, 0, 100, "%.2f");
    ImGui::Checkbox("Only while clicking", &onlyOnClick);
    ImGui::Checkbox("Teams Check", &teams);
    ImGui::Checkbox("Center", &center);
    ImGui::Checkbox("Target Dead", &dead);
}

bool AimAssist::isInFOV(Player* entity, Player* player, float fov) {
    auto rotations = getRotations(player, entity);

    float playerYaw = wrapAngleTo180(player->GetRotationYaw()) + 180;
    float targetYaw = wrapAngleTo180(rotations.first) + 180;

    float diff = std::abs(getAngleDiff(playerYaw, targetYaw));
    return diff < fov;
}
