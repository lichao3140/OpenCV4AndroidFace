LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# 打包apk时加载OpenCV的动态库
OPENCV_INSTALL_MODULES:=on
# 动态库
OPENCV_LIB_TYPE:=SHARED
# 指定OpenCV SDK中OpenCV.mk所在的路径
include E:\lichao\study\opencv\opencv3.4\OpenCV-android-sdk\sdk\native\jni\OpenCV.mk

# 指定jni目录中src文件
LOCAL_SRC_FILES  := DetectionBasedTracker_jni.cpp
# 指定本地链接库
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_LDLIBS     += -llog -ldl
# 本地.so文件名称
LOCAL_MODULE     := detection_based_tracker

include $(BUILD_SHARED_LIBRARY)
