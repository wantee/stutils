CU_SUFFIX = cu
NVCC = nvcc

ROOT_DIR = $(dir $(lastword $(MAKEFILE_LIST)))
include $(addprefix $(ROOT_DIR)/,common.mk)

CPPFLAGS += -I/usr/local/cuda/include
LDFLAGS += -L/usr/local/cuda/lib64
LDLIBS += -lcudart -lcublas -lcudnn -lcurand -lcusparse

NVDEPFLAGS = -MT $@ -M
NVCOMPILE.flags.cu = $(CPPFLAGS) $(NVFLAGS)
NVCOMPILE.cu = $(NVCC) $(NVCOMPILE.flags.cu) -c
ifndef STATIC_LINK
NVCOMPILE.cu += --compiler-options '-fPIC'
endif

# FIXME: following makefile rule WILL run no matter what target given by
#        command line. This is because that make will first updating all
#        makefiles included (i.e., the $(DEP_DIR)/%.$(CU_SUFFIX).d)
$(DEP_DIR)/%.$(CU_SUFFIX).d : %.$(CU_SUFFIX) | $(PREFIX)inc
	@mkdir -p "$(dir $@)"
	$(NVCC) $(CPPFLAGS) $(NVDEPFLAGS) $< > $(DEP_DIR)/$*.$(CU_SUFFIX).Td
	mv -f $(DEP_DIR)/$*.$(CU_SUFFIX).Td $(DEP_DIR)/$*.$(CU_SUFFIX).d

$(OBJ_DIR)/%.$(CU_SUFFIX).o : %.$(CU_SUFFIX) $(DEP_DIR)/%.$(CU_SUFFIX).d $(OUT_REV)
	@mkdir -p "$(dir $@)"
	$(NVCOMPILE.cu) $(OUTPUT_OPTION) $<
