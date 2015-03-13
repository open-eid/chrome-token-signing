# This is the Makefile for OSX/Linux. See Makefile for Windows NMake.
UNAME = `uname`
RELEASE = `grep '"version"' extension/manifest.json  | cut -d'"' -f 4`

detect:
	make $(UNAME)

Linux:
	make -C host-linux

Darwin:
	make -C host-osx

# Make the zip to be uploaded to chrome web store
release:
	test ! -f extension-$(RELEASE).zip
	test -z "`git status -s extension`"
	git clean -dfx extension
	zip -r extension-$(RELEASE).zip extension

test: detect
	python host-test/pipe-test.py -v

# Make the targzip for the native components
# FIXME: git describe vs $(RELEASE) ?
dist:
	git-archive-all chrome-token-signing-`git describe --tags --always`.tar.gz
