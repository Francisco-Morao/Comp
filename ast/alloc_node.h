#ifndef __UDF_AST_ALLOC_NODE_H__
#define __UDF_AST_ALLOC_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/unary_operation_node.h>

namespace udf {

  /**
   * Class for describing allocation of nodes.
   */
  class alloc_node : public cdk::unary_operation_node  {

  public:
    alloc_node(int lineno, cdk::expression_node *argument) :
        cdk::unary_operation_node (lineno, argument) {}

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_alloc_node(this, level);
    }

  };

} // udf

#endif
