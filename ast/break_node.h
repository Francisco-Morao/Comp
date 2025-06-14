#ifndef __UDF_AST_BREAK_NODE_H__
#define __UDF_AST_BREAK_NODE_H__

#include <cdk/ast/basic_node.h>

namespace udf {

  /**
   * Class for describing stop nodes.
   */
  class break_node : public cdk::basic_node {

  public:
    break_node(int lineno) :
      cdk::basic_node(lineno) {}

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_break_node(this, level);
    }

  };

} // udf

#endif
