build:
	msbuild host-windows\host-windows.sln

test: build
	python host-test\pipe-test.py -v
