#include "Reach.h"
#include <iostream>
#include <Minecraft.h>
#include <imgui.h>
#include "EntityPlayerSP.h"
#include "World.h"
#include "ItemStack.h"
#include "AxisAlignedBB.h"
#include "MovingObjectPosition.h"
#include "World.h"
#include <numbers>
#include <algorithm>
#include <limits>
#include <cmath>

// Windows-specific for GetTickCount64()
#ifdef _WIN32
#include <Windows.h>
#else
// Linux/other platforms - you might need to implement GetTickCount64 alternative
#include <chrono>
unsigned long long GetTickCount64() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
#endif

ReachModule::ReachModule(Phantom* phantom)
    : Cheat("Reach", "Extends reach distance and hitbox"), phantom(phantom) {
    enabled = false;
    distance = 3.5f;
    hitboxExpansion = 0.1f;
    onlyWeapon = true;
    onGround = false;
    liquidCheck = false;
    comboMode = false;
    
    // Initialize combo tracking
    lastLocalHit = 0;
    lastTargetHit = 0;
    firstTargetHit = 0;
    targetHitCount = 0;
    inCombo = false;
    
    initialize();
}

ReachModule::~ReachModule() {
    cleanup();
}

void ReachModule::initialize() {
    std::cout << "[ReachModule] Initialized" << std::endl;
}

void ReachModule::cleanup() {
    std::cout << "[ReachModule] Cleaned up" << std::endl;
}

