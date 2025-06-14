#ifndef __UDF_AST_PRINT_NODE_H__
#define __UDF_AST_PRINT_NODE_H__

#include <cdk/ast/sequence_node.h>
#include <cdk/ast/basic_node.h>

namespace udf {

  /**
   * Class for describing print nodes.
   */
  class print_node : public cdk::basic_node {
    cdk::sequence_node *_argument;
    bool _newline;

  public:
    print_node(int lineno, cdk::sequence_node *argument, bool newline = false)
        : cdk::basic_node(lineno), _argument(argument), _newline(newline) {}

    cdk::sequence_node *argument() { return _argument; }

    bool newline() { return _newline; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_print_node(this, level);
    }
  };

} // namespace udf

#endif
