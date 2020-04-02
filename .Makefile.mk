#
# see .Makefile.template for help
#

### variables ###

VARIABLE+=DEV
HELP_DEV=which device to flash/monitor
DEV?=none

### internal variables ###

# assume /etc/passwd holds right shell of user
USERSHELL?=$(shell awk -F ':' '/^'$$(id -un)':/{print $$NF}' /etc/passwd)

# export any additional docker options for docker Makefile
export DOCKEROPTS

# assume we are in projectdir
PROJECTDIR?=$(dir $(shell pwd))

# assume dirname is projectname
PROJECT?=$(notdir $(shell pwd))

# assume docker Makefile dir
DOCKERDIR?=../docker

# assume template path
TEMPLATEPATH?=../.Makefile.template

# assume we want to start where we are
WORKDIR?=$(PROJECTDIR)/$(PROJECT)
# export for docker Makefile
export WORKDIR

### default target ###
DEFAULT += help

### menuconfig targets ###

.PHONY: menuconfig
TARGET += menuconfig

HELP_menuconfig = configure project
menuconfig: | check-docker
	@make --no-print-directory -C $(DOCKERDIR) idf \
		EXEC="idf.py menuconfig"

### build targets ###

.PHONY: reconfigure
TARGET += reconfigure
ALL += reconfigure
HELP_reconfigure = reconfigure project (rebuilds cmake files)
reconfigure: | check-docker
	@make --no-print-directory -C $(DOCKERDIR) idf \
		EXEC="idf.py reconfigure"

.PHONY: build
TARGET += build
ALL += build
HELP_build = build project
build: | check-docker
	@make --no-print-directory -C $(DOCKERDIR) idf \
		EXEC="idf.py --ccache build"

.PHONY: clean-build
CLEAN += clean-build
HELP_clean-build = remove generated files of project components
clean-build: | check-docker
	rm -rf $(WORKDIR)/build

.PHONY: distclean-build
DISTCLEAN += distclean-build
HELP_distclean-build = remove all generated files
distclean-build:
	rm -f  $(WORKDIR)/sdkconfig

### flash targets ###

.PHONY: flash
TARGET += flash
HELP_flash = flash project to esp. Use DEV=path to provide  path to device or use make dev
flash: | check-flash
	@make --no-print-directory -C $(DOCKERDIR) idf \
		EXEC="sudo chgrp developer $(DEV);\
		      idf.py flash -p '$(DEV)'"

.PHONY: check-flash
CHECK += check-flash
HELP_check-flash = check env if flashing is possible
check-flash: | check-docker check-dev

### monitor targets ###

.PHONY: monitor
TARGET += monitor
HELP_monitor = connect to esp32 via serial. Use DEV=path to provide path to device or use make dev
monitor: | check-monitor
	@make --no-print-directory -C $(DOCKERDIR) idf \
		EXEC="sudo chgrp developer $(DEV); \
		      idf.py monitor -p '$(DEV)'"

.PHONY: check-monitor
CHECK += check-monitor
HELP_check-monitor = check env if monitor is possible
check-monitor: | check-docker check-dev

### qemu targets ###

.PHONY: qemu
TARGET += qemu
HELP_qemu = start qemu and run build image
qemu: qemu-image | check-docker
	@make --no-print-directory -C docker qemu \
		EXEC="cd $(PROJECT); \
	        echo \"############################\"; \
	        echo \"# TO EXIT QEMU: <CTRL-X A> #\"; \
	        echo \"############################\"; \
	        qemu-system-xtensa \
	          -no-reboot \
	          -nographic \
	          -machine esp32 \
	          -drive file=build/qemu.bin,if=mtd,format=raw \
	        2>&1 | tee build/qemu.log; \
	        grep -q \"0 Failures\" build/qemu.log"

.PHONY: qemu-image
TARGET_qemu += qemu-image
HELP_qemu-image = converts build image to image runnable by qemu
qemu-image: $(PROJECT)/build/qemu.bin
.DELETE_ON_ERROR: $(PROJECT)/build/qemu.bin
$(PROJECT)/build/qemu.bin: $(PROJECT)/build/$(PROJECT).bin
	dd if=/dev/zero bs=1024 count=4096 of=$@
	dd if=$(PROJECT)/build/bootloader/bootloader.bin bs=1 seek=$$((0x1000)) of=$@ conv=notrunc
	dd if=$(PROJECT)/build/partition_table/partition-table.bin bs=1 seek=$$((0x8000)) of=$@ conv=notrunc
	dd if=$(PROJECT)/build/$(PROJECT).bin bs=1 seek=$$((0x10000)) of=$@ conv=notrunc

