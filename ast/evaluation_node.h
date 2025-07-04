#ifndef __UDF_AST_EVALUATION_NODE_H__
#define __UDF_AST_EVALUATION_NODE_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace udf {

/**
 * Class for describing evaluation nodes.
 */
  class evaluation_node : public cdk::basic_node {
    cdk::expression_node *_argument;

  public:
    evaluation_node(int lineno, cdk::expression_node *argument)
        : cdk::basic_node(lineno), _argument(argument) {}

    cdk::expression_node *argument() { return _argument; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_evaluation_node(this, level);
    }
  };

} // namespace udf

#endif
