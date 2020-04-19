
export BUILD_ROOT = $(shell pwd)
export INCLUDE_PATH = $(BUILD_ROOT)/include \
					  -I /usr/include/mysql/
export LIB_NAME = pthread -lmysqlclient
export DEBUG = true

BUILD_DIR = $(BUILD_ROOT)/tools \
			$(BUILD_ROOT)/signal \
			$(BUILD_ROOT)/net \
			$(BUILD_ROOT)/mysql \
			$(BUILD_ROOT)/objects \
			$(BUILD_ROOT)/logic \
			$(BUILD_ROOT)/misc \
			$(BUILD_ROOT)/app
