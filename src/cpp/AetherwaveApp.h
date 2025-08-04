#pragma once

#include "ofMain.h"

namespace Aetherwave {

/**
 * Forward declaration for the main Aetherwave application class.
 * This header provides the interface for the media display engine.
 */
class AetherwaveApp;

// Configuration constants
namespace Config {
    constexpr int DEFAULT_WIDTH = 1920;
    constexpr int DEFAULT_HEIGHT = 1080;
    constexpr int TARGET_FPS = 60;
    constexpr float TRANSITION_DURATION = 2.0f;
}

// Future: Image metadata structures, display modes, transition types

} // namespace Aetherwave
