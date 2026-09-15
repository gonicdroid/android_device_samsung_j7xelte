#
# Copyright (C) 2022 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


# Inherit from those products. Most specific first.
#$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/product_launched_with_m.mk)
$(call inherit-product, frameworks/native/build/phone-xhdpi-2048-dalvik-heap.mk)
# Inherit device configuration
$(call inherit-product, device/samsung/j7xelte/common.mk)

# Inherit some common Lineage stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Device identifiers. These must come after all inclusions.
PRODUCT_DEVICE := j7xelte
PRODUCT_MODEL := SM-J710
PRODUCT_BRAND := samsung
PRODUCT_MANUFACTURER := Samsung
PRODUCT_NAME := lineage_j7xelte
PRODUCT_GMS_CLIENTID_BASE := android-samsung

PRODUCT_FULL_TREBLE_OVERRIDE := true
PRODUCT_VENDOR_MOVE_ENABLED := true

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="j7xelteub-user 8.1.0 M1AJQ J710MNUBS4CTF2 release-keys" \
    PRODUCT_NAME="j7xelte"

# Set the build fingerprint
BUILD_FINGERPRINT := google/ryu/dragon:8.1.0/OPM1.171019.016/4503492:user/release-keys