.PHONY: qemu-gdb
TARGET += qemu-gdb
HELP_qemu-gdb = start qemu and wait for gdb
qemu-gdb: qemu-image | check-docker
	@make --no-print-directory -C docker qemu \
	  DOCKEROPTS="--name $(PROJECT).qemu" \
		EXEC="cd $(PROJECT); \
	        echo \"###################################\"; \
	        echo \"#        TO START GDB RUN         #\"; \
	        echo \"# make PROJECT=<PROJECT> gdb-qemu #\"; \
	        echo \"#        IN ANOTHER SHELL         #\"; \
	        echo \"###################################\"; \
	        qemu-system-xtensa \
	          -s -S \
	          -no-reboot \
	          -nographic \
	          -machine esp32 \
	          -drive file=build/qemu.bin,if=mtd,format=raw"


### gdb targets ###

.PHONY: gdb-qemu
TARGET += gdb-qemu
gdb-HELP_qemu = start gdb and connect to qemu
gdb-qemu: $(PROJECT)/build/$(PROJECT).elf | check-docker
	docker exec -ti $(PROJECT).qemu \
	  bash -ic "xtensa-esp32-elf-gdb $^ \
	                -ex \"target remote :1234\" \
	                -ex \"tb app_main\" \
	                -ex \"c\""

### dev targets ###

.PHONY: dev
TARGET += dev
HELP_dev = specify which USB device to connect to
dev: | check-dialog
	export DEV=$$((for dev in $$(ls /dev/serial/by-id); do echo "$$(readlink -f /dev/serial/by-id/$$dev) $$dev "; done ) \
	| dialog \
		--stdout \
		--menu "choose which device to use" 0 0 0 "none" "disable device passthrough"\
		--file /dev/stdin);\
		clear; \
		exec $(USERSHELL)

.PHONY: check-dev
CHECK += check-dev
HELP_check-dev = check if device is specified
check-dev:
	@1>&2 echo -n "checking if device is valid & specified..."
ifeq ($(DEV),none)
	@1>&2 echo
	@1>&2 echo "##############################"
	@1>&2 echo "# FLASH DEVICE NOT SPECIFIED #"
	@1>&2 echo "##############################"
	@1>&2 echo
	@1>&2 echo "specify device by adding DEV as parameter"
	@1>&2 echo
	@1>&2 echo "make $(MAKECMDGOALS) DEV=/path/to/device"
	@1>&2 echo
	@1>&2 echo "or by running"
	@1>&2 echo
	@1>&2 echo "make dev"
	@1>&2 echo
	@exit 1
endif
ifeq ($(shell ! test -c $(DEV); echo $$?),0)
	@1>&2 echo
	@1>&2 echo "#############################"
	@1>&2 echo "# FLASH DEVICE IS NOT VALID #"
	@1>&2 echo "#############################"
	@1>&2 echo
	@1>&2 echo "the specified device ($(DEV)) is not a character device!"
	@1>&2 echo
	@1>&2 echo "specify device by adding DEV as parameter"
	@1>&2 echo
	@1>&2 echo "make $(MAKECMDGOALS) DEV=/path/to/device"
	@1>&2 echo
	@1>&2 echo "or by running"
	@1>&2 echo
	@1>&2 echo "make dev"
	@1>&2 echo
	@exit 1
endif
	@1>&2 echo "SUCCESS"

.PHONY: check-dialog
CHECK += check-dialog
HELP_check-dialog = check if dialog is installed
check-dialog:
	@1>&2 echo -n "checking if dialog is installed..."
ifeq ($(shell which dialog 2> /dev/null),)
	@1>&2 echo
	@1>&2 echo "###########################"
	@1>&2 echo "# DIALOG IS NOT INSTALLED #"
	@1>&2 echo "###########################"
	@1>&2 echo
	@1>&2 echo "Consider installing dialog."
	@1>&2 echo
	@1>&2 echo "on debian based systems:"
	@1>&2 echo "    sudo apt-get install dialog"
	@1>&2 echo
	@1>&2 echo "on arch based systems:"
	@1>&2 echo "    sudo pacman -S dialog"
	@1>&2 echo
	@exit 1
endif
	@1>&2 echo "SUCCESS"

### docker targets ###

.PHONY: check-docker
CHECK += check-docker
HELP_check-docker = check if docker is usable
check-docker:
	@make --no-print-directory -C $(DOCKERDIR) check

### git targets

.PHONY: setup-git
SETUP += setup-git
HELP_setup-git = setup git commit message
setup-git:
	git config --replace-all commit.template .gitcommitmsg
	git config --replace-all pull.rebase true

include $(TEMPLATEPATH)
