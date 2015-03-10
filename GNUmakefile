UNAME = `uname`
RELEASE = `grep '"version"' extension/manifest.json  | cut -d'"' -f 4`

detect:
	make $(UNAME)

Linux:
	make -C host-linux

Darwin:
	make -C host-osx

release:
	test ! -f extension-$(RELEASE).zip
	test -z "`git status -s extension`"
	git clean -dfx extension
	zip -r extension-$(RELEASE).zip extension

test: detect
	python host-test/pipe-test.py -v
