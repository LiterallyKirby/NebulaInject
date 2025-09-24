#include "Reach.h"

#include <Minecraft.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numbers>
#include <ostream>

#include "AxisAlignedBB.h"
#include "EntityPlayerSP.h"
#include "ItemStack.h"
#include "MovingObjectPosition.h"
#include "World.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <chrono>
unsigned long long GetTickCount64() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration)
        .count();
}
#endif

ReachModule::ReachModule(Phantom* phantom)
    : Cheat("Reach", "Extends reach distance and hitbox"), phantom(phantom) {
    distance = 3.5f;
    hitboxExpansion = 0.1f;
    onlyWeapon = true;
    onGround = false;
    liquidCheck = false;
    comboMode = false;

    lastLocalHit = 0;
    lastTargetHit = 0;
    firstTargetHit = 0;
    targetHitCount = 0;
    inCombo = false;

    initialize();
}

ReachModule::~ReachModule() { cleanup(); }

void ReachModule::initialize() {
    std::cout << "[ReachModule] Initialized" << std::endl;
}

void ReachModule::cleanup() {
    std::cout << "[ReachModule] Cleaned up" << std::endl;
}


void ReachModule::run(Minecraft* mc) {

    if (!mc || !phantom) return;

    JavaVM* jvm = phantom->getJvm();
    if (!jvm) {
        std::cerr << "[ReachModule] JVM pointer is null\n";
        return;
    }

    JNIEnv* env = nullptr;
    const jint attachRes = jvm->AttachCurrentThread((void**)&env, nullptr);
    if (attachRes != JNI_OK || !env) {
        std::cerr << "[ReachModule] Failed to attach thread to JVM: " << attachRes << "\n";
        return;
    }

    // --- Begin original logic but using the local env variable ---
    jobject playerObj = Minecraft::GetThePlayer(env);
    if (!playerObj) {
        jvm->DetachCurrentThread();
        return;
    }

    Player thePlayer(env, playerObj); // ctor stores its own JavaVM and global ref

    jobject worldObj = mc->GetTheWorld(env);
    if (!worldObj) {
        // Comment out cleanup (DeleteLocalRef)
        /*
        if (playerObj) env->DeleteLocalRef(playerObj);
        */
        jvm->DetachCurrentThread();
        return;
    }

    if (onGround && !thePlayer.IsOnGround()) {
        // Comment out cleanup (DeleteLocalRef)
        /*
        if (playerObj) env->DeleteLocalRef(playerObj);
        if (worldObj) env->DeleteLocalRef(worldObj);
        */
        jvm->DetachCurrentThread();
        return;
    }

    if (onlyWeapon) {
        ItemStack* heldItem = thePlayer.GetHeldItem();
        if (!heldItem) {
            std::cout << "[ReachModule] No held item\n";
            // Comment out cleanup (DeleteLocalRef)
            /*
            if (playerObj) env->DeleteLocalRef(playerObj);
            if (worldObj) env->DeleteLocalRef(worldObj);
            */
            jvm->DetachCurrentThread();
            return;
        }

        std::cout << "[ReachModule] Held Item:" << heldItem << "\n";
        std::cout << "[ReachModule] Held item found, checking weapon...\n";

        if (!heldItem->IsWeapon()) {
            std::cout << "[ReachModule] Held item is NOT a weapon\n";
            // Comment out cleanup (DeleteLocalRef)
            /*
            delete heldItem;
            if (playerObj) env->DeleteLocalRef(playerObj);
            if (worldObj) env->DeleteLocalRef(worldObj);
            */
            jvm->DetachCurrentThread();
            return;
        }
    }

    if (liquidCheck && thePlayer.IsInWater()) {
        // Comment out cleanup (DeleteLocalRef)
        /*
        if (playerObj) env->DeleteLocalRef(playerObj);
        if (worldObj) env->DeleteLocalRef(worldObj);
        */
        jvm->DetachCurrentThread();
        return;
    }

    Player* target = nullptr;
    const Vec3D thePlayerPos = thePlayer.GetPos();
    double minDist = std::numeric_limits<double>::max();

    World* world = reinterpret_cast<World*>(worldObj);
    std::vector<Player*> players = world->GetPlayerEntities(env);

    for (Player* pl : players) {
        if (!pl) continue;

        jobject otherObj = pl->GetJObject();
        if (!otherObj) continue;
        if (env->IsSameObject(otherObj, playerObj)) continue;

        const Vec3D playerPos = pl->GetPos();
        const double distBetween = thePlayerPos.distance(playerPos);

        if (distBetween <= minDist &&
            distBetween <= 10.0) {  // Changed to allow up to 10 blocks
            minDist = distBetween;
            target = pl;
        }
    }

    if (target && comboMode) {
        if (thePlayer.GetHurtResistantTime() >=
            thePlayer.GetMaxHurtResistantTime()) {
            lastLocalHit = GetTickCount64();
        }

        jobject objectMouseOverObj = mc->GetObjectMouseOver(env);
        if (objectMouseOverObj) {
            MovingObjectPosition* objectMouseOver =
                reinterpret_cast<MovingObjectPosition*>(objectMouseOverObj);

            if (target->GetHurtResistantTime() >=
                    target->GetMaxHurtResistantTime() &&
                objectMouseOver->IsAimingEntity(env)) {
                if (targetHitCount == 0) {
                    firstTargetHit = GetTickCount64();
                }
                lastTargetHit = GetTickCount64();
                targetHitCount++;
            }
        }

        if (targetHitCount >= 2 && lastLocalHit < firstTargetHit &&
            firstTargetHit != 0) {
            inCombo = true;
        }

        if ((inCombo && lastLocalHit > firstTargetHit) ||
            (inCombo && GetTickCount64() - lastTargetHit > 1500)) {
            inCombo = false;
            lastLocalHit = 0;
            firstTargetHit = 0;
            targetHitCount = 0;
            lastTargetHit = 0;
        }
    }

    if (comboMode && inCombo) return;

    if (target) {
        const Vec3D playerPos = target->GetPos();
        const double distBetween = thePlayerPos.distance(playerPos);

        if ((distBetween > distance + 0.5 || distBetween <= 3.0) &&
            hitboxExpansion == 0.0f)
            return;

        float x = static_cast<float>(playerPos.x);
        float z = static_cast<float>(playerPos.z);

        const float hypothenuseDistance =
            hypotf(static_cast<float>(thePlayerPos.x - playerPos.x),
                   static_cast<float>(thePlayerPos.z - playerPos.z));

        float dist = distance - 3.0f;
        while (distBetween > 3.0 && distBetween < (dist + 3.0) &&
               dist > 0.05f) {
            dist -= 0.05f;
        }

        if (hypothenuseDistance < dist) {
            dist -= hypothenuseDistance;
        }

        auto GetAngle = [](float ex, float ez, Vec3D mypos) -> float {
            const float dx = ex - static_cast<float>(mypos.x);
            const float dz = ez - static_cast<float>(mypos.z);

            float angle = static_cast<float>(-atanf(dx / dz) * 180.0f /
                                             std::numbers::pi_v<float>);

            if (dz < 0.0f && dx < 0.0f) {
                angle = 90.0f + static_cast<float>(atanf(dz / dx) * 180.0f /
                                                   std::numbers::pi_v<float>);
            } else if (dz < 0.0f && dx > 0.0f) {
                angle = -90.0f + static_cast<float>(atanf(dz / dx) * 180.0f /
                                                    std::numbers::pi_v<float>);
            }
            return angle;
        };

        const float angle = GetAngle(x, z, thePlayerPos);
        const float ax =
            cosf((angle + 90.0f) * std::numbers::pi_v<float> / 180.0f);
        const float bx =
            sinf((angle + 90.0f) * std::numbers::pi_v<float> / 180.0f);

        x = static_cast<float>(playerPos.x) - ax * dist;
        z = static_cast<float>(playerPos.z) - bx * dist;

        const float newWidth = (0.6f + hitboxExpansion) / 2.0f;

        AxisAlignedBB_t curHitbox = target->GetBoundingBox();
        if (curHitbox.minX != 0 || curHitbox.minY != 0 || curHitbox.minZ != 0 ||
            curHitbox.maxX != 0 || curHitbox.maxY != 0 || curHitbox.maxZ != 0) {
            jobject aabbObj = target->GetBoundingBoxJavaObject();
            if (aabbObj) {
                AxisAlignedBB aabb(env, aabbObj);

                AxisAlignedBB_t bb{};
                bb.minX = x - newWidth;
                bb.minY = curHitbox.minY;
                bb.minZ = z - newWidth;
                bb.maxX = x + newWidth;
                bb.maxY = curHitbox.maxY;
                bb.maxZ = z + newWidth;

                aabb.SetNativeBoundingBox(bb);

                // Comment out cleanup (DeleteLocalRef)
                /*
                env->DeleteLocalRef(aabbObj);
                */
            }
        }
    }

    // Comment out deleting players in players vector
    /*
    for (Player* pl : players) {
        if (pl != target) delete pl;
    }
    */
}


void ReachModule::renderSettings() {
    ImGui::Text("Reach Module Settings");
    ImGui::Separator();

    ImGui::SliderFloat("Distance", &distance, 3.0f, 10.0f,
                       "%.1f");  // Allow up to 10 blocks
    ImGui::SliderFloat("Hitbox Expansion", &hitboxExpansion, 0.0f, 0.5f,
                       "%.2f");

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
