#
# Copyright (C) 2026 The LineageOS Project
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

DEVICE_PATH := device/samsung/j7xelte

# Device Overlay
DEVICE_PACKAGE_OVERLAYS += $(DEVICE_PATH)/overlay

# -----------------------------------------------------------
# Audio - mixer paths specific to j7xelte
# -----------------------------------------------------------
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/audio/mixer_paths_rev03.xml:$(TARGET_COPY_OUT_VENDOR)/etc/mixer_paths_rev03.xml

# -----------------------------------------------------------
# NFC Samsung S3NRN80 (j7xelte only)
# -----------------------------------------------------------
PRODUCT_PACKAGES += \
    NfcNci \
    Tag \
    com.android.nfc_extras

# NFC Configs from J710MNUBS4CTF2
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/configs/nfc/libnfc-sec-hal.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-sec-hal.conf \
    $(DEVICE_PATH)/configs/nfc/libnfc-sec.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-sec.conf \
    $(DEVICE_PATH)/configs/nfc/libnfc-sec.conf:$(TARGET_COPY_OUT_VENDOR)/etc/libnfc-nci.conf \
    $(DEVICE_PATH)/configs/nfc/sec_s3nrn80_rfreg.bin:$(TARGET_COPY_OUT_VENDOR)/etc/sec_s3nrn80_rfreg.bin \
    $(DEVICE_PATH)/configs/nfc/libnfc-sec-hal.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/libnfc-sec-hal.conf \
    $(DEVICE_PATH)/configs/nfc/libnfc-sec.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/libnfc-sec.conf \
    $(DEVICE_PATH)/configs/nfc/libnfc-sec.conf:$(TARGET_COPY_OUT_SYSTEM)/etc/libnfc-nci.conf \
    $(DEVICE_PATH)/configs/nfc/nfcee_access.xml:$(TARGET_COPY_OUT_SYSTEM)/etc/nfcee_access.xml

# NFC Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.nfc.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.xml \
    frameworks/native/data/etc/android.hardware.nfc.hce.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hce.xml \
    frameworks/native/data/etc/android.hardware.nfc.hcef.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.nfc.hcef.xml \
    frameworks/native/data/etc/com.android.nfc_extras.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/com.android.nfc_extras.xml
