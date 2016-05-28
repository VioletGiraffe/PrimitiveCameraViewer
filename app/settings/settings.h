#pragma once

#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QString>
RESTORE_COMPILER_WARNINGS

#define CAMERA_NAME_FILTER_SETTING QStringLiteral("Camera/NameFilter")

#define IMAGE_PIXEL_VALUE_THRESHOLD_SETTING QStringLiteral("Camera/PixelValueThreshold")
#define IMAGE_PIXEL_VALUE_THRESHOLD_DEFAULT 20

#define IMAGE_VERTICAL_MIRROR_SETTING QStringLiteral("Camera/MirrorVertically")
#define IMAGE_HORIZONTAL_MIRROR_SETTING QStringLiteral("Camera/MirrorHorizontally")
