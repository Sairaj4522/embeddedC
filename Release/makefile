################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include source/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
autoclk3.0.lss \

FLASH_IMAGE += \
autoclk3.0.hex \

EEPROM_IMAGE += \
autoclk3.0.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: autoclk3.0.elf secondary-outputs

# Tool invocations
autoclk3.0.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,autoclk3.0.map -mmcu=atmega32 -o "autoclk3.0.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

autoclk3.0.lss: autoclk3.0.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S autoclk3.0.elf  >"autoclk3.0.lss"
	@echo 'Finished building: $@'
	@echo ' '

autoclk3.0.hex: autoclk3.0.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex autoclk3.0.elf  "autoclk3.0.hex"
	@echo 'Finished building: $@'
	@echo ' '

autoclk3.0.eep: autoclk3.0.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex autoclk3.0.elf  "autoclk3.0.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: autoclk3.0.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega32 autoclk3.0.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) autoclk3.0.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
