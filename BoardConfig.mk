# Include the common BoardConfig
include device/samsung/universal7870-common/BoardConfigCommon.mk

# Inherit proprietary vendor blobs
include vendor/samsung/j7xelte/BoardConfigVendor.mk

DEVICE_PATH := device/samsung/j7xelte

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/configs/bluetooth

# Bootanimation
TARGET_SCREEN_WIDTH := 720
TARGET_SCREEN_HEIGHT := 1280

# Kernel
TARGET_KERNEL_CONFIG := exynos7870-j7xelte_defconfig

# OTA assertions
TARGET_OTA_ASSERT_DEVICE := j7xelte

# Partitions
BOARD_USERDATAIMAGE_PARTITION_SIZE := 10737418240
