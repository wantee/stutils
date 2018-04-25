CU_SUFFIX = cu
NVCC = nvcc

ROOT_DIR = $(dir $(lastword $(MAKEFILE_LIST)))
include $(addprefix $(ROOT_DIR)/,common.mk)

NVDEPFLAGS = -MT $@ -M
NVCOMPILE.flags.cu = $(CPPFLAGS) $(NVFLAGS)
NVCOMPILE.cu = $(NVCC) $(NVCOMPILE.flags.cu) -dlink -c
ifndef STATIC_LINK
NVCOMPILE.cu += --compiler-options '-fPIC'
endif

$(DEP_DIR)/%.$(CU_SUFFIX).d : %.$(CU_SUFFIX) | $(PREFIX)inc
	@mkdir -p "$(dir $@)"
	$(NVCC) $(CPPFLAGS) $(NVDEPFLAGS) $< > $(DEP_DIR)/$*.$(CU_SUFFIX).Td
	mv -f $(DEP_DIR)/$*.$(CU_SUFFIX).Td $(DEP_DIR)/$*.$(CU_SUFFIX).d

$(OBJ_DIR)/%.$(CU_SUFFIX).o : %.$(CU_SUFFIX) $(DEP_DIR)/%.$(CU_SUFFIX).d $(OUT_REV)
	@mkdir -p "$(dir $@)"
	$(NVCOMPILE.cu) $(OUTPUT_OPTION) $<
