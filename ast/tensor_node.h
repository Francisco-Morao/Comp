#ifndef __UDF_AST_TENSOR_NODE_H__
#define __UDF_AST_TENSOR_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Class for describing tensor nodes.
   */
  class tensor_node: public cdk::expression_node {
    cdk::sequence_node *_fields;

  public:
    tensor_node(int lineno, cdk::sequence_node *fields) :
        cdk::expression_node(lineno), _fields(fields) {}

    cdk::expression_node* field(size_t ix) {
      return (cdk::expression_node*)_fields->node(ix);
    }

    cdk::sequence_node* fields() {
      return _fields;
    }

    size_t length() {
      return _fields->size();
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_node(this, level);
    }

  };

} // udf

#endif
