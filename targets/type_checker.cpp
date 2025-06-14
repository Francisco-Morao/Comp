#include <string>
#include "targets/type_checker.h"
// #include "targets/symbol.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#include "udf_parser.tab.h"

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void udf::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void udf::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void udf::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

void udf::type_checker::do_nullptr_node(udf::nullptr_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, nullptr));
}

//---------------------------------------------------------------------------

void udf::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);

  if (node->argument()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));

  else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) 
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  
  else if (node->argument()->is_typed(cdk::TYPE_TENSOR))
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->argument()->type())->dims()));

  else
    throw std::string("wrong type in argument of unary expression");
}

void udf::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void udf::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_ScalarLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    throw std::string("integer expression expected in binary logical expression (left)");
  }

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    throw std::string("integer expression expected in binary logical expression (right)");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_GeneralLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  if (node->left()->type() != node->right()->type()) {
    throw std::string("same type expected on both sides of equality operator");
  }
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_BooleanLogicalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary expression");
  }

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary expression");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_PIDExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(node->left()->type());
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER)) {
    node->type(node->right()->type());
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT)); //??
  } else if (node->left()->is_typed(cdk::TYPE_UNSPEC)){
    node->type(node->right()->type());
    node->left()->type(node->right()->type());
  }
  //TENSORS
  else if (node->left()->is_typed(cdk::TYPE_TENSOR) && node->right()->is_typed(cdk::TYPE_TENSOR)){
    //check dims
    auto ltype = cdk::tensor_type::cast(node->left()->type());
    auto rtype = cdk::tensor_type::cast(node->right()->type());
    if (ltype->n_dims() != rtype->n_dims()) {
      throw std::string("wrong dimensions in tensor operation");
    }
    node->type(cdk::tensor_type::create(ltype->dims()));
  
  } else if (node->left()->is_typed(cdk::TYPE_TENSOR) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->left()->type())->dims()));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_TENSOR)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->right()->type())->dims()));
  } else if (node->left()->is_typed(cdk::TYPE_TENSOR) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->left()->type())->dims()));
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_TENSOR)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->right()->type())->dims()));
  } else {
    throw std::string("wrong type in binary expression");
  }
}

void udf::type_checker::do_IDExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->left()->is_typed(cdk::TYPE_UNSPEC) && node->right()->is_typed(cdk::TYPE_UNSPEC)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->left()->is_typed(cdk::TYPE_UNSPEC)){
    node->type(node->right()->type());
    node->left()->type(node->right()->type());
  }
  else if (node->left()->is_typed(cdk::TYPE_TENSOR) && node->right()->is_typed(cdk::TYPE_TENSOR)){
    //check dims
    auto ltype = cdk::tensor_type::cast(node->left()->type());
    auto rtype = cdk::tensor_type::cast(node->right()->type());
    if (ltype->n_dims() != rtype->n_dims()) {
      throw std::string("wrong dimensions in tensor operation");
    }
    node->type(cdk::tensor_type::create(ltype->dims()));
  
  } else if (node->left()->is_typed(cdk::TYPE_TENSOR) && node->right()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->left()->type())->dims()));
  } else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_TENSOR)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->right()->type())->dims()));
  } else if (node->left()->is_typed(cdk::TYPE_TENSOR) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->left()->type())->dims()));
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_TENSOR)) {
    node->type(cdk::tensor_type::create(cdk::tensor_type::cast(node->right()->type())->dims()));
  } else {
    throw std::string("wrong type in binary expression");
  }
}

void udf::type_checker::do_IntOnlyExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl + 2);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary operator (left)");
  }

  node->right()->accept(this, lvl + 2);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("integer expression expected in binary operator (right)");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void udf::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  do_PIDExpression(node, lvl);
}
void udf::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  do_PIDExpression(node, lvl);
}
void udf::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  do_IDExpression(node, lvl);
}
void udf::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  do_IDExpression(node, lvl);
}
void udf::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  do_IntOnlyExpression(node, lvl);
}

