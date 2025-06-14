#ifndef __UDF_AST_INPUT_NODE_H__
#define __UDF_AST_INPUT_NODE_H__

#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Class for describing input nodes.
   */
  class input_node : public cdk::expression_node {

    public:
      input_node(int lineno) : cdk::expression_node(lineno) {}

      void accept(basic_ast_visitor *sp, int level) {
        sp->do_input_node(this, level);
      }
  };

} // namespace udf

#endif