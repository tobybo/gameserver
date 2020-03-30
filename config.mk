
export BUILD_ROOT = $(shell pwd)
export INCLUDE_PATH = $(BUILD_ROOT)/include
export DEBUG = true

BUILD_DIR = $(BUILD_ROOT)/tools \
			$(BUILD_ROOT)/signal \
			$(BUILD_ROOT)/net \
			$(BUILD_ROOT)/logic \
			$(BUILD_ROOT)/misc \
			$(BUILD_ROOT)/app
