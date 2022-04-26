################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scene/scene.cpp \
../src/scene/scene_asynctask.cpp \
../src/scene/scene_db.cpp \
../src/scene/scene_operation.cpp \
../src/scene/scene_operations.cpp \
../src/scene/scene_repository.cpp \
../src/scene/scene_repository_factory.cpp \
../src/scene/scene_search_condition.cpp \
../src/scene/scenes.cpp 

CPP_DEPS += \
./src/scene/scene.d \
./src/scene/scene_asynctask.d \
./src/scene/scene_db.d \
./src/scene/scene_operation.d \
./src/scene/scene_operations.d \
./src/scene/scene_repository.d \
./src/scene/scene_repository_factory.d \
./src/scene/scene_search_condition.d \
./src/scene/scenes.d 

OBJS += \
./src/scene/scene.o \
./src/scene/scene_asynctask.o \
./src/scene/scene_db.o \
./src/scene/scene_operation.o \
./src/scene/scene_operations.o \
./src/scene/scene_repository.o \
./src/scene/scene_repository_factory.o \
./src/scene/scene_search_condition.o \
./src/scene/scenes.o 


# Each subdirectory must supply rules for building sources it contributes
src/scene/%.o: ../src/scene/%.cpp src/scene/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-scene

clean-src-2f-scene:
	-$(RM) ./src/scene/scene.d ./src/scene/scene.o ./src/scene/scene_asynctask.d ./src/scene/scene_asynctask.o ./src/scene/scene_db.d ./src/scene/scene_db.o ./src/scene/scene_operation.d ./src/scene/scene_operation.o ./src/scene/scene_operations.d ./src/scene/scene_operations.o ./src/scene/scene_repository.d ./src/scene/scene_repository.o ./src/scene/scene_repository_factory.d ./src/scene/scene_repository_factory.o ./src/scene/scene_search_condition.d ./src/scene/scene_search_condition.o ./src/scene/scenes.d ./src/scene/scenes.o

.PHONY: clean-src-2f-scene

