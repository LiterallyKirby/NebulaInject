#include "Fastplace.h"
#include <Minecraft.h>
#include <imgui.h>
#include <EntityPlayerSP.h>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include "../utils/XUtils.h"

inline float randFloat(float min, float max) {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

FastPlaceModule::FastPlaceModule(Phantom* phantom)
    : Cheat("Right Clicker", "Fast right-click for block placement"),
      phantom(phantom) {
    blockOnly = true;
    
    // Right-click specific settings
    cps = 8.0f;  // Lower CPS for right-click (more realistic)
    randomizeCps = true;
    cpsVariance = 1.0f;
    jitterMs = 3.0f;
    holdLength = 0.2f;
    holdLengthRandom = 0.08f;
    
    // Timers
    clickTimer = std::make_unique<MSTimer>();
    eventTimer = std::make_unique<MSTimer>();
    
    nextDelay = 0;
    eventDelay = 400;
    nextEventDelay = static_cast<int>(randFloat(0.8f, 1.2f) * (float)eventDelay);
    
    // Spike/drop for natural variation
    dropChance = 0.15f;
    spikeChance = 0.1f;
    spikeMultiplier = 1.3f;
    
    // State tracking
    isHolding = false;
    shouldClick = false;
    
    // X11 display
    xDisplay = nullptr;
    
    initialize();
}

FastPlaceModule::~FastPlaceModule() { cleanup(); }

void FastPlaceModule::initialize() {
    std::cout << "[FastPlaceModule] Initialized" << std::endl;
    
    // Initialize X11 display
    if (!xDisplay) {
        xDisplay = XOpenDisplay(nullptr);
        if (!xDisplay) {
            std::cerr << "[FastPlaceModule] Failed to open X11 display\n";
        }
    }
}

void FastPlaceModule::cleanup() {
    std::cout << "[FastPlaceModule] Cleaned up" << std::endl;
    
    if (xDisplay) {
        XCloseDisplay(xDisplay);
        xDisplay = nullptr;
    }
}

void FastPlaceModule::run(Minecraft* mc) {
    if (!mc || !enabled || !phantom) return;

    // Get JVM and attach thread (GetEnv first, attach only if needed)
    JavaVM* jvm = phantom->getJvm();
    if (!jvm) {
        std::cerr << "[FastPlaceModule] JVM pointer is null\n";
        return;
    }

    JNIEnv* env = nullptr;
    bool attachedHere = false;
    if (jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) {
        if (jvm->AttachCurrentThread((void**)&env, nullptr) != JNI_OK || !env) {
            std::cerr << "[FastPlaceModule] Failed to attach thread to JVM\n";
            return;
        }
        attachedHere = true;
    }

    // Initialize X11 display if needed
    if (!xDisplay) {
        xDisplay = XOpenDisplay(nullptr);
        if (!xDisplay) {
            isHolding = false;
            if (attachedHere) jvm->DetachCurrentThread();
            return;
        }
    }

    // Get mouse state
    XUtils::DeviceState mouseState = XUtils::getDeviceState(xDisplay, XUtils::mouseDeviceID);
    if (mouseState.numButtons <= 2 || mouseState.buttonStates == nullptr) {
        XUtils::isDeviceShit = true;
        isHolding = false;
        if (attachedHere) jvm->DetachCurrentThread();
        return;
    } else {
        XUtils::isDeviceShit = false;
    }

    // Robust right-button detection (handles 0-based vs 1-based and fallback)
    bool rightButtonHeld = false;
    unsigned int mask = 0; // <<--- moved here so it's visible for debug print below

    // Try common indexing
    if (mouseState.numButtons >= 3) {
        // try 0-based index 2 first, then 1-based index 3
        if (mouseState.buttonStates[2]) rightButtonHeld = true;
        else if (mouseState.numButtons > 3 && mouseState.buttonStates[3]) rightButtonHeld = true;
    }

    // Fallback: query pointer mask from X if the above didn't detect a right button
    if (!rightButtonHeld) {
        Window root, child;
        int root_x, root_y, win_x, win_y;
        if (XQueryPointer(xDisplay, DefaultRootWindow(xDisplay), &root, &child,
                          &root_x, &root_y, &win_x, &win_y, &mask)) {
            rightButtonHeld = (mask & Button3Mask);
        }
    }

    // Debugging output (remove later)
    if (false) {
        std::cout << "[FastPlace] numButtons=" << mouseState.numButtons
                  << " btn2=" << (mouseState.numButtons > 2 ? (int)mouseState.buttonStates[2] : 0)
                  << " btn3=" << (mouseState.numButtons>3? (int)mouseState.buttonStates[3] : 0)
                  << " qmask=" << (mask & Button3Mask) << " right=" << rightButtonHeld << "\n";
    }

    // Get the player object
    jobject playerObj = Minecraft::GetThePlayer(env);
    if (!playerObj) {
        isHolding = false;
        if (attachedHere) jvm->DetachCurrentThread();
        return;
    }

    // Create Player wrapper
    Player thePlayer(env, playerObj);

    // Check if we should fast place (only with blocks if blockOnly is enabled)
    bool shouldFastPlace = true;
    if (blockOnly) {
        ItemStack* heldItem = thePlayer.GetHeldItem();
        if (!heldItem) {
            isHolding = false;
            if (attachedHere) jvm->DetachCurrentThread();
            return;
        }

        if (!heldItem->IsBlock(env)) {
            isHolding = false;
            if (attachedHere) jvm->DetachCurrentThread();
            return;
        }
    }

    // Handle right-click holding state
    if (rightButtonHeld && !isHolding.load()) {
        isHolding = true;
        clickTimer->reset();
        shouldClick = true;
        std::cout << "[FastPlaceModule] Started fast placing\n";
    } else if (!rightButtonHeld) {
        isHolding = false;
        shouldClick = false;
        if (attachedHere) jvm->DetachCurrentThread();
        return;
    }

    // Perform fast right-clicks while holding
    if (isHolding && shouldClick && clickTimer->hasTimePassed(nextDelay)) {
        clickTimer->reset();
        updateValues();
        performRightClick();
    }

    // detach if we attached here
    if (attachedHere) jvm->DetachCurrentThread();
}

void FastPlaceModule::renderSettings() {
    ImGui::Text("FastPlace Settings");
    ImGui::Separator();
    
    ImGui::Checkbox("Block Only", &blockOnly);
    ImGui::SameLine();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Only enable fast place when holding blocks");
    }
    
    ImGui::Spacing();
    
    // CPS settings for right-click
    ImGui::Checkbox("Randomize CPS", &randomizeCps);
    if (!randomizeCps) {
        ImGui::SliderFloat("CPS", &cps, 1.0f, 20.0f, "%.1f");
    } else {
        ImGui::SliderFloat("Target CPS", &cps, 1.0f, 20.0f, "%.1f");
        ImGui::SliderFloat("CPS variance", &cpsVariance, 0.0f, 3.0f, "%.2f");
    }
    
    ImGui::SliderFloat("Jitter (ms)", &jitterMs, 0.0f, 10.0f, "%.1f");
    ImGui::SliderFloat("Hold length (fraction)", &holdLength, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Hold randomness", &holdLengthRandom, 0.0f, 0.3f, "%.2f");
    
    ImGui::Spacing();
    
    // Advanced timing variation
    if (ImGui::CollapsingHeader("Timing Variation")) {
        ImGui::SliderFloat("Drop chance", &dropChance, 0.0f, 0.5f, "%.2f");
        ImGui::SliderFloat("Spike chance", &spikeChance, 0.0f, 0.3f, "%.2f");
        ImGui::SliderFloat("Spike multiplier", &spikeMultiplier, 1.0f, 3.0f, "%.2f");
    }
}

void FastPlaceModule::performRightClick() {
    int baseHold = static_cast<int>(nextDelay * holdLength);
    float randFac = randFloat(1.0f - holdLengthRandom, 1.0f + holdLengthRandom);
    int holdTime = static_cast<int>(baseHold * randFac);
    holdTime = std::clamp(holdTime, 10, std::max(10, nextDelay / 2));

    if (jitterMs > 0.0f) {
        int jitter = static_cast<int>(randFloat(0.0f, jitterMs));
        if (jitter > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(jitter));
        }
    }

    // Perform right-click (button 3 for right mouse button)
    std::thread([](int btn, int hold) { 
        XUtils::clickMouseXEvent(btn, hold); 
    }, 3, holdTime).detach();
}

