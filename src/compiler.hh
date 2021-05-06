#pragma once
#include <string>
#include <vector>

#include "ast.hh"

void compile (const std::string& filename, std::vector<Expr*>& exprs);
