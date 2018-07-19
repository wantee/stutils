# options like '-I /path/to/headers' and '-D_MACRO_=1' should appended to CPPFLAGS
# options like '-L /path/to/libs' should appended to LDFLAGS
# options like '-llib' should appended to LDLIBS
#
# CXX related options should be appended to CXXFLAGS
# C related options should be appended to CFLAGS
# NVCC related options should be appended to NVFLAGS
ifndef _STUTILS_COMMON_MK_
_STUTILS_COMMON_MK_ = true

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.$(SRC_SUFFIX).Td
POSTCOMPILE = mv -f $(DEP_DIR)/$*.$(SRC_SUFFIX).Td $(DEP_DIR)/$*.$(SRC_SUFFIX).d

COMPILE.flags = $(DEPFLAGS) $(CPPFLAGS) $(TARGET_ARCH)
LINK.flags = $(LDFLAGS) $(LDLIBS) $(TARGET_ARCH)

LINK.o = $(LD) $(LINK.flags)

ifdef STATIC_LINK
  TARGET_LIB=$(OBJ_DIR)/lib$(PROJECT).a
else
  ifeq ($(shell uname -s),Darwin)
  TARGET_LIB = $(OUTLIB_DIR)/lib$(PROJECT).dylib
#  SO_FLAGS = -dynamiclib -install_name $(abspath $(TARGET_LIB))
  SO_FLAGS = -dynamiclib -install_name "@rpath/$(notdir $(TARGET_LIB))"
  else
  TARGET_LIB = $(OUTLIB_DIR)/lib$(PROJECT).so
  SO_FLAGS = -shared -Wl,-rpath,'$$ORIGIN'
  endif
endif
TARGET_BINS = $(addprefix $(OUTBIN_DIR)/,$(BINS))

ifdef REV_INC
OUT_REV = $(OUTINC_DIR)/$(PROJECT)/$(REV_INC)
else
OUT_REV =
endif
OUT_INCS = $(addprefix $(OUTINC_DIR)/$(PROJECT)/,$(INCS))

.PHONY: $(PREFIX)all $(PREFIX)inc $(PREFIX)rev
.PHONY: $(PREFIX)test $(PREFIX)val-test
.PHONY: $(PREFIX)clean $(PREFIX)clean-bin

$(PREFIX)all: $(PREFIX)inc $(TARGET_LIB) $(TARGET_BINS)

ifdef REV_INC

# always check revision
$(PREFIX)rev: $(REPLACE_REVISION)
	@$(REPLACE_REVISION) $(REV_INC) $(OUT_REV) $(GIT_COMMIT_MACRO)

# copy REV_HEADER when updated
$(OUT_REV): $(REPLACE_REVISION) $(REV_INC)
	$(REPLACE_REVISION) $(REV_INC) $(OUT_REV) $(GIT_COMMIT_MACRO) force

endif

$(PREFIX)inc: $(OUT_INCS) $(PREFIX)rev $(OUT_REV)

$(OUT_INCS) : $(OUTINC_DIR)/$(PROJECT)/%.h : %.h
	@mkdir -p "$(dir $@)"
	cp "$<" "$@"

ifdef STATIC_LINK

$(TARGET_LIB) : $(patsubst %,$(OBJ_DIR)/%.o,$(SRCS))
	@mkdir -p "$(dir $@)"
	ar rcs $@ $^

else

$(TARGET_LIB) : $(patsubst %,$(OBJ_DIR)/%.o,$(SRCS))
	@mkdir -p "$(dir $@)"
	$(LINK.o) $(SO_FLAGS) $^ -o $@

endif

$(DEP_DIR)/%.d: ;
.PRECIOUS: $(DEP_DIR)/%.d

-include $(patsubst %,$(DEP_DIR)/%.d,$(SRCS))

$(TARGET_BINS) : $(TARGET_LIB) | $(PREFIX)inc

-include $(patsubst %,$(DEP_DIR)/%.d,$(basename $(BINS)))

TARGET_TESTS = $(addprefix $(OBJ_DIR)/,$(TESTS))

$(TARGET_TESTS) : $(OUT_INCS) $(OUT_REV) $(TARGET_LIB)

-include $(patsubst %,$(DEP_DIR)/%.d,$(basename $(TESTS)))

$(PREFIX)test: $(TARGET_TESTS)
	@result=0; \
     for x in $(TARGET_TESTS); do \
       printf "Running $$x ..."; \
       ./$$x >/dev/null 2>&1; \
       if [ $$? -ne 0 ]; then \
         echo "... FAIL $$x"; \
         result=1; \
       else \
         echo "... SUCCESS"; \
       fi; \
     done; \
     exit $$result

TARGET_VAL_TESTS = $(addprefix $(OBJ_DIR)/,$(VAL_TESTS))

$(PREFIX)val-test: $(TARGET_VAL_TESTS)
	@result=0; \
     for x in $(TARGET_VAL_TESTS); do \
       printf "Valgrinding $$x ..."; \
       valgrind ./$$x; \
       if [ $$? -ne 0 ]; then \
         echo "... FAIL $$x"; \
         result=1; \
       else \
         echo "... SUCCESS"; \
       fi; \
     done; \
     exit $$result

ifdef BINS

$(PREFIX)clean-bin:
	rm -f $(addprefix $(OUTBIN_DIR)/,$(BINS))
	rm -rf $(addsuffix .dSYM,$(addprefix $(OUTBIN_DIR)/,$(BINS)))
	rmdir -p $(addprefix $(OUTBIN_DIR)/,$(filter-out ./,$(dir $(BINS)))) 2>/dev/null || true

else

$(PREFIX)clean-bin:

endif

ifdef TESTS

$(PREFIX)clean-test:
	rm -f $(TARGET_TESTS)
	rm -rf $(addsuffix .dSYM,$(TARGET_TESTS))

else

$(PREFIX)clean-test:

endif

ifdef clean-static
$(PREFIX)clean: clean-static
endif

$(PREFIX)clean: $(PREFIX)clean-bin
	rm -rf $(OBJ_DIR)
	rm -rf $(DEP_DIR)
	rm -f $(TARGET_LIB)
	rmdir -p $(OUTLIB_DIR) 2>/dev/null || true
	rm -rf $(OUTINC_DIR)/$(PROJECT)
	rmdir -p $(OUTINC_DIR) 2>/dev/null || true

endif
