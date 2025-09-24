//
// AutoClicker.cpp — cached Display, burst worker, no sword checks
//
#include "AutoClicker.h"
#include <random>
#include <imgui.h>
#include <EntityPlayerSP.h>
#include <unistd.h>  // close

#include <algorithm>
#include <chrono>
#include <thread>

#include "../utils/ImGuiUtils.h"

#include "../utils/XUtils.h"
#include <Minecraft.h>

inline float randFloat(float min, float max) {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}
AutoClicker::AutoClicker()
    : Cheat("AutoClicker", "Clicks 4 u (so ur hand doesn't break)") {
    cps = 12.0f;
    onlyInGame = true;
    mineBlocks = true;

    clickTimer = std::make_unique<MSTimer>();
    eventTimer = std::make_unique<MSTimer>();
    releaseTimer = std::make_unique<MSTimer>();
    burstTimer = std::make_unique<MSTimer>();

    nextDelay = 0;
    eventDelay = 350;
    nextEventDelay =
        static_cast<int>(randFloat(0.8f, 1.2f) * (float)eventDelay);

    dropChance = 0.3f;
    spikeChance = 0.2f;
    spikeMultiplier = 1.15f;

    holdLength = 0.15f;
    holdLengthRandom = 0.05f;
    jitterMs = 2.0f;

    burstMode = false;
    burstClicks = 3;
    burstDelayMs = 150;

    showAdvanced = false;
    randomizeCps = true;
    cpsVariance = 0.1f;

    uinput_fd = -1;
    shouldRelease = false;
    keyPressed = false;
    releaseDelayMs = 50;

    // Initialize cached display and burst worker
    initializeDisplay();
    initializeUInput();

    // Start burst worker
    burstWorkerRunning = true;
    burstThread = std::thread(&AutoClicker::burstWorkerLoop, this);
}

AutoClicker::~AutoClicker() {
    // Stop burst worker
    burstWorkerRunning = false;
    {
        std::lock_guard<std::mutex> lk(burstMutex);
        burstCv.notify_all();
    }
    if (burstThread.joinable()) burstThread.join();

    cleanup();
}

void AutoClicker::initializeDisplay() {
    if (!xDisplay) {
        xDisplay = XOpenDisplay(nullptr);
    }
}

void AutoClicker::initializeUInput() { uinput_fd = -1; }

void AutoClicker::cleanup() {
    if (uinput_fd != -1) {
        ::close(uinput_fd);
        uinput_fd = -1;
    }
    if (xDisplay) {
        XCloseDisplay(xDisplay);
        xDisplay = nullptr;
    }
}

void AutoClicker::reset(Minecraft *mc) {
    (void)mc;
    isHolding = false;
    shouldClick = false;
    isSpiking = false;
    isDropping = false;

    if (clickTimer) clickTimer->reset();
    if (eventTimer) eventTimer->reset();
    if (releaseTimer) releaseTimer->reset();
    if (burstTimer) burstTimer->reset();

    {
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.clear();
    }
}

// add this to AutoClicker.cpp

void AutoClicker::renderSettings() {
    ImGui::Text("AutoClicker");
    ImGui::Separator();

    // General
    ImGui::Checkbox("Only in game", &onlyInGame);
    ImGui::Checkbox("Mine blocks (disable while aiming at blocks)",
                    &mineBlocks);

    ImGui::Spacing();

    // CPS settings
    ImGui::Checkbox("Randomize CPS", &randomizeCps);
    if (!randomizeCps) {
        ImGui::SliderFloat("CPS", &cps, 1.0f, 40.0f, "%.1f");
    } else {
        ImGui::SliderFloat("Target CPS", &cps, 1.0f, 40.0f, "%.1f");
        ImGui::SliderFloat("CPS variance", &cpsVariance, 0.0f, 5.0f, "%.2f");
    }

    ImGui::SliderFloat("Jitter (ms)", &jitterMs, 0.0f, 25.0f, "%.1f");
    ImGui::SliderFloat("Hold length (fraction)", &holdLength, 0.0f, 1.0f,
                       "%.2f");
    ImGui::SliderFloat("Hold length randomness", &holdLengthRandom, 0.0f, 0.5f,
                       "%.2f");

    ImGui::Spacing();

    // Burst
    ImGui::Checkbox("Burst mode", &burstMode);
    if (burstMode) {
        ImGui::SliderInt("Burst clicks", &burstClicks, 2, 50);
        ImGui::SliderInt("Burst delay (ms)", &burstDelayMs, 10, 2000);
    }

    ImGui::Spacing();

    // Spike / Drop behaviour
    if (ImGui::CollapsingHeader("Spike & Drop (advanced)")) {
        ImGui::SliderFloat("Drop chance", &dropChance, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Spike chance", &spikeChance, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Spike multiplier", &spikeMultiplier, 1.0f, 10.0f,
                           "%.2f");
    }

    ImGui::Spacing();

    // Release / misc
    ImGui::SliderInt("Release delay (ms)", &releaseDelayMs, 0, 1000);

    // Optional debug/advanced info
    if (showAdvanced) {
        ImGui::Separator();
        ImGui::Text("Advanced / debug:");
        ImGui::Text("uinput_fd: %d", uinput_fd);
        ImGui::Text("nextDelay (ms): %d", nextDelay);
        ImGui::Text("burstQueue size: %zu", burstQueue.size());
    }
}

