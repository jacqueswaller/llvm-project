set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyEvolutionModule
  EvolutionModule.cpp
  ProTypeMemberInitCheck.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangSerialization
  clangTidy
  clangTidyMiscModule
  clangTidyModernizeModule
  clangTidyReadabilityModule
  clangTidyUtils
  clangTooling
  )
