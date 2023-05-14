BUILDDIR := ../out
EXEPATH := "$(BUILDDIR)/myinit"

.SILENT: all build

all: build

build:
	mkdir -p $(BUILDDIR)
	gcc main.c \
		./error/error.c \
		./log/log.c \
		./process_manager/process_manager.c \
		./config/config.c \
		./file/utils.c \
		./signals/signal_handlers.c \
		-o $(EXEPATH) -std=gnu11