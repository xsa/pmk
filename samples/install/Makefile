# $Id$

# install sample test

INST=	pmkinstall

TEST_DIR=	test_dir
TEST_BIN=	hello.sh
TEST_FILE=	hello.txt
TEST_TARGET1=	$(TEST_DIR)/test1.txt
TEST_TARGET2=	$(TEST_DIR)/test2.txt
TEST_TARGET3=	$(TEST_DIR)/test3.sh

TARGET_LIST=	$(TEST_TARGET1) $(TEST_TARGET2) $(TEST_TARGET3)

all: $(TEST_DIR) chk_dir inst_data inst_prog chk_files clean

$(TEST_DIR):
	@echo ""
	@echo "-> Creating test directory"
	@echo ""
	$(INST) -d -m 770 $(TEST_DIR)

chk_dir:
	@echo ""
	@echo "-> Checking test directory"
	@echo ""
	@if (test -d "$(TEST_DIR)"); then \
		ls -ld $(TEST_DIR); \
		echo "Directory OK."; \
	else \
		echo "Failed."; \
		exit 1; \
	fi

inst_data:
	@echo ""
	@echo "-> Installing test data"
	@echo ""
	$(INST) -m u+rw $(TEST_FILE) $(TEST_TARGET1)
	$(INST) -m ug+r $(TEST_FILE) $(TEST_TARGET2)


inst_prog:
	@echo ""
	@echo "-> Installing test program"
	@echo ""
	$(INST) -m 766 $(TEST_BIN) $(TEST_TARGET3)

chk_files:
	@echo ""
	@echo "-> Checking test files"
	@echo ""
	@for i in $(TARGET_LIST); do \
		if (test -f $$i); then \
			ls -l $$i; \
		else \
			echo "$$i detection failed."; \
			exit 1; \
		fi \
	done
	@echo ""

clean:
	@echo ""
	@echo "-> Cleaning ..."
	@echo ""
	@for i in $(TARGET_LIST); do \
		if (test -f $$i); then \
			echo "Removing $$i"; \
			rm -f $$i; \
		fi \
	done
	@echo "Removing $(TEST_DIR)";
	@rmdir $(TEST_DIR)
	@echo "Done."
	@echo ""