void AutoClicker::run(Minecraft *mc) {
    if (!mc) return;

  

    if (!xDisplay) {
        initializeDisplay();
        if (!xDisplay) {
            isHolding = false;
            return;
        }
    }

    XUtils::DeviceState mouseState =
        XUtils::getDeviceState(xDisplay, XUtils::mouseDeviceID);

    if (mouseState.numButtons <= 1 || mouseState.buttonStates == nullptr) {
        XUtils::isDeviceShit = true;
        isHolding = false;
        return;
    } else {
        XUtils::isDeviceShit = false;
    }

    bool leftButtonHeld = false;
    if (mouseState.numButtons > 1) {
        leftButtonHeld = mouseState.buttonStates[1];
    } else if (mouseState.numButtons > 0) {
        leftButtonHeld = mouseState.buttonStates[0];
    }

    if (leftButtonHeld && !isHolding.load()) {
        isHolding = true;
        clickTimer->reset();
        shouldClick = true;
    } else if (!leftButtonHeld) {
        isHolding = false;
        shouldClick = false;
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.clear();
        return;
    }

    if (isHolding && shouldClick && clickTimer->hasTimePassed(nextDelay)) {
        clickTimer->reset();
        updateValues();

        if (burstMode && burstClicks > 1) {
            int baseHold = static_cast<int>(nextDelay * holdLength);
            float randFac = randFloat(1.0f - holdLengthRandom,
                                                  1.0f + holdLengthRandom);
            int holdTime = static_cast<int>(baseHold * randFac);
            holdTime = std::clamp(holdTime, 10, std::max(10, nextDelay / 2));

            requestBurst(burstClicks, 50, holdTime);
            nextDelay = std::max(nextDelay, burstDelayMs);
        } else {
            performClick(mc);
        }
    }
}

void AutoClicker::requestBurst(int clicks, int spacingMs, int holdMs) {
    {
        std::lock_guard<std::mutex> lk(burstMutex);
        burstQueue.push_back({clicks, spacingMs, holdMs});
    }
    burstCv.notify_one();
}

void AutoClicker::burstWorkerLoop() {
    while (burstWorkerRunning) {
        BurstRequest req;
        bool have = false;
        {
            std::unique_lock<std::mutex> lk(burstMutex);
            burstCv.wait(lk, [&]() {
                return !burstQueue.empty() || !burstWorkerRunning;
            });
            if (!burstWorkerRunning) break;
            if (!burstQueue.empty()) {
                req = burstQueue.front();
                burstQueue.erase(burstQueue.begin());
                have = true;
            }
        }
        if (!have) continue;

        for (int i = 0; i < req.clicks && burstWorkerRunning; ++i) {
            performClickImmediate(req.holdMs);
            if (i + 1 < req.clicks) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(req.spacingMs));
            }
        }
    }
}

void AutoClicker::performClickImmediate(int holdMs) {
    XUtils::clickMouseXEvent(1, holdMs);
}

void AutoClicker::performClick(Minecraft *mc) {
    (void)mc;
    int baseHold = static_cast<int>(nextDelay * holdLength);
    float randFac =
        randFloat(1.0f - holdLengthRandom, 1.0f + holdLengthRandom);
    int holdTime = static_cast<int>(baseHold * randFac);
    holdTime = std::clamp(holdTime, 10, std::max(10, nextDelay / 2));

    if (jitterMs > 0.0f) {
        int jitter = static_cast<int>(randFloat(0.0f, jitterMs));
        if (jitter > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(jitter));
    }

    std::thread([](int btn, int hold) { XUtils::clickMouseXEvent(btn, hold); },
                1, holdTime)
        .detach();
}


void AutoClicker::updateValues() {
    // Basic guard: ensure cps sane
    float baseCps = cps;
    if (baseCps <= 0.01f) baseCps = 0.01f;

    // Randomize CPS if requested
    float effectiveCps = baseCps;
    if (randomizeCps) {
        // treat cpsVariance as absolute CPS variance
        float var = cpsVariance;
        effectiveCps = baseCps + randFloat(-var, var);
        if (effectiveCps < 0.01f) effectiveCps = 0.01f;
    }

    // Convert to delay (ms)
    int computedDelay = static_cast<int>(1000.0f / effectiveCps);

    // Event based spike/drop: occasionally make a unusually long or short delay
    if (eventTimer && eventTimer->hasTimePassed(nextEventDelay)) {
        float r = randFloat(0.0f, 1.0f);
        if (r < dropChance) {
            // Drop: significantly increase delay for this click
            int extra = static_cast<int>(randFloat(100.0f, 600.0f)); // 100-600ms extra
            computedDelay += extra;
        } else if (r < dropChance + spikeChance) {
            // Spike: reduce delay for this click (faster bursts)
            computedDelay = static_cast<int>(computedDelay / spikeMultiplier);
            if (computedDelay < 1) computedDelay = 1;
        }

        // schedule next event (randomize slightly)
        nextEventDelay = static_cast<int>(randFloat(0.8f, 1.2f) * (float)eventDelay);
        eventTimer->reset();
    }

    // Jitter: micro-variation to make timing less uniform
    if (jitterMs > 0.0f) {
        int jitter = static_cast<int>(randFloat(-jitterMs, jitterMs));
        computedDelay += jitter;
    }

    // Clamp nextDelay to reasonable bounds
    constexpr int MIN_DELAY = 1;
    constexpr int MAX_DELAY = 10000;
    nextDelay = std::clamp(computedDelay, MIN_DELAY, MAX_DELAY);

    // Release handling: optionally clear shouldRelease after configured delay
    if (shouldRelease && releaseTimer) {
        if (releaseTimer->hasTimePassed(releaseDelayMs)) {
            shouldRelease = false;
            // reset the timer so we don't immediately retrigger
            releaseTimer->reset();
        }
    }
}
