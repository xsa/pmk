# $Id$

# source scanning test

SCAN=		pmkscan
SCAN_FILE=	pmkfile.scan
SAMPLE=		hello.c

all: show_sample $(SCAN) show_scan clean

show_sample:
	@echo "-> Dumping sample C file"
	@echo ""
	@echo "$(SAMPLE):"
	@echo "--dump-begin----------------------------"
	@cat $(SAMPLE)
	@echo "--dump-end------------------------------"

$(SCAN):
	@echo ""
	@echo "-> Running $(SCAN)"
	@echo ""
	$(SCAN) -p

show_scan:
	@echo ""
	@echo "-> Dumping $(SCAN_FILE)"
	@echo ""
	@echo "--dump-begin----------------------------"
	@cat $(SCAN_FILE)
	@echo "--dump-end------------------------------"

clean:
	@echo ""
	@echo "-> Cleaning ..."
	@echo ""
	@echo "Removing $(SCAN_FILE)"
	@rm $(SCAN_FILE)
	@echo ""
	@echo "Done."

