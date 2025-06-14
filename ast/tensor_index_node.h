#ifndef __UDF_AST_TENSOR_INDEX_NODE_H__
#define __UDF_AST_TENSOR_INDEX_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include <cdk/ast/sequence_node.h>

namespace udf {

  /**
   * Class for describing tensor index nodes.
   */
  class tensor_index_node: public cdk::lvalue_node {
    cdk::expression_node *_base;
    cdk::sequence_node *_index;

  public:
    tensor_index_node(int lineno, cdk::expression_node *base, cdk::sequence_node *index) :
        cdk::lvalue_node(lineno), _base(base), _index(index) {}

    cdk::expression_node* base() {
      return _base;
    }

    cdk::sequence_node* index() {
      return _index;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_index_node(this, level);
    }

  };

} // udf

#endif
