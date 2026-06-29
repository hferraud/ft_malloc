ifeq ($(HOSTTYPE),)
	export HOSTTYPE=$(shell uname -m)_$(shell uname -s)
endif

NAME        	:=	libft_malloc_$(HOSTTYPE)
BUILD_DIR   	:=	.build
BUILD_TEST_DIR  :=	.build-test
BIN_DIR			:=	bin
LIB_DIR			:=	lib

.PHONY: all
all: $(NAME)

.PHONY: $(NAME)
$(NAME): $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

$(BUILD_DIR):
	cmake -S . -B $(BUILD_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BUILD_TEST_DIR)

.PHONY: fclean
fclean: clean
	rm -rf $(BIN_DIR)
	rm -rf $(LIB_DIR)

.PHONY: re
re: fclean
	$(MAKE) all

$(BUILD_TEST_DIR):
	cmake -S . -B $(BUILD_TEST_DIR) -DBUILD_TEST=ON

.PHONY: build_test
build_test: $(BUILD_TEST_DIR)
	cmake --build $(BUILD_TEST_DIR)

.PHONY: test
test: build_test
	ctest --test-dir $(BUILD_TEST_DIR)

.PHONY: test_verbose
test_verbose: build_test
	ctest --test-dir $(BUILD_TEST_DIR) --verbose
