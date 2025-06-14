#ifndef __UDF_AST_RETURN_NODE_H__
#define __UDF_AST_RETURN_NODE_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Class for describing returning nodes.
   */
  class return_node : public cdk::basic_node {
    cdk::expression_node *_returnval; 

  public:
    return_node(int lineno, cdk::expression_node *returnval = nullptr) :
        cdk::basic_node(lineno), _returnval(returnval) {
    }

    cdk::expression_node *returnval() {
        return _returnval;
    }
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_return_node(this, level);
    }

  };

} // udf

#endif