void ReachModule::run(Minecraft* mc) {
    if (!mc) return;
    if (!enabled) return;
    if (!phantom) return;

    JNIEnv* env = phantom->getEnv();
    if (!env) return;

    jobject playerObj = Minecraft::GetThePlayer(env);
    if (!playerObj) return;

    Player thePlayer(env, playerObj);
    
    // Get world and object mouse over
    jobject worldObj = mc->GetTheWorld(env);
    if (!worldObj) return;
    
    // Apply condition checks
    if (onGround && !thePlayer.IsOnGround()) {
        return;
    }

    if (onlyWeapon) {
        const auto heldItem = thePlayer.GetHeldItem();
        if (!heldItem || !heldItem->IsWeapon(env)) {
            return;
        }
    }

    if (liquidCheck && thePlayer.IsInWater()) {
        return;
    }

    // Find closest target player
    Player* target = nullptr;
    const Vec3D thePlayerPos = thePlayer.GetPos();
    double minDist = std::numeric_limits<double>::max();

    // Get world and find closest player
    World* world = reinterpret_cast<World*>(worldObj);
    std::vector<Player*> players = world->GetPlayerEntities(env);
    
    for (Player* pl : players) {
        if (env->IsSameObject((jobject)pl, playerObj))
            continue;

        const Vec3D playerPos = pl->GetPos();
        const double distBetween = sqrt(pow(thePlayerPos.x - playerPos.x, 2.0) + 
                                       pow(thePlayerPos.y - playerPos.y, 2.0) + 
                                       pow(thePlayerPos.z - playerPos.z, 2.0));
        
        if (distBetween <= minDist && distBetween <= 6.0) {
            minDist = distBetween;
            target = pl;
        }
    }

    // Combo mode logic
    if (target != nullptr && comboMode) {
        // Track local player hits
        if (thePlayer.GetHurtResistantTime() >= thePlayer.GetMaxHurtResistantTime()) {
            lastLocalHit = GetTickCount64();
        }

        // Get object mouse over for aiming check
        jobject objectMouseOverObj = mc->GetObjectMouseOver(env); // You may need to implement this
        if (objectMouseOverObj) {
            MovingObjectPosition* objectMouseOver = reinterpret_cast<MovingObjectPosition*>(objectMouseOverObj);
            
            // Track target hits
            if (target->GetHurtResistantTime() >= target->GetMaxHurtResistantTime() && 
                objectMouseOver->IsAimingEntity(env)) {
                if (targetHitCount == 0) {
                    firstTargetHit = GetTickCount64();
                }
                lastTargetHit = GetTickCount64();
                targetHitCount++;
            }
        }

        // Determine if in combo
        if (targetHitCount >= 2 && lastLocalHit < firstTargetHit && firstTargetHit != 0) {
            inCombo = true;
        }

        // Reset combo state
        if ((inCombo && lastLocalHit > firstTargetHit) || 
            (inCombo && GetTickCount64() - lastTargetHit > 1500)) {
            inCombo = false;
            lastLocalHit = 0;
            firstTargetHit = 0;
            targetHitCount = 0;
            lastTargetHit = 0;
        }
    }

    // Skip if in combo mode and currently in combo
    if (comboMode && inCombo) {
        return;
    }

    // Apply reach modifications to target
    if (target != nullptr) {
        const Vec3D playerPos = target->GetPos();
        const double distBetween = thePlayerPos.distance(playerPos);
        
        // Distance and hitbox checks
        if ((distBetween > distance + 0.5 || distBetween <= 3.0) && hitboxExpansion == 0.0f) {
            return;
        }
        
        float x = static_cast<float>(playerPos.x);
        float z = static_cast<float>(playerPos.z);

        const float hypothenuseDistance = hypotf(static_cast<float>(thePlayerPos.x - playerPos.x), 
                                                static_cast<float>(thePlayerPos.z - playerPos.z));
        float dist = distance - 3.0f;

        while (distBetween > 3.0 && distBetween < (dist + 3.0) && dist > 0.05f) {
            dist -= 0.05f;
        }

        if (hypothenuseDistance < dist) {
            dist -= hypothenuseDistance;
        }

        // Calculate angle
        auto GetAngle = [](float ex, float ez, Vec3D mypos) -> float {
            const float dx = ex - static_cast<float>(mypos.x);
            const float dz = ez - static_cast<float>(mypos.z);

            float angle = static_cast<float>(-atanf(dx / dz) * 180.0f / std::numbers::pi_v<float>);
            
            if (dz < 0.0f && dx < 0.0f) {
                angle = 90.0f + static_cast<float>(atanf(dz / dx) * 180.0f / std::numbers::pi_v<float>);
            } else if (dz < 0.0f && dx > 0.0f) {
                angle = -90.0f + static_cast<float>(atanf(dz / dx) * 180.0f / std::numbers::pi_v<float>);
            }
            return angle;
        };

        const float angle = GetAngle(x, z, thePlayerPos);
        const float ax = cosf((angle + 90.0f) * std::numbers::pi_v<float> / 180.0f);
        const float bx = sinf((angle + 90.0f) * std::numbers::pi_v<float> / 180.0f);

        x = static_cast<float>(playerPos.x);
        z = static_cast<float>(playerPos.z);

        x -= ax * dist;
        z -= bx * dist;

        // Modify hitbox
        const float newWidth = (0.6f + hitboxExpansion) / 2.0f;
        
        // Get and modify the bounding box
        jobject boundingBoxObj = target->GetBoundingBox();
        if (boundingBoxObj) {
            AxisAlignedBB* boundingBox = reinterpret_cast<AxisAlignedBB*>(boundingBoxObj);
            const AxisAlignedBB_t curHitbox = boundingBox->GetNativeBoundingBox(env);

            AxisAlignedBB_t bb{};
            bb.minX = x - newWidth;
            bb.minY = curHitbox.minY;
            bb.minZ = z - newWidth;
            bb.maxX = x + newWidth;
            bb.maxY = curHitbox.maxY;
            bb.maxZ = z + newWidth;

            boundingBox->SetNativeBoundingBox(bb, env);
        }
    }
    
    // Clean up allocated players
    for (Player* pl : players) {
        if (pl != target) { // Don't delete the target we might still be using
            delete pl;
        }
    }
}

void ReachModule::renderSettings() {
    ImGui::Text("Reach Module Settings");
    ImGui::Separator();

    ImGui::Checkbox("Enabled", &enabled);
    
    ImGui::SliderFloat("Distance", &distance, 3.0f, 10.0f, "%.1f");
    ImGui::SliderFloat("Hitbox Expansion", &hitboxExpansion, 0.0f, 0.5f, "%.2f");
    
    ImGui::Separator();
    ImGui::Text("Conditions");
    
    ImGui::Checkbox("Only with Weapon", &onlyWeapon);
    ImGui::Checkbox("Only on Ground", &onGround);
    ImGui::Checkbox("Liquid Check", &liquidCheck);
    ImGui::Checkbox("Combo Mode", &comboMode);
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Prevents reach when in combat combo");
    }
}
