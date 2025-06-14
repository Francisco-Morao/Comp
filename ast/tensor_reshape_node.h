#ifndef __UDF_AST_TENSOR_RESHAPE_NODE_H__
#define __UDF_AST_TENSOR_RESHAPE_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

/**
 * Class for describing tensor reshape nodes.
 */
class tensor_reshape_node : public cdk::expression_node {
  cdk::expression_node *_tensor;
  cdk::sequence_node *_arguments;

public:
  inline tensor_reshape_node(int lineno, cdk::expression_node *tensor,
                             cdk::sequence_node *arguments)
      : cdk::expression_node(lineno), _tensor(tensor), _arguments(arguments) {}

  cdk::expression_node *tensor() { return _tensor; }

  cdk::sequence_node *arguments() { return _arguments; }

  cdk::expression_node *argument(size_t ix) {
    return dynamic_cast<cdk::expression_node *>(_arguments->node(ix));
  }
  size_t length() { return _arguments->size(); }

  void accept(basic_ast_visitor *sp, int level) {
    sp->do_tensor_reshape_node(this, level);
  }
};

} // namespace udf

#endif