# $Id$ 

PREMAKE=	pmk
SETUP=		$(PREMAKE)setup
SCAN=		$(PREMAKE)scan
INST=           $(PREMAKE)install
PKGCFG=		$(PREMAKE)pc

SAMPLE=		$(PREMAKE)file.sample
CONFIG=		$(PREMAKE).conf.sample

SELFINST=	self-$(INST)

TEST_SAMPLE=	test_samples
TEST_INST=	test_pmkinstall
TEST_TARGET=	$(TEST_INST)/$(INST).test

test_lib_c:
	@echo ""
	@echo "=> Testing C library building"
	@echo ""
	@echo "-> Dumping target files"
	@echo ""
	@echo "lib_c/Makefile.pmk:"
	@echo "--dump-begin----------------------------"
	@cat lib_c/Makefile.pmk
	@echo "--dump-end------------------------------"
	@echo ""
	@echo "-> Runing pmk"
	@echo ""
	@cd lib_c && pmk
	@echo "-> Dumping generated files"
	@echo ""
	@echo "lib_c/Makefile:"
	@echo "--dump-begin----------------------------"
	@cat lib_c/Makefile
	@echo "--dump-end------------------------------"
	@echo ""
	@echo "-> Running make"
	@echo ""
	@cd lib_c && make
	@echo ""
	@echo "-> Running program"
	@echo ""
	@echo "--exec-begin----------------------------"
	@cd lib_c; LD_LIBRARY_PATH=. hello
	@echo "--exec-end------------------------------"
	@echo ""
	@echo "-> cleaning"
	@echo ""
	@cd lib_c && make clean
	@echo ""
	@echo "=> End of test"
	@echo ""

test_$(PREMAKE):
	@echo ""
	@echo "=> Testing $(PREMAKE) with sample files"
	@echo ""
	@echo "-> Dumping target files"
	@echo ""
	@echo "Makefile.sample.pmk"
	@echo "----------------------------------------"
	@cat Makefile.sample.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "subdir/Makefile.subdir.pmk"
	@echo "----------------------------------------"
	@cat subdir/Makefile.subdir.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "config_sample.h.pmk"
	@echo "----------------------------------------"
	@cat config_sample.h.pmk
	@echo "----------------------------------------"
	@echo ""
	@echo "ac_config.h"
	@echo "----------------------------------------"
	@cp ac_config.h.sample ac_config.h
	@cat ac_config.h
	@echo "----------------------------------------"
	@echo ""
	@echo "-> Running pmk"
	$(PREMAKE) -l -b $(TEST_SAMPLE) -e use_gtk -f pmkfile.sample -o ovrfile.sample
	@echo ""
	@echo "-> Dumping generated files"
	@echo ""
	@echo "$(TEST_SAMPLE)/Makefile.sample"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/Makefile.sample
	@echo "----------------------------------------"
	@echo ""
	@echo "$(TEST_SAMPLE)/subdir/Makefile.subdir"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/subdir/Makefile.subdir
	@echo "----------------------------------------"
	@echo ""
	@echo "$(TEST_SAMPLE)/config_sample.h"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/config_sample.h
	@echo "----------------------------------------"
	@echo ""
	@echo "$(TEST_SAMPLE)/ac_config.h"
	@echo "----------------------------------------"
	@cat $(TEST_SAMPLE)/ac_config.h
	@echo "----------------------------------------"
	@echo ""
	@echo "=> End of test"
	@echo ""

test_$(SETUP):
	@echo ""
	@echo "=> Testing $(SETUP)"
	@echo "Generating local pmk.conf."
	@echo "(need USERMODE enabled)"
	@echo ""
	$(SETUP) -V
	@echo ""

test_$(SCAN):
	@echo ""
	@echo "=> Testing $(SCAN)"
	$(SCAN)
	@echo ""
	@echo "Dumping pmkfile.scan"
	@echo "----------------------------------------"
	@cat pmkfile.scan
	@echo "----------------------------------------"
	@echo ""
	@echo "=> End of test"
	@echo ""

test_$(INST):
	@echo ""
	@echo "=> Testing $(INST)"
	@echo ""
	@echo "-> Creating test directory"
	@echo ""
	$(INST) -d -m 770 $(TEST_INST)
	@echo ""
	@echo "-> Checking test directory"
	@if (test -d "$(TEST_INST)"); then \
		echo ""; \
		echo "- - - - - - - - - - - - - - - - - - -" \
			"- - - - - - - - - - - - - - - - - -"; \
		echo ""; \
		ls -ld $(TEST_INST); \
		echo ""; \
		echo "- - - - - - - - - - - - - - - - - - -" \
			"- - - - - - - - - - - - - - - - - -"; \
		echo ""; \
		echo "Directory OK."; \
	else \
		echo ""; \
		echo "Failed."; \
		exit 1; \
	fi
	@echo ""
	@echo "-> Installing test data"
	@echo ""
	$(INST) -m u+rw README.samples $(TEST_TARGET)1
	$(INST) -m ug+r README.samples $(TEST_TARGET)2
	@echo ""
	@echo "-> Checking test file"
	@echo "";
	@echo "- - - - - - - - - - - - - - - - - - -" \
		"- - - - - - - - - - - - - - - - - -";
	@echo "";
	@for i in 1 2; do \
		if (test -f "$(TEST_TARGET)$$i"); then \
			ls -l $(TEST_TARGET)$$i; \
		else \
			echo "$(TEST_TARGET)$$i failed."; \
			exit 1; \
		fi \
	done
	@echo "";
	@echo "- - - - - - - - - - - - - - - - - - -" \
		"- - - - - - - - - - - - - - - - - -";
	@echo "";
	@echo ""
	@echo "=> End of test"
	@echo ""


test_clean:
	@echo ""
	@echo "=> Removing generated files"
	rm -rf $(TEST_SAMPLE)
	rm -rf $(TEST_TARGET)*
	rm -rf $(TEST_INST)
	rm -f ac_config.h pmkfile.scan pmk*.log

	@echo ""
	@echo "=> End of cleaning."
	@echo ""

test_old_all: test_$(SETUP) test_$(PREMAKE) test_$(SCAN) test_$(INST) test_clean

test_pmk_only: test_$(SETUP) test_$(PREMAKE) test_clean

test_all: test_lib_c

test: test_old_all test_all

