#ifndef __UDF_AST_SIZEOF_NODE_H__
#define __UDF_AST_SIZEOF_NODE_H__

#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Class for describing sizeof nodes.
   */
  class sizeof_node: public cdk::expression_node {
    cdk::expression_node *_argument;

  public:
    sizeof_node(int lineno, cdk::expression_node *argument) :
        cdk::expression_node(lineno), _argument(argument) {}

    cdk::expression_node* argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_sizeof_node(this, level);
    }

  };

} // udf

#endif