void udf::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_INT))
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    
  else if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }
  else {
    throw std::string("wrong type in unary logical expression");
  }
}

void udf::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}

void udf::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}

void udf::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}

void udf::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  do_ScalarLogicalExpression(node, lvl);
}

void udf::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  do_GeneralLogicalExpression(node, lvl);
}

void udf::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  do_GeneralLogicalExpression(node, lvl);
}

void udf::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  do_BooleanLogicalExpression(node, lvl);
}
void udf::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  do_BooleanLogicalExpression(node, lvl);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<udf::symbol> symbol = _symtab.find(id);

  if (symbol) {
    node->type(symbol->type());
  } 
  else {
    throw std::string("undeclared variable '" + id + "'");
  }
}

void udf::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

void udf::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl + 4);
  node->rvalue()->accept(this, lvl + 4);

  if (node->lvalue()->is_typed(cdk::TYPE_INT)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else {
      throw std::string("wrong assignment to integer");
    }
  }
  else if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT) || node->rvalue()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      node->rvalue()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    } else {
      throw std::string("wrong assignment to double");
    }
  } else if (node->lvalue()->is_typed(cdk::TYPE_STRING)) {
    if (node->rvalue()->is_typed(cdk::TYPE_STRING)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
      node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
    } else {
      throw std::string("wrong assignment to string");
    }
  } 
  else if (node->lvalue()->is_typed(cdk::TYPE_TENSOR)) {
    if (node->rvalue()->is_typed(cdk::TYPE_TENSOR)) {
      auto ltype = cdk::tensor_type::cast(node->lvalue()->type());
      auto rtype = cdk::tensor_type::cast(node->rvalue()->type());
      if (ltype->n_dims() != rtype->n_dims()) {
        throw std::string("wrong dimensions in tensor assignment");
      }
      for (size_t i = 0; i < ltype->n_dims(); i++) {
        if (ltype->dim(i) != rtype->dim(i)) {
          throw std::string("wrong dimensions in tensor assignment");
        }
      }
      node->type(cdk::tensor_type::create(ltype->dims()));
    } 
    else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {

      auto ltype = cdk::tensor_type::cast(node->lvalue()->type());
      auto rtype = cdk::tensor_type::cast(node->rvalue()->type());
      if (ltype->n_dims() != rtype->n_dims()) {
        throw std::string("wrong dimensions in tensor assignment");
      }
      for (size_t i = 0; i < ltype->n_dims(); i++) {
        if (ltype->dim(i) != rtype->dim(i)) {
          throw std::string("wrong dimensions in tensor assignment");
        }
      }
      node->type(cdk::tensor_type::create(ltype->dims()));
      node->rvalue()->type(cdk::tensor_type::create(ltype->dims()));
    } 
    else {
      throw std::string("wrong assignment to tensor");
    }
  } 
  else if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {

    if (node->rvalue()->is_typed(cdk::TYPE_INT) || node->rvalue()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_POINTER));
    } else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_ERROR));
      node->rvalue()->type(cdk::primitive_type::create(4, cdk::TYPE_ERROR));
    } else if (node->rvalue()->is_typed(cdk::TYPE_POINTER)) {

      int lt = 0, rt = 0;
      auto ltype = node->lvalue()->type();
      while (ltype->name() == cdk::TYPE_POINTER) {
        std::cout << "lvalue" << std::endl;
        lt++;
        ltype = cdk::reference_type::cast(ltype)->referenced();
      } //depth do pointeiro do leftvalue

      auto rtype = node->rvalue()->type();
      while (rtype != nullptr && rtype->name() == cdk::TYPE_POINTER) {
        std::cout << "rvalue" << std::endl;
        rt++;
        rtype = cdk::reference_type::cast(rtype)->referenced();
      } //depth do ponteiro do rvalue

      bool compatible = (lt == rt) && (rtype == nullptr || (rtype != nullptr && ltype->name() == rtype->name()));
      if (!compatible) 
        throw std::string("wrong assignment to pointer - incompatible types");
      
      node->type(node->rvalue()->type());
    }else{
      throw std::string("wrong assignment to pointer");
    }
  }
  else if (node->lvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    if(node->rvalue()->is_typed(cdk::TYPE_INT) || node->rvalue()->is_typed(cdk::TYPE_DOUBLE) 
        || node->rvalue()->is_typed(cdk::TYPE_STRING) || node->rvalue()->is_typed(cdk::TYPE_TENSOR)) {
      node->type(node->rvalue()->type());
      node->lvalue()->type(node->rvalue()->type());
    } 
    else
      throw std::string("wrong assignment to auto type");
  } 
  else {
    throw std::string("unknown type in assignment");
  }
  
}

