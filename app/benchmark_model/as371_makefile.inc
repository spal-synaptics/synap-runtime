#
# NDA AND NEED-TO-KNOW REQUIRED
#
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.
#
# This file contains information that is proprietary to Synaptics
# Incorporated ("Synaptics"). The holder of this file shall treat all
# information contained herein as confidential, shall use the
# information only for its intended purpose, and shall not duplicate,
# disclose, or disseminate any of this information in any manner
# unless Synaptics has otherwise provided express, written
# permission.
#
# Use of the materials may require a license of intellectual property
# from a third party or from Synaptics. This file conveys no express
# or implied licenses to any intellectual property rights belonging
# to Synaptics.
#
# INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS", AND
# SYNAPTICS EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES,
# INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE, AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY
# INTELLECTUAL PROPERTY RIGHTS. IN NO EVENT SHALL SYNAPTICS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE, OR
# CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION WITH THE USE
# OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED AND
# BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF
# COMPETENT JURISDICTION DOES NOT PERMIT THE DISCLAIMER OF DIRECT
# DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS' TOTAL CUMULATIVE LIABILITY
# TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S. DOLLARS.
#
# Settings for generic aarch64 boards such as Odroid C2 or Pine64.
ifeq ($(TARGET),as371)
  # The aarch64 architecture covers all 64-bit ARM chips. This arch mandates
  # NEON, so FPU flags are not needed below.
  TARGET_ARCH := armv7-a
  CC_PREFIX := $(TARGET_TOOLCHAIN_ROOT)/bin/

  CXXFLAGS += \
    -march=armv7-a \
    -funsafe-math-optimizations \
    -ftree-vectorize \
    -fPIC

  CFLAGS += \
    -march=armv7-a \
    -funsafe-math-optimizations \
    -ftree-vectorize \
    -fPIC

  LDFLAGS := \
    -Wl,--no-export-dynamic \
    -Wl,--exclude-libs,ALL \
    -Wl,--gc-sections \
    -Wl,--as-needed \
    -lrt

       
  LIBS := \
    -lstdc++ \
    -lpthread \
    -lm \
    -ldl

else ifeq ($(TARGET),android_vssdk)
  # The aarch64 architecture covers all 64-bit ARM chips. This arch mandates
  # NEON, so FPU flags are not needed below.
  TARGET_ARCH := armv7-a
  CC_PREFIX := $(TARGET_TOOLCHAIN_ROOT)/bin/

  CXXFLAGS += \
    -march=armv7-a \
    -funsafe-math-optimizations \
    -ftree-vectorize \
    -fPIC \
    -fPIE \
    -pie \
    --sysroot=$(SYNAP_ROOT)/sysroot/$(TARGET) \
    -std=c++11 \
    -fpermissive

  CFLAGS += \
    -march=armv7-a \
    -funsafe-math-optimizations \
    -ftree-vectorize \
    -fPIC \
    -fPIE \
    -pie \
    --sysroot=$(SYNAP_ROOT)/sysroot/$(TARGET) \
    -std=c11 \
    -fpermissive

  LDFLAGS := \
    -Wl,--no-export-dynamic \
    -Wl,--exclude-libs,ALL \
    -Wl,--gc-sections \
    -Wl,--as-needed

  LIBS := \
    -lc++_static \
    -lm

  INCLUDES += \
  -I$(SYNAP_ROOT)/sysroot/$(TARGET)/include/libcxx \
  -I$(SYNAP_ROOT)/sysroot/$(TARGET)/include/libcxxabi

endif
