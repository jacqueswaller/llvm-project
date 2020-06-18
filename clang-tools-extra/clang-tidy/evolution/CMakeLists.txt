set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTidyEvolutionModule
  AssertUsageCheck.cpp
  EvolutionModule.cpp
  ContainerFirstElementAccessCheck.cpp
  OverloadingWithVirtualCheck.cpp
  ProTypeMemberInitCheck.cpp
  SymbolNonCopyingConstructorCheck.cpp
  ZeroVectorInitCheck.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTidy
  clangTidyUtils
  clangTooling
  )