void udf::type_checker::do_evaluation_node(udf::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void udf::type_checker::do_print_node(udf::print_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_input_node(udf::input_node *const node, int lvl) {
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_if_node(udf::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void udf::type_checker::do_if_else_node(udf::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_block_node(udf::block_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_continue_node(udf::continue_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_return_node(udf::return_node * const node, int lvl) {
  if (node->returnval()) {
    if (_function->type() != nullptr && _function->is_typed(cdk::TYPE_VOID)) 
      throw std::string("initializer specified for void function.");

    node->returnval()->accept(this, lvl + 2);

    // function is auto: copy type of first return expression
    if (_function->type() == nullptr) {
      _function->set_type(node->returnval()->type());
      return; // simply set the type
    }
  }

  if (_inBlockReturnType == nullptr) {
    return;
  } 
  else {
    if (_inBlockReturnType != node->returnval()->type()) {
      throw std::string("all return statements in a function must return the same type.");
    }
  }

  std::cout << "FUNCT TYPE " << (_function->type() == nullptr ? "auto" : cdk::to_string(_function->type())) << std::endl;
  std::cout << "RETVAL TYPE " << cdk::to_string(node->returnval()->type()) << std::endl;

  if (_function->is_typed(cdk::TYPE_INT)) {
    if (!node->returnval()->is_typed(cdk::TYPE_INT)) 
      throw std::string("wrong type for initializer (integer expected).");
  }
  else if (_function->is_typed(cdk::TYPE_DOUBLE)) {
    if (!node->returnval()->is_typed(cdk::TYPE_INT) && !node->returnval()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("wrong type for initializer (integer or double expected).");
    }
  }
  else if (_function->is_typed(cdk::TYPE_STRING)) {
    if (!node->returnval()->is_typed(cdk::TYPE_STRING)) {
      throw std::string("wrong type for initializer (string expected).");
    }
  }
  else if (_function->is_typed(cdk::TYPE_TENSOR)) {
    if (!node->returnval()->is_typed(cdk::TYPE_TENSOR)) {
      throw std::string("wrong type for initializer (tensor expected).");
    }
  }
  else if (_function->is_typed(cdk::TYPE_VOID)) {}
  else if (_function->is_typed(cdk::TYPE_POINTER)) {
    int ft = 0, rt = 0;
    auto ftype = _function->type();
    while (ftype->name() == cdk::TYPE_POINTER) {
      ft++;
      ftype = cdk::reference_type::cast(ftype)->referenced();
    } //depth do pointeiro do funcao 
  
    auto rtype = node->returnval()->type();
    while (rtype != nullptr && rtype->name() == cdk::TYPE_POINTER) {
      rt++;
      rtype = cdk::reference_type::cast(rtype)->referenced();
    } //depth do ponteiro do retorno

    std::cout << "FUNCT TYPE " << cdk::to_string(_function->type()) << " --- " << ft << " -- " << ftype->name() << std::endl;
    std::cout << "RETVAL TYPE " << cdk::to_string(node->returnval()->type()) << " --- " << rt << " -- " << cdk::to_string(rtype)
        << std::endl;

    bool compatible = (ft == rt) && (rtype == nullptr || (rtype != nullptr && ftype->name() == rtype->name()));
    if (!compatible) 
      throw std::string("wrong type for return expression (pointer expected).");

  } else {
    throw std::string("unknown type for initializer.");
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_break_node(udf::break_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_definition_node(udf::function_definition_node * const node, int lvl) {
  std::string id;

  if (node->identifier() == "udf")
    id = "_main";
  else if (node->identifier() == "_main")
    id = "._main";
  else 
    id = node->identifier();

  _inBlockReturnType = nullptr;

  // remember symbol so that args know
  auto function = udf::make_symbol(false, node->qualifier(), node->type(), id, false, true, true);

  if (id == "_main" && node->arguments()->size() != 0)
    throw std::string("udf function cannot have arguments.:" + node->arguments()->size());
  
  std::vector < std::shared_ptr < cdk::basic_type >> argtypes;
  for (size_t ax = 0; ax < node->arguments()->size(); ax++)
    argtypes.push_back(node->argument(ax)->type());
  function->set_argument_types(argtypes);

  std::shared_ptr<udf::symbol> previous = _symtab.find(function->name());
 if (previous) {
    if (previous->forward()
        && ((previous->qualifier() == tPUBLIC && node->qualifier() == tPUBLIC)
            || (previous->qualifier() == tPRIVATE && node->qualifier() == tPRIVATE))) {
      _symtab.replace(function->name(), function);
      _parent->set_new_symbol(function);
    } else {
      throw std::string("conflicting definition for '" + function->name() + "'");
    }
  } 
  else {
    _symtab.insert(function->name(), function);
    _parent->set_new_symbol(function);
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_declaration_node(udf::function_declaration_node * const node, int lvl) {
  std::string id;

  if (node->identifier() == "udf")
    id = "_main";
  else if (node->identifier() == "_main")
    id = "._main";
  else
    id = node->identifier();

  // remember symbol so that args know
  auto function = udf::make_symbol(false, node->qualifier(), node->type(), id, false, true, true);

  std::vector < std::shared_ptr < cdk::basic_type >> argtypes;
  for (size_t ax = 0; ax < node->arguments()->size(); ax++)
    argtypes.push_back(node->argument(ax)->type());
  function->set_argument_types(argtypes);

  std::shared_ptr<udf::symbol> previous = _symtab.find(function->name());
  if(previous) {
    if(previous->isFunction()) {
      if(!previous->forward())
        throw std::string("'" + function->name() + "' has already been defined.");
      if(function->qualifier() != previous->qualifier())
        throw std::string("'" + function->name() + "' declaration has a different qualifier.");
      if(function->type()->name() != previous->type()->name()) 
        throw std::string("'" + function->name() + "' redeclaration has a different return type.");

      if(previous->number_of_arguments() == function->number_of_arguments()) {
        for (size_t ax = 0; ax < node->arguments()->size(); ax++) {
          if (function->argument_type(ax) == previous->argument_type(ax)) 
            continue;
          throw std::string("type mismatch for argument " + std::to_string(ax + 1) + " of '" + id + "'.");
        }
      } 
      else {
        throw std::string("number of arguments in declaration (" + std::to_string(function->number_of_arguments())
            + ") must match definition (" + std::to_string(previous->number_of_arguments()) + ").");
      }
      _symtab.replace(function->name(), function);
    }
    else
      throw std::string("'" + function->name() + "' has already been declared as a different type of symbol.");
  }
  else {
    _symtab.insert(function->name(), function);
    _parent->set_new_symbol(function);
  }  
}

//---------------------------------------------------------------------------

void udf::type_checker::do_function_call_node(udf::function_call_node * const node, int lvl) {
  ASSERT_UNSPEC;

  const std::string &id = node->identifier();
  auto symbol = _symtab.find(id);
  if (symbol == nullptr) 
    throw std::string("symbol '" + id + "' is undeclared.");
  if (!symbol->isFunction()) 
    throw std::string("symbol '" + id + "' is not a function.");

  node->type(symbol->type());
  if (node->arguments()->size() == symbol->number_of_arguments()) {
    node->arguments()->accept(this, lvl + 4);
    for (size_t ax = 0; ax < node->arguments()->size(); ax++) {
      if (node->argument(ax)->type() == symbol->argument_type(ax)) 
        continue;
      if (symbol->argument_is_typed(ax, cdk::TYPE_DOUBLE) && node->argument(ax)->is_typed(cdk::TYPE_INT)) 
        continue;
      throw std::string("type mismatch for argument " + std::to_string(ax + 1) + " of '" + id + "'.");
    }
  } else {
    throw std::string(
        "number of arguments in call (" + std::to_string(node->arguments()->size()) + ") must match declaration ("
            + std::to_string(symbol->number_of_arguments()) + ").");
  }

}

//---------------------------------------------------------------------------

void udf::type_checker::do_for_node(udf::for_node * const node, int lvl) {
  
  node->init()->accept(this, lvl + 4);
  node->condition()->accept(this, lvl + 4);
  node->iteration()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void udf::type_checker::do_sizeof_node(udf::sizeof_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_index_node(udf::index_node * const node, int lvl) {
  ASSERT_UNSPEC;
  std::shared_ptr < cdk::reference_type > btype;

  if (node->base()) {
    node->base()->accept(this, lvl + 2);
    btype = cdk::reference_type::cast(node->base()->type());
    if (!node->base()->is_typed(cdk::TYPE_POINTER)) throw std::string("pointer expression expected in index left-value");
  } else {
    btype = cdk::reference_type::cast(_function->type());
    if (!_function->is_typed(cdk::TYPE_POINTER)) throw std::string("return pointer expression expected in index left-value");
  }

  node->index()->accept(this, lvl + 2);
  if (!node->index()->is_typed(cdk::TYPE_INT)) throw std::string("integer expression expected in left-value index");

  node->type(btype->referenced());
}

//---------------------------------------------------------------------------

void udf::type_checker::do_address_of_node(udf::address_of_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl + 2);
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_alloc_node(udf::alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if(node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    udf::input_node *input = dynamic_cast<udf::input_node *>(node->argument());

    if(input != nullptr)
      node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("Unknown node with unspecified type.");
  }
  else if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("Integer expression expected in allocation expression.");

  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_var_declaration_node(udf::var_declaration_node * const node, int lvl) {

  if (node->type() == nullptr){ // auto type
    node->init()->accept(this, lvl + 2);
    if (node->init()->is_typed(cdk::TYPE_UNSPEC))
      node->init()->type(cdk::primitive_type::create(4, cdk::TYPE_INT)); 
    else if (node->init()->is_typed(cdk::TYPE_POINTER)) { //
      auto ref = cdk::reference_type::cast(node->init()->type());
      if (ref->referenced()->name() == cdk::TYPE_UNSPEC) {
        node->init()->type(cdk::reference_type::create(4,
            cdk::primitive_type::create(4, cdk::TYPE_INT)));
      }
    }
    node->type(node->init()->type());
  }
  else{
    if (node->init() != nullptr) {
      node->init()->accept(this, lvl + 2);
      if (node->is_typed(cdk::TYPE_INT)) {
        if (!node->init()->is_typed(cdk::TYPE_INT)) 
          throw std::string("wrong type for initializer (integer expected).");
      }
      
      else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (!node->init()->is_typed(cdk::TYPE_INT) && !node->init()->is_typed(cdk::TYPE_DOUBLE))
          throw std::string("wrong type for initializer (integer or double expected).");
      } 
      else if (node->is_typed(cdk::TYPE_STRING)) {
        if (!node->init()->is_typed(cdk::TYPE_STRING)) {
          throw std::string("wrong type for initializer (string expected).");
        }
      } 
      else if (node->is_typed(cdk::TYPE_POINTER)) {
        std::cout << "TYPE POINTER" << std::endl;
        if (!node->init()->is_typed(cdk::TYPE_POINTER)) {
          auto in = (cdk::literal_node<int>*)node->init();
          if (in == nullptr || in->value() != 0) 
            throw std::string("wrong type for initializer (pointer expected).");
        }
      }
      else if (node->is_typed(cdk::TYPE_TENSOR)) {
        if (!node->init()->is_typed(cdk::TYPE_TENSOR)) {
          throw std::string("wrong type for initializer (tensor expected).");
        }
      } 
      else
        throw std::string("unknown type for initializer.");
    }
  }

  const std::string &id = node->identifier();
  auto symbol = udf::make_symbol(false, node->qualifier(), node->type(), id, (bool)node->init(), false);
  if (_symtab.insert(id, symbol))
    _parent->set_new_symbol(symbol);  // advise parent that a symbol has been inserted
}

//---------------------------------------------------------------------------


std::vector<size_t> get_tensor_shape(udf::tensor_node * const node, bool &ok) {
  if (!node)
    return {};
	
  size_t sz = node->fields() ? node->fields()->size() : 0;
  std::vector<size_t> shape = {sz};

  for (size_t i = 0; i < sz; i++) {
    auto field = dynamic_cast<udf::tensor_node*>(node->fields()->node(i));
    if (field) {
      bool sub_ok = true;
      std::vector<size_t> sub_shape = get_tensor_shape(field, sub_ok);
      if (!sub_ok) {
        ok = false;
        return {};
      }
      if (i == 0) {
        shape.insert(shape.end(), sub_shape.begin(), sub_shape.end());
      }
    } 
  }
  return shape;
}

void udf::type_checker::do_tensor_node(udf::tensor_node * const node, int lvl) {
  ASSERT_UNSPEC;
  
  for (size_t i = 0; i < node->fields()->size(); i++) {
    cdk::expression_node *expr = dynamic_cast<cdk::expression_node *>(node->fields()->node(i));
    expr->accept(this, lvl + 2);

    if (!expr->is_typed(cdk::TYPE_INT) && !expr->is_typed(cdk::TYPE_DOUBLE) && !expr->is_typed(cdk::TYPE_TENSOR)) {
      throw std::string("Tensor elements must be of type int, double or tensor.");
    }
  }
  
  bool ok = true;
  std::vector<size_t> dims = get_tensor_shape(node, ok);

  for (size_t dim : dims) {
    std::cout << dim << " ";
  }

  if (dims.empty()) {
    throw std::string("Error: Non-numeric field in tensor");
    ok = false;
  }
  else if (ok) {
    auto tensor_type = cdk::tensor_type::create(dims);
    node->type(tensor_type);
  } 
  else 
    throw std::string("Error: Not a valid rectangular tensor");
}


//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_index_node(udf::tensor_index_node * const node, int lvl) {
  node->base()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);

  if(node->base()->is_typed(cdk::TYPE_TENSOR)) {
    for (size_t i = 0; i < node->index()->size(); i++){
      node->index()->node(i)->accept(this, lvl + 2);
      cdk::expression_node *expr = dynamic_cast<cdk::expression_node *>(node->index()->node(i));
      if (expr == nullptr)
        throw std::string("tensor index must be an expression.");
      if (!expr->is_typed(cdk::TYPE_INT))
        throw std::string("wrong type for tensor index (integer expected).");
    }
  }
  else
    throw std::string("wrong type for tensor dimension (tensor expected).");

  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_capacity_node(udf::tensor_capacity_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->tensor()->accept(this, lvl + 2);

  if(!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string("wrong type for tensor capacity (tensor expected).");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_rank_node(udf::tensor_rank_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->tensor()->accept(this, lvl + 2);

  if(!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string("wrong type for tensor dimension (tensor expected).");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_dim_node(udf::tensor_dim_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->tensor()->accept(this, lvl + 2);
  node->dim()->accept(this, lvl + 2);

  if(node->tensor()->is_typed(cdk::TYPE_TENSOR)) {
    if(!node->dim()->is_typed(cdk::TYPE_INT))
      throw std::string("wrong type for tensor dimension (integer expected).");
  }
  else
    throw std::string("wrong type for tensor dimension (tensor expected).");

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_dims_node(udf::tensor_dims_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->tensor()->accept(this, lvl + 2);

  if(!node->tensor()->is_typed(cdk::TYPE_TENSOR))
    throw std::string("wrong type for tensor capacity (tensor expected).");

  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_contraction_node(udf::tensor_contraction_node * const node, int lvl) {
  ASSERT_UNSPEC;
  
  node->tensor1()->accept(this, lvl + 2);
  node->tensor2()->accept(this, lvl + 2);
  // last dimension of left tensor must match first dimension of right tensor
  if (node->tensor1()->is_typed(cdk::TYPE_TENSOR) && node->tensor2()->is_typed(cdk::TYPE_TENSOR)) {
    auto left_type = cdk::tensor_type::cast(node->tensor1()->type());
    auto right_type = cdk::tensor_type::cast(node->tensor2()->type());
    if (left_type->n_dims() == 0 || right_type->n_dims() == 0) {
      throw std::string("tensor contraction requires non-empty tensors.");
    }
    if (left_type->dim(left_type->n_dims() - 1) != right_type->dim(0)) {
      throw std::string("last dimension of left tensor must match first dimension of right tensor.");
    }
    std::vector<size_t> new_dims;
    new_dims.reserve(left_type->n_dims() + right_type->n_dims() - 2);
    for (size_t i = 0; i < left_type->n_dims() - 1; ++i) { // all but last dimension of left tensor
      new_dims.push_back(left_type->dim(i));
    }
    for (size_t i = 1; i < right_type->n_dims(); ++i) { // all but first dimension of right tensor
      new_dims.push_back(right_type->dim(i));
    }
    node->type(cdk::tensor_type::create(new_dims));
    std::cout << "CONTRACTION TYPE: ";
    for (size_t dim : new_dims) {
      std::cout << dim << " ";
    }
    std::cout << std::endl;
  } else {
    throw std::string("tensor contraction requires two tensor operands.");
  }
}

//---------------------------------------------------------------------------

void udf::type_checker::do_tensor_reshape_node(udf::tensor_reshape_node * const node, int lvl) {
  ASSERT_UNSPEC;
  
  node->tensor()->accept(this, lvl + 2);
  node->arguments()->accept(this, lvl + 2);
  if (!node->tensor()->is_typed(cdk::TYPE_TENSOR)) {
    throw std::string("tensor reshape requires a tensor operand.");
  }
  if (node->arguments()->size() == 0) {
    throw std::string("tensor reshape requires at least one dimension argument.");
  }
  //Asign dims to node
  std::vector<size_t> new_dims;
  for (size_t i = node->arguments()->size(); i-- > 0;) {
    auto arg = dynamic_cast<cdk::literal_node<int> *>(node->arguments()->node(i));
    if (arg == nullptr || !arg->is_typed(cdk::TYPE_INT)) {
      throw std::string("tensor reshape requires integer dimension arguments.");
    }
    new_dims.push_back(arg->value());
  }
  if (new_dims.size() == 0) {
    throw std::string("tensor reshape requires at least one dimension argument.");
  }
  node->type(cdk::tensor_type::create(new_dims));
  std::cout << "RESHAPE TYPE: ";
  for (size_t dim : new_dims) {
    std::cout << dim << " ";
  }
  std::cout << std::endl;
}

//---------------------------------------------------------------------------