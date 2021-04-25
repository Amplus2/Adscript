#ifndef COMPILER_HH_
#define COMPILER_HH_

#include <string>
#include <vector>

#include "ast.hh"

void compile (const std::string& filename, std::vector<Expr*>& exprs);

#endif