void FastPlaceModule::updateValues() {
    // Ensure CPS is reasonable
    float baseCps = cps;
    if (baseCps <= 0.01f) baseCps = 0.01f;

    // Randomize CPS if requested
    float effectiveCps = baseCps;
    if (randomizeCps) {
        float var = cpsVariance;
        effectiveCps = baseCps + randFloat(-var, var);
        if (effectiveCps < 0.01f) effectiveCps = 0.01f;
    }

    // Convert to delay (ms)
    int computedDelay = static_cast<int>(1000.0f / effectiveCps);

    // Event-based variation: occasional spikes and drops
    if (eventTimer && eventTimer->hasTimePassed(nextEventDelay)) {
        float r = randFloat(0.0f, 1.0f);
        if (r < dropChance) {
            // Drop: increase delay (slower click)
            int extra = static_cast<int>(randFloat(50.0f, 200.0f));
            computedDelay += extra;
        } else if (r < dropChance + spikeChance) {
            // Spike: reduce delay (faster click)
            computedDelay = static_cast<int>(computedDelay / spikeMultiplier);
            if (computedDelay < 1) computedDelay = 1;
        }

        // Schedule next event
        nextEventDelay = static_cast<int>(randFloat(0.8f, 1.2f) * (float)eventDelay);
        eventTimer->reset();
    }

    // Apply jitter to timing
    if (jitterMs > 0.0f) {
        int jitter = static_cast<int>(randFloat(-jitterMs, jitterMs));
        computedDelay += jitter;
    }

    // Clamp to reasonable bounds
    constexpr int MIN_DELAY = 1;
    constexpr int MAX_DELAY = 5000;
    nextDelay = std::clamp(computedDelay, MIN_DELAY, MAX_DELAY);
}
