# $Id$ 

PREMAKE=	pmk
SETUP=		$(PREMAKE)setup
SCAN=		$(PREMAKE)scan
INST=           $(PREMAKE)install
PKGCFG=		$(PREMAKE)pc

SAMPLE=		$(PREMAKE)file.sample
CONFIG=		$(PREMAKE).conf.sample

#SELFINST=	self-$(INST)

TEST_SAMPLE=	test_samples

all: test_all

test_bin_c:
	@echo ""
	@echo "=> Testing C binary building"
	@echo ""
	@echo "-> Running pmk"
	@echo ""
	@cd bin_c && pmk
	@echo ""
	@cd bin_c && make
	@echo ""
	@echo "=> End of test"
	@echo ""

test_bin_cxx:
	@echo ""
	@echo "=> Testing C++ binary building"
	@echo ""
	@echo "-> Running pmk"
	@echo ""
	@cd bin_c++ && pmk
	@echo ""
	@cd bin_c++ && make
	@echo ""
	@echo "=> End of test"
	@echo ""

test_lib_c:
	@echo ""
	@echo "=> Testing C library building"
	@echo ""
	@echo "-> Running pmk"
	@echo ""
	@cd lib_c && pmk
	@echo ""
	@cd lib_c && make
	@echo ""
	@echo "=> End of test"
	@echo ""

test_lib_cxx:
	@echo ""
	@echo "=> Testing C++ library building"
	@echo ""
	@echo "-> Running pmk"
	@echo ""
	@cd lib_c++ && pmk
	@echo ""
	@cd lib_c++ && make
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

test_scan_c:
	@echo ""
	@echo "=> Testing $(SCAN)"
	@cd scan_c && make
	@echo ""
	@echo "=> End of test"
	@echo ""

test_$(INST):
	@echo ""
	@echo "=> Testing $(INST)"
	@cd install && make
	@echo ""
	@echo "=> End of test"
	@echo ""

test_clean:
	@echo ""
	@echo "=> Removing generated files"
	rm -rf $(TEST_SAMPLE)
	rm -f ac_config.h pmkfile.scan pmk*.log

	@echo ""
	@echo "=> End of cleaning."
	@echo ""

test_pmk_only: test_$(SETUP) test_$(PREMAKE) test_clean

test_old_all: test_pmk_only

# all binary tests
test_bins: test_bin_c test_bin_cxx

# all scan tests
test_scans: test_scan_c

# all library tests
test_libs: test_lib_c test_lib_cxx

# all tests
test_all: test_bins test_libs test_scans test_$(INST)

# test target => alias to all target 
test: all

