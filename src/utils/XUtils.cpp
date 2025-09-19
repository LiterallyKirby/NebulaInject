//
// Created by somepineaple on 1/31/22.
//

#include "XUtils.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <imgui.h>

#include "ImGuiUtils.h"

int XUtils::mouseDeviceIndex = 0;
int XUtils::keyboardDeviceIndex = 0;
unsigned long XUtils::mouseDeviceID = 0;
unsigned long XUtils::keyboardDeviceID = 0;
bool XUtils::isDeviceShit = false;

XUtils::DeviceState XUtils::getDeviceState(Display *display, unsigned long deviceID) {
    DeviceState state{};

    // Validate inputs first
    if (!display || deviceID == 0) {
        return state; // Return empty state
    }

    XDevice *device = XOpenDevice(display, deviceID);
    if (!device) {
        // Device couldn't be opened - invalid device ID
        return state;
    }

    XDeviceState *xState = XQueryDeviceState(display, device);
    if (!xState) {
        // Failed to query device state
        XCloseDevice(display, device);
        return state;
    }

    XValuatorState *valState;
    XButtonState *buttonState;
    XKeyState *keyState;

    XInputClass *cls = xState->data;

    for (int i = 0; i < xState->num_classes; i++) {
        if (!cls) break; // Safety check
        
        int i2;
        switch(cls->c_class) {
        case ValuatorClass:
            valState = (XValuatorState *) cls;
            if (valState->num_valuators > 0) {
                state.numValuators = valState->num_valuators;
                state.valuatorStates = (int *) malloc(state.numValuators * sizeof(int));
                if (state.valuatorStates) {
                    for (i2 = 0; i2 < valState->num_valuators; i2++)
                        state.valuatorStates[i2] = valState->valuators[i2];
                }
            }
            break;
        case ButtonClass:
            buttonState = (XButtonState *) cls;
            if (buttonState->num_buttons > 0) {
                state.numButtons = buttonState->num_buttons;
                state.buttonStates = (bool *) malloc(state.numButtons * sizeof(bool));
                if (state.buttonStates) {
                    // Initialize all buttons to false first
                    memset(state.buttonStates, 0, state.numButtons * sizeof(bool));
                    // Fix: Start from 0, not 1, and add bounds checking
                    for (i2 = 0; i2 < buttonState->num_buttons; i2++) {
                        int byte_index = i2 / 8;
                        int bit_index = i2 % 8;
                        if (byte_index < (buttonState->num_buttons + 7) / 8) {
                            state.buttonStates[i2] = (buttonState->buttons[byte_index] & (1 << bit_index)) != 0;
                        }
                    }
                }
            }
            break;
        case KeyClass:
            keyState = (XKeyState *) cls;
            if (keyState->num_keys > 0) {
                state.numKeys = keyState->num_keys;
                state.keyStates = (bool *) malloc(state.numKeys * sizeof(bool));
                if (state.keyStates) {
                    // Initialize all keys to false first
                    memset(state.keyStates, 0, state.numKeys * sizeof(bool));
                    for (i2 = 0; i2 < keyState->num_keys; i2++) {
                        int byte_index = i2 / 8;
                        int bit_index = i2 % 8;
                        if (byte_index < (keyState->num_keys + 7) / 8) {
                            state.keyStates[i2] = (keyState->keys[byte_index] & (1 << bit_index)) != 0;
                        }
                    }
                }
            }
            break;
        }
        cls = (XInputClass *) ((char *) cls + cls->length);
    }
    
    XFreeDeviceState(xState);
    XCloseDevice(display, device);

    return state;
}

void XUtils::clickMouseXEvent(int button, long delayMS) {
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    
    XButtonEvent event;
    memset(&event, 0, sizeof(event));
    event.button = button;
    event.same_screen = true;
    event.subwindow = DefaultRootWindow(dpy);

    while (event.subwindow) {
        event.window = event.subwindow;
        XQueryPointer(dpy, event.window,
                      &event.root, &event.subwindow,
                      &event.x_root, &event.y_root,
                      &event.x, &event.y,
                      &event.state);
    }

    event.type = ButtonPress;
    XSendEvent(dpy, PointerWindow, True, ButtonPressMask, (XEvent *)&event);
    XFlush(dpy);
    usleep(delayMS * 1000);
    event.type = ButtonRelease;
    XSendEvent(dpy, PointerWindow, True, ButtonReleaseMask, (XEvent*)&event);
    XFlush(dpy);
    XCloseDisplay(dpy);
}

void XUtils::renderMouseSelector() {
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    
    XDeviceInfo *devices;
    int numDevices;
    devices = XListInputDevices(dpy, &numDevices);
    
    if (!devices) {
        XCloseDisplay(dpy);
        return;
    }

    if (isDeviceShit)
        ImGui::Text("Please select a valid mouse device");
    else
        ImGui::Text("Valid Mouse Selected");

    std::string comboItems;
    for (int i = 0; i < numDevices; i++) {
        if (devices[i].name) {
            comboItems.append(devices[i].name);
            comboItems.push_back('\0');
        }
    }

    if (ImGui::Combo("Select your mouse", &mouseDeviceIndex, comboItems.c_str())) {
        if (mouseDeviceIndex >= 0 && mouseDeviceIndex < numDevices) {
            mouseDeviceID = devices[mouseDeviceIndex].id;
        }
    }

    ImGui::SameLine();
    ImGuiUtils::drawHelper("You must select the mouse you use to click with in minecraft. If you don't, autoclicker and aimassist won't work");

    XFreeDeviceList(devices);
    XCloseDisplay(dpy);
}

void XUtils::renderKeyboardSelector() {
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    
    XDeviceInfo *devices;
    int numDevices;
    devices = XListInputDevices(dpy, &numDevices);
    
    if (!devices) {
        XCloseDisplay(dpy);
        return;
    }

    std::string comboItems;
    for (int i = 0; i < numDevices; i++) {
        if (devices[i].name) {
            comboItems.append(devices[i].name);
            comboItems.push_back('\0');
        }
    }

    if (ImGui::Combo("Select your keyboard", &keyboardDeviceIndex, comboItems.c_str())) {
        if (keyboardDeviceIndex >= 0 && keyboardDeviceIndex < numDevices) {
            keyboardDeviceID = devices[keyboardDeviceIndex].id;
        }
    }

    ImGui::SameLine();
    ImGuiUtils::drawHelper("You must select you keyboard, or else keybindings won't work");

    XFreeDeviceList(devices);
    XCloseDisplay(dpy);
}

XUtils::DeviceState::DeviceState() {
    numKeys = 0;
    numButtons = 0;
    numValuators = 0;
    keyStates = nullptr;
    buttonStates = nullptr;
    valuatorStates = nullptr;
}

XUtils::DeviceState::~DeviceState() {
    if (keyStates != nullptr)
        free(keyStates);
    if (buttonStates != nullptr)
        free(buttonStates);
    if (valuatorStates != nullptr)
        free(valuatorStates);
}
