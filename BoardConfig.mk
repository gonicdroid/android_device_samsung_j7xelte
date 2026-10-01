# Include the common BoardConfig
include device/samsung/universal7870-common/BoardConfigCommon.mk

# Inherit proprietary vendor blobs
include vendor/samsung/j7xelte/BoardConfigVendor.mk

DEVICE_PATH := device/samsung/j7xelte

# Properties
TARGET_VENDOR_PROP += $(DEVICE_PATH)/vendor.prop

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/configs/bluetooth

# Kernel
TARGET_KERNEL_CONFIG := exynos7870-j7xelte_defconfig

# OTA assertions
TARGET_OTA_ASSERT_DEVICE := j7xelte

# Partitions
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 3145728000
BOARD_USERDATAIMAGE_PARTITION_SIZE := 10737418240
