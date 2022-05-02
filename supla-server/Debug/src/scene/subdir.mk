################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scene/scene.cpp \
../src/scene/scene_abstract_repository.cpp \
../src/scene/scene_action_config.cpp \
../src/scene/scene_asynctask.cpp \
../src/scene/scene_db.cpp \
../src/scene/scene_operation.cpp \
../src/scene/scene_operations.cpp \
../src/scene/scene_repository.cpp \
../src/scene/scene_search_condition.cpp \
../src/scene/scenes.cpp 

CPP_DEPS += \
./src/scene/scene.d \
./src/scene/scene_abstract_repository.d \
./src/scene/scene_action_config.d \
./src/scene/scene_asynctask.d \
./src/scene/scene_db.d \
./src/scene/scene_operation.d \
./src/scene/scene_operations.d \
./src/scene/scene_repository.d \
./src/scene/scene_search_condition.d \
./src/scene/scenes.d 

OBJS += \
./src/scene/scene.o \
./src/scene/scene_abstract_repository.o \
./src/scene/scene_action_config.o \
./src/scene/scene_asynctask.o \
./src/scene/scene_db.o \
./src/scene/scene_operation.o \
./src/scene/scene_operations.o \
./src/scene/scene_repository.o \
./src/scene/scene_search_condition.o \
./src/scene/scenes.o 


# Each subdirectory must supply rules for building sources it contributes
src/scene/%.o: ../src/scene/%.cpp src/scene/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-scene

clean-src-2f-scene:
	-$(RM) ./src/scene/scene.d ./src/scene/scene.o ./src/scene/scene_abstract_repository.d ./src/scene/scene_abstract_repository.o ./src/scene/scene_action_config.d ./src/scene/scene_action_config.o ./src/scene/scene_asynctask.d ./src/scene/scene_asynctask.o ./src/scene/scene_db.d ./src/scene/scene_db.o ./src/scene/scene_operation.d ./src/scene/scene_operation.o ./src/scene/scene_operations.d ./src/scene/scene_operations.o ./src/scene/scene_repository.d ./src/scene/scene_repository.o ./src/scene/scene_search_condition.d ./src/scene/scene_search_condition.o ./src/scene/scenes.d ./src/scene/scenes.o

.PHONY: clean-src-2f-scene

