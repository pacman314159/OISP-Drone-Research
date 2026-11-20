#pragma once

// Layer 4 facade for the vector/matrix engine.
//
// The definitions live in Layer 1: "platforms/hal/hal_matrix.h".
// They were moved down on 2026-11-18 because Layer 3 ("src/middleware/ipc.h")
// and Layer 2 ("src/drivers/telemetry/ble_packet.h") both need Vec3/Vec4, and
// a lower layer importing a higher layer is forbidden. Keeping this facade
// means no Layer 4 call site had to change.
//
// Layer 2 and Layer 3 code MUST include "platforms/hal/hal_matrix.h" directly.

#include "platforms/hal/hal_matrix.h"
