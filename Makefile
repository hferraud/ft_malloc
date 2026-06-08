NAME        	:=	malloc
BUILD_DIR   	:=	.build
BUILD_TEST_DIR  :=	.build-test
BIN_DIR			:=	bin
LIB_DIR			:=	lib

.PHONY: all
all:
	cmake --build $(BUILD_DIR)

.PHONY: configure
configure:
	cmake -S . -B $(BUILD_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BUILD_TEST_DIR)

.PHONY: fclean
fclean:
	rm -rf $(BIN_DIR)
	rm -rf $(LIB_DIR)

.PHONY: re
re: fclean
	$(MAKE) configure
	$(MAKE) all

.PHONY: configure_test
configure_test:
	cmake -S . -B $(BUILD_TEST_DIR) -DBUILD_TEST=ON

.PHONY: build_test
build_test:
	cmake --build $(BUILD_TEST_DIR)

.PHONY: test
test: build_test
	ctest --test-dir $(BUILD_TEST_DIR)

.PHONY: test_verbose
test_verbose: build_test
	ctest --test-dir $(BUILD_TEST_DIR) --verbose
