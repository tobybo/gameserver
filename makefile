
include config.mk

all:
	@for dir in $(BUILD_DIR);\
	do \
		make -C $$dir; \
	done
	@echo "end make ----------------------"
	#rm -rf app/link_obj app/dep
	@echo "end remove ----------------------"
clean:
	rm -rf app/link_obj app/dep gameserver

