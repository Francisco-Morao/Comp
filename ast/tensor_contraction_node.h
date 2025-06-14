#ifndef __UDF_AST_TENSOR_CONTRACTION_NODE_H__
#define __UDF_AST_TENSOR_CONTRACTION_NODE_H__

#include <cdk/ast/expression_node.h>

namespace udf {

  /**
   * Class for describing tensor contraction nodes.
   */
  class tensor_contraction_node: public cdk::expression_node {
    cdk::expression_node *_tensor1;
    cdk::expression_node *_tensor2;
  public:
    inline tensor_contraction_node(int lineno, cdk::expression_node *tensor1, cdk::expression_node *tensor2) :
        cdk::expression_node(lineno), _tensor1(tensor1), _tensor2(tensor2) {
    }
    
    cdk::expression_node *tensor1() {
      return _tensor1;
    }

    cdk::expression_node *tensor2() {
      return _tensor2;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tensor_contraction_node(this, level);
    }

  };

} // udf

#endif