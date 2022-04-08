################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scene/supla_scene_asynctask.cpp \
../src/scene/supla_scene_operation.cpp \
../src/scene/supla_scene_operations.cpp 

CPP_DEPS += \
./src/scene/supla_scene_asynctask.d \
./src/scene/supla_scene_operation.d \
./src/scene/supla_scene_operations.d 

OBJS += \
./src/scene/supla_scene_asynctask.o \
./src/scene/supla_scene_operation.o \
./src/scene/supla_scene_operations.o 


# Each subdirectory must supply rules for building sources it contributes
src/scene/%.o: ../src/scene/%.cpp src/scene/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-scene

clean-src-2f-scene:
	-$(RM) ./src/scene/supla_scene_asynctask.d ./src/scene/supla_scene_asynctask.o ./src/scene/supla_scene_operation.d ./src/scene/supla_scene_operation.o ./src/scene/supla_scene_operations.d ./src/scene/supla_scene_operations.o

.PHONY: clean-src-2f-scene

