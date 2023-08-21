################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/client/abstract_client_dao.cpp \
../src/client/abstract_client_scene_dao.cpp \
../src/client/client.cpp \
../src/client/client_channel_relation_remote_updater.cpp \
../src/client/client_channel_relations.cpp \
../src/client/client_dao.cpp \
../src/client/client_scene.cpp \
../src/client/client_scene_change_indicator.cpp \
../src/client/client_scene_dao.cpp \
../src/client/client_scene_remote_updater.cpp \
../src/client/client_scenes.cpp \
../src/client/clientchannel.cpp \
../src/client/clientchannelgroup.cpp \
../src/client/clientchannelgrouprelation.cpp \
../src/client/clientchannelgroups.cpp \
../src/client/clientchannelgroupvalue.cpp \
../src/client/clientchannels.cpp \
../src/client/clientlocation.cpp \
../src/client/clientobjcontainer.cpp \
../src/client/clientobjcontaineritem.cpp 

CPP_DEPS += \
./src/client/abstract_client_dao.d \
./src/client/abstract_client_scene_dao.d \
./src/client/client.d \
./src/client/client_channel_relation_remote_updater.d \
./src/client/client_channel_relations.d \
./src/client/client_dao.d \
./src/client/client_scene.d \
./src/client/client_scene_change_indicator.d \
./src/client/client_scene_dao.d \
./src/client/client_scene_remote_updater.d \
./src/client/client_scenes.d \
./src/client/clientchannel.d \
./src/client/clientchannelgroup.d \
./src/client/clientchannelgrouprelation.d \
./src/client/clientchannelgroups.d \
./src/client/clientchannelgroupvalue.d \
./src/client/clientchannels.d \
./src/client/clientlocation.d \
./src/client/clientobjcontainer.d \
./src/client/clientobjcontaineritem.d 

OBJS += \
./src/client/abstract_client_dao.o \
./src/client/abstract_client_scene_dao.o \
./src/client/client.o \
./src/client/client_channel_relation_remote_updater.o \
./src/client/client_channel_relations.o \
./src/client/client_dao.o \
./src/client/client_scene.o \
./src/client/client_scene_change_indicator.o \
./src/client/client_scene_dao.o \
./src/client/client_scene_remote_updater.o \
./src/client/client_scenes.o \
./src/client/clientchannel.o \
./src/client/clientchannelgroup.o \
./src/client/clientchannelgrouprelation.o \
./src/client/clientchannelgroups.o \
./src/client/clientchannelgroupvalue.o \
./src/client/clientchannels.o \
./src/client/clientlocation.o \
./src/client/clientobjcontainer.o \
./src/client/clientobjcontaineritem.o 


# Each subdirectory must supply rules for building sources it contributes
src/client/%.o: ../src/client/%.cpp src/client/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-client

clean-src-2f-client:
	-$(RM) ./src/client/abstract_client_dao.d ./src/client/abstract_client_dao.o ./src/client/abstract_client_scene_dao.d ./src/client/abstract_client_scene_dao.o ./src/client/client.d ./src/client/client.o ./src/client/client_channel_relation_remote_updater.d ./src/client/client_channel_relation_remote_updater.o ./src/client/client_channel_relations.d ./src/client/client_channel_relations.o ./src/client/client_dao.d ./src/client/client_dao.o ./src/client/client_scene.d ./src/client/client_scene.o ./src/client/client_scene_change_indicator.d ./src/client/client_scene_change_indicator.o ./src/client/client_scene_dao.d ./src/client/client_scene_dao.o ./src/client/client_scene_remote_updater.d ./src/client/client_scene_remote_updater.o ./src/client/client_scenes.d ./src/client/client_scenes.o ./src/client/clientchannel.d ./src/client/clientchannel.o ./src/client/clientchannelgroup.d ./src/client/clientchannelgroup.o ./src/client/clientchannelgrouprelation.d ./src/client/clientchannelgrouprelation.o ./src/client/clientchannelgroups.d ./src/client/clientchannelgroups.o ./src/client/clientchannelgroupvalue.d ./src/client/clientchannelgroupvalue.o ./src/client/clientchannels.d ./src/client/clientchannels.o ./src/client/clientlocation.d ./src/client/clientlocation.o ./src/client/clientobjcontainer.d ./src/client/clientobjcontainer.o ./src/client/clientobjcontaineritem.d ./src/client/clientobjcontaineritem.o

.PHONY: clean-src-2f-client

