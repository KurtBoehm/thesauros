use_iokit=false
build=build
SETUP_BASE=meson setup -Dbuild_tests=true -Dbuild_bench=true -Duse_iokit=$(use_iokit) --wrap-mode=forcefallback

clear:
	rm -rf $(build)
setup-opt: clear
	$(SETUP_BASE) --warnlevel 1 --optimization=3 -Db_ndebug=true $(build)
setup-opt-debug: clear
	$(SETUP_BASE) --warnlevel 1 --optimization=3 -Ddebug=true $(build)
setup-debug: clear
	$(SETUP_BASE) --warnlevel 3 --buildtype=debug $(build)
